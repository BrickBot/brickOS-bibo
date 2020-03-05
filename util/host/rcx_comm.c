/*
 *  rcx_comm.c
 *
 *  RCX communication routines.
 *
 *  The contents of this file are subject to the Mozilla Public License
 *  Version 1.0 (the "License"); you may not use this file except in
 *  compliance with the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *  License for the specific language governing rights and limitations
 *  under the License.
 *
 *  The Original Code is Firmdl code, released October 3, 1998.
 *
 *  The Initial Developer of the Original Code is Kekoa Proudfoot.
 *  Portions created by Kekoa Proudfoot are Copyright (C) 1998, 1999
 *  Kekoa Proudfoot. All Rights Reserved.
 *
 *  Contributor(s): Kekoa Proudfoot <kekoa@graphics.stanford.edu>
 */

/*  2002.04.01
 *
 *  Modifications to the original loader.c file in LegOS 0.2.4 include:
 *
 *  Hary D. Mahesan's update to support USB IR firmware downloading 
 *  using RCX 2.0's USB tower under WIN32 on Cygwin.
 *	<hdmahesa@engmail.uwaterloo.ca>
 *	<hmahesan@hotmail.com>
 *
 *  CVS inclusion, revision and modification by Paolo Masetti.
 *	<paolo.masetti@itlug.org>
 *
 *  Matthew Sheets - 2009-04-18
 *   - Changed from a boolean identifier for serial/USB to
 *     a type identifier supporting serial, USB, TCP, and NCD
 *
 * Matthew Sheets - 2009-05-02
 * - Consolidated and merged duplicate firmdl, dll, and lnpmsg code
 *
 * Merged from lnphost.sf.net under the GNU GPL (C) 2004:
 * -Stephan Höhrmann's lnp_hexdump as rcx_hexdump
 *
 * Matthew Sheets - 2009-06-13
 * - Enabled changing the baud rate and timeout settings at run-time
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#ifndef _WIN32
  #if defined(LINUX) | defined(linux)
    #include <sys/ioctl.h>
  #endif

  #include <errno.h>
  #include <termios.h>
  #include <signal.h>
#endif

#if defined(_WIN32) & ( (! defined(__CYGWIN__)) | defined(__MINGW32__) )
  #define usleep(t)    Sleep((t) / 1000)
#else
  // Includes for TCP support (not available under mingw native windows)
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
#endif // __MINGW32__

#include <lnp-logical.h>
#include <lnp.h>
#include <sys/lnp-logical.h>
#include <sys/lnp.h>
#include "rcx_comm.h"

/* Globals */

int __comm_debug = FALSE;

/* Timer routines */

typedef struct timeval timeval_t;

#define tvupdate(tv)  gettimeofday(tv,NULL)
#define tvsec(tv)     ((tv)->tv_sec)
#define tvmsec(tv)    ((tv)->tv_usec * 1e-3)

static float
timer_reset(timeval_t *timer)
{
    tvupdate(timer);
    return 0;
}

static float
timer_read(timeval_t *timer)
{
    timeval_t now;
    tvupdate(&now);
    return tvsec(&now) - tvsec(timer) + (tvmsec(&now) - tvmsec(timer)) * 1e-3;
}

void rcx_set_debug(int comm_debug)
{
    __comm_debug = comm_debug;
}

void rcx_perror(char *str) {
#if defined(_WIN32)
    fprintf(stderr, "Error %lu: %s\n", (unsigned long) GetLastError(), str);
#else
    perror(str);
#endif
}

#if defined(__CYGWIN__) & ! defined(_WIN32)
// Workaround for Cygwin, which is missing cfmakeraw
// Pasted from man page; added in serial.c arbitrarily
void cfmakeraw(struct termios *termios_p)
{
    termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    termios_p->c_oflag &= ~OPOST;
    termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    termios_p->c_cflag &= ~(CSIZE|PARENB);
    termios_p->c_cflag |= CS8;
}
#endif // defined(__CYGWIN__)  & !  defined(_WIN32)

/* Timeout read routine */

// 2004/07/14: modified for ir3, make nbread() non-static, ppetrovic (AT) acm.org
// if TRUE, keep_alive will send a keep-alive signal prior to reading
// is only meant to be used internally
// (some situations, such a echo handling and rx flushing, should not send a keep-alive signal)
int rcx_nbread (tty_t *tty, void *buf, int maxlen, int timeout)
{
    char *bufp = (char *)buf;
    int len = 0;
    fd_set fds;
    struct timeval tv;
#if defined(_WIN32)
    DWORD count = 0;
    // Linux-type file descriptor for TCP communication
    int fd_winux = (int)tty->fd;
    const char *timedoutmsg =
      "%s mode: nbread(len=%d, maxlen=%d) break...timed out\n";
#else
    int count;
#endif

    while (len < maxlen) {

#if defined(_WIN32)
     if(tty_t_usb == tty->type) {
          // USB Stuff here
          // We can't use Serial timeouts.
          struct timeval timebegin ,timenow;
          unsigned long elapsed; // for timeout values
          count = 0;

          gettimeofday(&timebegin,0); 
          while(count==0) {
            ReadFile(tty->fd, &bufp[len], maxlen - len, &count, NULL);
            gettimeofday(&timenow,0);
            elapsed = ((timenow.tv_sec - timebegin.tv_sec )*1000) +
              ((timenow.tv_usec - timebegin.tv_usec)/1000);				
            if(elapsed > timeout) 
                break;
          }
          if(count==0) {
            if(__comm_debug)
              printf(timedoutmsg, "USB", len, maxlen);
            break;
          }

          len += (int)count; //update len
      } else if (tty_types_local_serial & tty->type) {
        // serial stuff now.
        count = 0;

        COMMTIMEOUTS CommTimeouts;

        GetCommTimeouts (tty->fd, &CommTimeouts);

        // Change the COMMTIMEOUTS structure settings.
        CommTimeouts.ReadIntervalTimeout = MAXDWORD;
        CommTimeouts.ReadTotalTimeoutMultiplier = 0;
        CommTimeouts.ReadTotalTimeoutConstant = timeout;
        CommTimeouts.WriteTotalTimeoutMultiplier = 10;
        CommTimeouts.WriteTotalTimeoutConstant = 1000;

        // Set the time-out parameters for all read and write operations
        // on the port.
        SetCommTimeouts(tty->fd, &CommTimeouts);

        if (ReadFile(tty->fd, &bufp[len], maxlen - len, &count, NULL) == FALSE) {
          rcx_perror("ReadFile");
	      fprintf(stderr, "nbread: error reading tty (%d): %lu\n",
	        (int) tty->type, (unsigned long) GetLastError());
	      exit(1);
	    }

        len += (int)count;

        if (count == 0) {
		  if(__comm_debug) {
            printf(timedoutmsg, "Serial", len, maxlen);
          }
		  break;
        }
	  } else {
        // TCP uses more Linux-style communication
		FD_ZERO(&fds);
		FD_SET(fd_winux, &fds);

		if (len == 0) {
		    tv.tv_sec = (timeout) / 1000 + 2;
		    tv.tv_usec = (timeout % 1000) * 1000;
		} else {
		    tv.tv_sec = timeout / 1000;
		    tv.tv_usec = (timeout % 1000) * 1000;
		}
  
		if (select(fd_winux+1, &fds, NULL, NULL, &tv) < 0) {
		    rcx_perror("select");
		    exit(1);
		}
		if (!FD_ISSET(fd_winux, &fds))
		    break;

	    count = read(fd_winux, &bufp[len], maxlen - len);
        
        len += (int)count;

        if (count == 0) {
		  if(__comm_debug) 
			printf(timedoutmsg, "TCP", len, maxlen);
		  break;
        }
      }
#else
	 if (tty_t_usb == tty->type)
	 {
#if defined(LINUX) | defined(linux)
	    // LegoUSB doesn't work with select(), so just set a read
	    // timeout and then later check to see if the read timed out
	    // without reading data.

	    ioctl(tty->fd, _IOW('u', 0xc8, int), timeout);
#endif
	 }
     else
	 {
		FD_ZERO(&fds);
		FD_SET(tty->fd, &fds);

		if (len == 0) {
		    tv.tv_sec = (timeout) / 1000 + 2;
		    tv.tv_usec = (timeout % 1000) * 1000;
		} else {
		    tv.tv_sec = timeout / 1000;
		    tv.tv_usec = (timeout % 1000) * 1000;
		}
  
		if (select(tty->fd+1, &fds, NULL, NULL, &tv) < 0) {
		    rcx_perror("select");
		    exit(1);
		}
		if (!FD_ISSET(tty->fd, &fds))
		    break;
	 }

	 count = read(tty->fd, &bufp[len], maxlen - len);

     // If no data was read from a USB tower and the read() timed out,
     // go ahead and assume we are done reading data.
     if (tty_t_usb == tty->type && count == -1 && errno == ETIMEDOUT) 
       break;

     if (count < 0) {
         rcx_perror("read");
         exit(1);
     }

     len += count;
#endif

    }
    
    
    if (__comm_debug) {
      printf("nbread: total read len %d from FD %d\n", len, (int)tty->fd);
      rcx_hexdump ("NBR", buf, len, TRUE);
    }

    return len;
}

//! write to IR port, blocking; specific to the "host" implementation.
/* discard all characters in the input queue of tty */
static void rx_flush(tty_t *tty, int timeout)
{
    if (__comm_debug)
      printf("rcx_comm: flushing receive buffer\n");
    
#if defined(_WIN32)
    if (tty_t_serial == tty->type || tty_t_usb == tty->type) {
	  PurgeComm(tty->fd, PURGE_RXABORT | PURGE_RXCLEAR);
    } else {
      char echo[RCX_BUFFERSIZE];
      rcx_nbread(tty, echo, RCX_BUFFERSIZE, timeout);
    }
#else
    char echo[RCX_BUFFERSIZE];
    rcx_nbread(tty, echo, RCX_BUFFERSIZE, timeout);
#endif
}

/*! \param file descriptor to write to
    \param tty type
    \param buf data to transmit
    \param len number of bytes to transmit
    \return 0 on success, else collision
*/
int rcx_nbwrite(tty_t *tty, void *buf, int len, int timeout)
{
    char echo[RCX_BUFFERSIZE];
    int echolen = -1;
    
    rcx_keepalive_reset(tty);
    
    if((tty_types_echo & tty->type) != 0) {
    // Need to flush the receive buffer so as not to confuse the echo handling
        rx_flush(tty, timeout);
    }
    if (__comm_debug) {
        rcx_hexdump("NBW", buf, len, TRUE);
    }
    int result = lnp_logical_write_host(tty, buf, len);
    rcx_keepalive_reset(tty);
    
    // Skip echo handling for tty types (such as USB Tower) that don't echo
    if((tty_types_echo & tty->type) != 0) {
        if (__comm_debug) { printf("send: receive echo\n"); }
        
        echolen = rcx_nbread(tty, echo, len, timeout);
        
        if (__comm_debug) {
            printf("msglen = %d, echolen = %d\n", len, echolen);
        }
        
        // Check echo, ignoring data since RCX might send ack even if echo data is wrong
        // echo may contain extra "keepalive" bytes
        if (echolen != len /* || memcmp(echo, buf, len) */ ) {
            /* Flush connection if echo is bad */
            rx_flush(tty, timeout);
            return RCX_BAD_ECHO;
        }
    }

    return result;
}

int rcx_recv_lnp(tty_t *tty, void *buf, int maxlen, int timeout)
{
    int i = 0;
    char *bufp = (char *)buf;
    int len = rcx_nbread(tty, buf, maxlen, timeout);
    
    // Handle the data read in
    for (i = 0; i < len; i++){
        lnp_integrity_byte(bufp[i]);
    }

    return len;
}


//! initialize keepalive
void rcx_keepalive_reset(tty_t *tty)
{
    if (tty_types_keepalive & tty->type) {
        if (__comm_debug) { printf("KeepAlive Reset...\n"); }
        alarm(RCX_KEEPALIVE_INTERVAL);
    }
}

//! disable keepalive (does not end the keepalive tick)
// This must first be called from rcx_keepalive_init to initialize the fd and tty_type values before using it via the handler
void rcx_keepalive_send(tty_t *tty)
{
    static tty_t send_tty = { BADFILE, tty_t_undefined };
    
    if (tty->fd != BADFILE && tty->type != tty_t_undefined){
        send_tty.fd = tty->fd;
        send_tty.type = tty->type;
    }
    
    if (tty_types_keepalive & send_tty.type) {
        if (__comm_debug) { printf("KeepAlive Send...\n"); }
        
        char keepalive = RCX_KEEPALIVE_BYTE;
        // Send the keepalive byte
        if (lnp_logical_write_host(&send_tty, &keepalive, 1) == 0) {
            rcx_keepalive_reset(&send_tty);
        }
    }
}

void rcx_keepalive_handler(int signal)
{
    tty_t tty_null = { BADFILE, tty_t_undefined };
    if (__comm_debug) { printf("KeepAlive Handler...\n"); }
    rcx_keepalive_send(&tty_null);
}

void rcx_keepalive_init(tty_t *tty)
{
    if (tty_types_keepalive & tty->type) {
        if (__comm_debug) { printf("KeepAlive Init...\n"); }
        signal(SIGALRM, rcx_keepalive_handler);
        rcx_keepalive_send(tty);
    }
}

void rcx_keepalive_end(tty_t *tty)
{
    if (tty_types_keepalive & tty->type) {
        if (__comm_debug) { printf("KeepAlive End...\n"); }
        alarm(0);
        signal(SIGALRM, SIG_DFL);
    }
}


// Wake up the serial tower
int rcx_wakeup_tower (tty_t *tty, int timeout, int wakeup_timeout)
{
    if (__comm_debug) { printf("Wakeup Tower...\n"); }
    
   // Only serial IR towers need woken up
    if (tty_types_keepalive & tty->type) {
        char msg[] = RCX_WAKEUP_MESSAGE;
        char keepalive = RCX_KEEPALIVE_BYTE;
        char buf[RCX_BUFFERSIZE];
        timeval_t timer;
        int count = 0;
        int len;

        // Send a KeepAlive Byte to settle IR Tower...
        lnp_logical_write_host(tty, &keepalive, 1);
        usleep(timeout * 10000);
        rx_flush(tty, timeout);
        
        // Start the timer
        timer_reset(&timer);

        do {
            if (__comm_debug) {
                printf("wakeup tower writelen = %u\n", (unsigned)sizeof(msg));
                rcx_hexdump("WTW", msg, sizeof(msg), FALSE);
            }
            // We are looking for the echo, so we use lnp_logical_write_host
            if (lnp_logical_write_host(tty, msg, sizeof(msg)) != 0) {
                rcx_perror("write wakeup message");
                exit(1);
            }
            count += len = rcx_nbread(tty, buf, RCX_BUFFERSIZE, timeout);
            if (len == sizeof(msg)&& !memcmp(buf, msg, sizeof(msg))) {
                return RCX_OK; /* success */
            }
            if (__comm_debug) {
                printf("wakeup tower recvlen = %d\n", len);
            }
            rx_flush(tty, timeout);
            usleep(timeout * 10000);
        } while (timer_read(&timer) < (float)wakeup_timeout / 1000.0f);

        if (!count)
            return RCX_NO_TOWER; /* tower not responding */
        else
            return RCX_BAD_LINK; /* bad link */
    } else {
        return RCX_OK;
    }
}


char * rcx_read_conf_file(const char *conf_file_path, char *tty_name_buf, const int buf_len)
{
    char *tty_name = 0;
    int fd_conf = open(conf_file_path, O_RDONLY);
    if (fd_conf != -1)
    {
      fprintf(stderr, "rcx_comm: reading file %s\n", conf_file_path);
        int len_conf = read(fd_conf, tty_name_buf, buf_len);
        if (len_conf > 0  && len_conf < buf_len)
        {
            // Change the first CR, LF, or EOF to NULL to set the first line as a string
            char *p_tmp;
            p_tmp = strchr(tty_name_buf, '\r');
            if (p_tmp != NULL) { *p_tmp = 0; }
            p_tmp = strchr(tty_name_buf, '\n');
            if (p_tmp != NULL) { *p_tmp = 0; }
            p_tmp = strchr(tty_name_buf, EOF);
            if (p_tmp != NULL) { *p_tmp = 0; }
            
            // Verify the string is not the empty string
            if (strlen(tty_name_buf) > 0)
            {
                tty_name = tty_name_buf;
                if(__comm_debug && tty_name)
                  fprintf(stderr, "rcx_comm: using tty %s per config file %s\n", tty_name, conf_file_path);
            }
        } else {
            if(__comm_debug)
                fprintf(stderr, "rcx_comm: no tty detected in %s (len = %d)\n", conf_file_path, len_conf);
        }
        close(fd_conf);
    } else {
        if(__comm_debug)
            fprintf(stderr, "rcx_comm: cannot open file %s\n", conf_file_path);
    }
    return tty_name;
}


#if ! ( ( defined(_WIN32) & ( (! defined(__CYGWIN__)) | defined(__MINGW32__) ) ) )
//! initialize RCX communications port via tcp
void rcx_init_tcp(tty_t *tty, const char *tty_name, int baud)
{
  tty->fd = BADFILE;
  
  char *portStr;
  int port = 50637;
  struct hostent *h;
  struct sockaddr_in localAddr, ttyAddr;

  portStr = strchr(tty_name, ':');
  if (portStr) {
    *portStr = 0;
    port = atoi(portStr+1);
  }

  h = gethostbyname(tty_name);

  if (portStr)
    *portStr = ':';

  if (!h) {
    fprintf(stderr, "Unknown host: %s\n", tty_name);
    return;
  }

  if (tty_t_ncd == tty->type) {
      char buffer[400];
      int len;
      /* create setup socket */

      tty->fd = socket(AF_INET, SOCK_STREAM, 0);
      if(tty->fd<0) {
        rcx_perror("cannot open socket "); 
        return;
      }

      /* bind any port number */
      localAddr.sin_family = AF_INET;
      localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
      localAddr.sin_port = htons(0);
      
      if (bind(tty->fd, (struct sockaddr *) &localAddr, sizeof(localAddr)) < 0) {
        rcx_perror("cannot bind");
        return;
      }

      ttyAddr.sin_family = h->h_addrtype;
      memcpy((char *) &ttyAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
      ttyAddr.sin_port = htons(5979);
      
      if (connect(tty->fd, (struct sockaddr *) &ttyAddr, sizeof(ttyAddr)) < 0) {
        rcx_perror("cannot connect to config");
        return;
      }

      /* Read in password prompt */
      read(tty->fd, buffer, sizeof(buffer));
      /* write password */
      write (tty->fd, "<insert correct password here>\n", 8);

      /* Read in prompt */
      read(tty->fd, buffer, sizeof(buffer));

      /* write commands */
      len = sprintf(buffer,
            "set serial-interfaces-table = { "
            "{ 1 printer printer %d 8 1 %s none none } }\n"
            "apply\n", baud,
            (2400 == baud) ? "odd" : "none");
      write(tty->fd, buffer, len);

      /* Read in prompts */
      read(tty->fd, buffer, sizeof(buffer));

      /* Quit */
      write(tty->fd, "quit\n", 5);
      rcx_close(tty);
  } // ncd setup


  /* create socket */

  tty->fd = socket(AF_INET, SOCK_STREAM, 0);
  if(tty->fd<0) {
    rcx_perror("cannot open socket "); 
    return;
  }

  /* bind any port number */
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(0);
  
  if (bind(tty->fd, (struct sockaddr *) &localAddr, sizeof(localAddr)) < 0) {
    rcx_perror("cannot bind");
    return;
  }

  ttyAddr.sin_family = h->h_addrtype;
  memcpy((char *) &ttyAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  ttyAddr.sin_port = htons(port);
  
  if (connect(tty->fd, (struct sockaddr *) &ttyAddr, sizeof(ttyAddr)) < 0) {
    rcx_perror("cannot connect");
    return ;
  }
}
#endif // __MINGW32__


void rcx_init(tty_t *tty, const char *tty_name, int baud, int timeout, int start_keepalive)
{
    char tty_name_buf[PATH_MAX];
    memset(tty_name_buf, 0, sizeof(tty_name_buf));

    tty->fd = BADFILE;
    tty->type = tty_t_undefined;

#if defined(_WIN32)
    DCB dcb;
    COMMTIMEOUTS CommTimeouts;
#else
    struct termios ios;
#endif

    // Check the environment variable for the tty name
    if (!tty_name)
    {
	  tty_name = getenv("RCXTTY");
      if(__comm_debug && tty_name)
        fprintf(stderr, "rcx_comm: using environment variable tty of %s\n", tty_name);
    }
    // Attempt to read the tty name from the configuration file under the user's home directory
    if (!tty_name)
    {
        char conf_file_path[PATH_MAX];
        
        // Get the HOME directory path
        if (strncmp(CONF_DEVICE_USER_CONF_FILE, "~", 1) == 0)
        {
            char *p_home = getenv("HOME");
            strcpy(conf_file_path, p_home);
            strcat(conf_file_path, CONF_DEVICE_USER_CONF_FILE + 1);
        } else {
            strcpy(conf_file_path, CONF_DEVICE_USER_CONF_FILE);
        }
        
        tty_name = rcx_read_conf_file(conf_file_path, tty_name_buf, sizeof(tty_name_buf));
    }
#if defined(LINUX) | defined(linux) | defined(__CYGWIN__)
    // Attempt to read the tty name from the configuration file under /etc
    if (!tty_name)
    {
        tty_name = rcx_read_conf_file(CONF_DEVICE_ETC_CONF_FILE, tty_name_buf, sizeof(tty_name_buf));
    }
#else
    // TODO: Check the "All Users" profile on Windows (???)
#endif
    if (!tty_name)
    {
	  tty_name = DEFAULTTTY;
      if(__comm_debug)
        fprintf(stderr, "rcx_comm: using default tty of %s\n", tty_name);
    }

    if(__comm_debug)
      fprintf(stderr, "rcx_comm: tty is %s\n", tty_name);

    // BEGIN: Determine the tty type
    // if (*tty_name == '-' && !*(tty_name + 1)) {  // read from standard input if tty_name="-"
    // tty->fd = 0;
    // } else
    if (strncmp(tty_name, "tcp:", 4) == 0)
    {
      tty->type = tty_t_tcp;
      tty_name += 4;
	  if(__comm_debug)
		fprintf(stderr, "rcx_comm: Jochen Hoenicke - TCP Mode.\n");
    }
    else if (strncmp(tty_name, "ncd:", 4) == 0)
    {
      tty->type = tty_t_ncd;
      tty_name += 4;
	  if(__comm_debug)
		fprintf(stderr, "rcx_comm: Jochen Hoenicke - NCD Mode.\n");
    }
    else if (strncmp(tty_name, "usb:", 4) == 0)
    {
      tty->type = tty_t_usb;
      tty_name += 4;
	  if(__comm_debug)
		fprintf(stderr, "rcx_comm: USB-specified Mode.\n");
    }
    else if (strncmp(tty_name, "com:", 4) == 0)
    {
      tty->type = tty_t_com;
      tty_name += 4;
	  if(__comm_debug)
		fprintf(stderr, "rcx_comm: Non-Echo, Non-Keepalive Serial Communication Mode.\n");
    }
    else if (strncmp(tty_name, "serial:", 7) == 0)
    {
      tty->type = tty_t_serial;
      tty_name += 7;
	  if(__comm_debug)
		fprintf(stderr, "rcx_comm: Serial-specified Mode.\n");
    }

    //Check the command line to see if IR tower is USB.
#if defined(_WIN32)
    else if(stricmp(tty_name, "usb")==0)	{
        tty->type = tty_t_usb;
        if(__comm_debug)
            fprintf(stderr, "rcx_comm: Hary Mahesan - USB IR Tower Mode.\n");
        tty_name = "\\\\.\\legotower1"; // Set the correct usb tower if you have more than one (unlikely).
    }
#elif defined(LINUX) || defined(linux)
    // If the tty string contains "usb", e.g. /dev/usb/legousbtower0, we
    // assume it is the USB tower.  If you use something else that doesn't
    // have "usb" in the device name, link it.
    else if (strstr(tty_name,"usb") !=0) {
       tty->type = tty_t_usb;
       if (__comm_debug)
           fprintf(stderr, "rcx_comm: P.C. Chan & Tyler Akins - USB IR Tower Mode for Linux.\n");
    }
#endif
    else {
       tty->type = tty_t_serial;
       if (__comm_debug)
           fprintf(stderr, "rcx_comm: Serial IR Tower Mode.\n");
    }
    //END: Determine the tty type

    if (__comm_debug) printf("rcx_comm: baud = %d\n", baud);


    // BEGIN: Create the FILEDESCR connection
    if (tty_types_tcp & (tty->type)) {
	    rcx_init_tcp(tty, tty_name, baud);
    }
#if defined(_WIN32)
    // Windows platform I/O
    else {
        if ((tty->fd = CreateFile(tty_name, GENERIC_READ | GENERIC_WRITE,
                              0, NULL, OPEN_EXISTING,
                              0, NULL)) == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "Error %lu: Opening %s\n", (unsigned long) GetLastError(), tty_name);
            return;
        }

        //These settings apply to the serial tower, so if{} them out.
        if(tty_types_local_serial & tty->type) {
            // Serial settings
            FillMemory(&dcb, sizeof(dcb), 0);
            if (!GetCommState(tty->fd, &dcb)) {	// get current DCB
                // Error in GetCommState
                rcx_perror("GetCommState");
                return;
            } else {
              dcb.ByteSize = 8;
              dcb.StopBits = 0;			// 0,1,2 = 1, 1.5, 2
              dcb.fBinary  = TRUE ;
              dcb.fAbortOnError = FALSE ;
              dcb.Parity   = 1;	// 0-4=no,odd,even,mark,space
              dcb.fParity  = TRUE;
              switch (baud) {
                case 2400:
                  dcb.BaudRate = CBR_2400;	// Update DCB rate.
                  break;
                case 4800:
                  dcb.BaudRate = CBR_4800;	// Update DCB rate.
                  break;
                case 9600:
                  dcb.BaudRate = CBR_9600;
                  break;
                case 19200:
                  dcb.BaudRate = CBR_19200;
                  break;
                case 38400:
                  dcb.BaudRate = CBR_38400;
                  break;
                default:
                  fprintf(stderr, "ERROR: Invalid baud rate of %d", baud);
                  return;
              }

              // Set new state.
              if (!SetCommState(tty->fd, &dcb)) {
                // Error in SetCommState. Possibly a problem with the communications
                // port handle or a problem with the DCB structure itself.
                rcx_perror("SetCommState");
                return;
              }
              if (!GetCommTimeouts (tty->fd, &CommTimeouts))
                rcx_perror("GetCommTimeouts");

              // Change the COMMTIMEOUTS structure settings.
              CommTimeouts.ReadIntervalTimeout = MAXDWORD;
              CommTimeouts.ReadTotalTimeoutMultiplier = 0;
              CommTimeouts.ReadTotalTimeoutConstant = 0;
              CommTimeouts.WriteTotalTimeoutMultiplier = 10;
              CommTimeouts.WriteTotalTimeoutConstant = 1000;

              // Set the time-out parameters for all read and write operations
              // on the port.
              if (!SetCommTimeouts(tty->fd, &CommTimeouts))
                rcx_perror("SetCommTimeouts");
            } // GetCommState
        } // serial
    } // Windows platform I/O

#else
	// Linux platform I/O
    else {
        if ((tty->fd = open(tty_name, O_RDWR | O_NOCTTY)) < 0) {
            fprintf(stderr, "ERROR: tty=%s, ", tty_name);
            rcx_perror("open");
            return;
        }

        if (tty_types_local_serial & tty->type){
            if (!isatty(tty->fd)) {
                rcx_close(tty);
                fprintf(stderr, "%s: not a tty\n", tty_name);
                return;
            }

            memset(&ios, 0, sizeof(ios));
            tcgetattr(tty->fd, &ios);

            cfmakeraw(&ios);
            ios.c_cc[VMIN] = 0;
            ios.c_cc[VTIME] = 10; // timeout is 1 second

            // The baud rate, word length, and handshake
            // Not all <termios.h> have cfsetspeed (e.g. Cygwin); call
            // both cfsetispeed and cfsetospeed instead

            // Use 8N1
            //ios.c_cflag |= (is_fast ? 0 : PARENB | PARODD);
            ios.c_cflag |= (PARENB | PARODD);
            
            switch (baud) {
              case 2400:
                cfsetispeed(&ios, B2400); // Set input speed
                cfsetospeed(&ios, B2400); // Set output speed
                break;
              case 4800:
                cfsetispeed(&ios, B4800); // Set input speed
                cfsetospeed(&ios, B4800); // Set output speed
                break;
              case 9600:
                cfsetispeed(&ios, B9600); // Set input speed
                cfsetospeed(&ios, B9600); // Set output speed
                break;
              case 19200:
                cfsetispeed(&ios, B19200); // Set input speed
                cfsetospeed(&ios, B19200); // Set output speed
                break;
              case 38400:
                cfsetispeed(&ios, B38400); // Set input speed
                cfsetospeed(&ios, B38400); // Set output speed
                break;
            }

            // set parity enable
            ios.c_cflag &= ~(CSTOPB); // one stop bit
            ios.c_cflag &= ~CSIZE; // Mask the character size bits
            ios.c_cflag |= (CS8); // 8bit

            ios.c_cflag &= ~CRTSCTS; // no hw flow control
            ios.c_iflag &= ~(IXON | IXOFF | IXANY); // no sw flow control

            ios.c_cflag |= ( CREAD | CLOCAL); // enable receiver, local line
            ios.c_iflag &= ~(INPCK); // disable input parity checking
            ios.c_iflag |= IGNBRK; // ignore break

            // RAW outout and input
            ios.c_oflag &= ~OPOST;
            ios.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);

            if (tcsetattr(tty->fd, TCSANOW, &ios) == -1) {
                rcx_perror("tcsetattr");
                return;
            }
        }
    }
#endif
    // END: Create the FILEDESCR connection
    
    // Final initialization steps if the fd is good
    if (tty->fd != BADFILE) {
        // LNP initialization
        lnp_init();
        lnp_logical_init(tty);

        // Wakeup tower
        int status;
        if ((status = rcx_wakeup_tower(tty, timeout, RCX_WAKEUP_TIMEOUT)) < 0) {
            fprintf(stderr, "init wakeup tower: %s\n", rcx_strerror(status));
            return;
        }

        // KeepAlive initialization
        if (start_keepalive) {
            rcx_keepalive_init(tty);
        }
    }
    
    rx_flush(tty, timeout);
}

void rcx_close(tty_t *tty)
{
    lnp_logical_shutdown();
    lnp_shutdown();

    rcx_keepalive_end(tty);
#if defined(_WIN32)
    CloseHandle(tty->fd);
#else
    close(tty->fd);
#endif
}

/* Hexdump routine */

// Bytes per line
#define HEXDUMP_LINE 16
// Number of bytes in a line to be grouped
#define HEXDUMP_GROUP 8

void rcx_hexdump(char *prefix, const void *data, unsigned length, char show_ASCII)
{
  unsigned offset;
  unsigned char *dataptr=(unsigned char *)data;
  unsigned linesize, lineindex;

  for (offset=0; offset<length; offset+=HEXDUMP_LINE) {
    linesize=(length-offset<HEXDUMP_LINE)?(length-offset):HEXDUMP_LINE;
    if (prefix) printf("%s ",prefix);
    printf("%04X",offset);
    for (lineindex=0; lineindex<HEXDUMP_LINE; lineindex++) {
      if (lineindex<linesize) printf(" %02X",*(dataptr++));
      else if (show_ASCII) printf("   ");
      if ((lineindex%HEXDUMP_GROUP)==(HEXDUMP_GROUP-1)) printf(" ");
    }
    if (show_ASCII) {
      printf(" ");
      dataptr-=linesize;
      for (lineindex=0; lineindex<HEXDUMP_LINE; lineindex++) {
	if (lineindex<linesize)
	  printf("%c",(isprint(*dataptr))?(*dataptr):'.');
	dataptr++;
      }
    }
    printf("\n");
  }
}


int rcx_send (tty_t *tty, void *buf, int len, int timeout, int use_comp)
{
    char *bufp = (char *)buf;
    char buflen = len;
    char msg[RCX_BUFFERSIZE];
    int msglen;
    int sum;

    /* Encode message */

    msglen = 0;
    sum = 0;

    if (use_comp) {
        msg[msglen++] = 0x55;
        msg[msglen++] = 0xff;
        msg[msglen++] = 0x00;
        while (buflen--) {
            msg[msglen++] = *bufp;
            msg[msglen++] = (~*bufp) & 0xff;
            sum += *bufp++;
        }
        msg[msglen++] = sum;
        msg[msglen++] = ~sum;
    }
    else {
        msg[msglen++] = 0xff;
            while (buflen--) {
            msg[msglen++] = *bufp;
            sum += *bufp++;
        }
        msg[msglen++] = sum;
    }

    /* Send message */
    
    if (__comm_debug) {
        printf("buflen = %d; sendlen = %d\n", len, msglen);
        rcx_hexdump("MSB", buf, len, TRUE);
    }

    if (rcx_nbwrite(tty, msg, msglen, timeout) != 0) {
        rcx_perror("write");
        exit(1);
    }

    return len;
}

int rcx_recv (tty_t *tty, void *buf, int maxlen, int timeout, int use_comp)
{
    char *bufp = (char *)buf;
    unsigned char msg[RCX_BUFFERSIZE];
    int msglen;
    int sum;
    int pos;
    int len;

    /* Receive message */

    msglen = rcx_nbread(tty, msg, RCX_BUFFERSIZE, timeout);

    if (__comm_debug) {
        printf("recvlen = %d\n", msglen);
    }

    /* Check for message */

    if (!msglen)
	  return RCX_NO_RESPONSE;

    /* Verify message */

    if (use_comp) {
        if (msglen < 5 || (msglen - 3) % 2 != 0)
            return RCX_BAD_RESPONSE;

        if (msg[0] != 0x55 || msg[1] != 0xff || msg[2] != 0x00)
            return RCX_BAD_RESPONSE;

        for (sum = 0, len = 0, pos = 3; pos < msglen - 2; pos += 2) {
            if (msg[pos] != ((~msg[pos+1]) & 0xff))
                return RCX_BAD_RESPONSE;
            sum += msg[pos];
            if (len < maxlen)
                bufp[len++] = msg[pos];
        }

        if (msg[pos] != ((~msg[pos+1]) & 0xff))
            return RCX_BAD_RESPONSE;

        if (msg[pos] != (sum & 0xff))
            return RCX_BAD_RESPONSE;

        /* Success */
        return len;
    }
    else {
        if (msglen < 4)
            return RCX_BAD_RESPONSE;

        if (msg[0] != 0x55 || msg[1] != 0xff || msg[2] != 0x00)
            return RCX_BAD_RESPONSE;

        for (sum = 0, len = 0, pos = 3; pos < msglen - 1; pos++) {
            sum += msg[pos];
            if (len < maxlen)
            bufp[len++] = msg[pos];
        }

        /* Return success if checksum matches */
        if (msg[pos] == (sum & 0xff))
            return len;

        /* Failed.  Possibly a 0xff byte queued message? (unlock firmware) */
        for (sum = 0, len = 0, pos = 3; pos < msglen - 2; pos++) {
          sum += msg[pos];
          if (len < maxlen)
            bufp[len++] = msg[pos];
        }

        /* Return success if checksum matches */
        if (msg[pos] == (sum & 0xff))
          return len;

        /* Failed.  Possibly a long message? */
        /* Long message if opcode is complemented and checksum okay */
        /* If long message, checksum does not include opcode complement */
        for (sum = 0, len = 0, pos = 3; pos < msglen - 1; pos++) {
            if (pos == 4) {
            if (msg[3] != ((~msg[4]) & 0xff))
                return RCX_BAD_RESPONSE;
            }
            else {
            sum += msg[pos];
            if (len < maxlen)
                bufp[len++] = msg[pos];
            }
        }

        if (msg[pos] != (sum & 0xff))
            return RCX_BAD_RESPONSE;

        /* Success */
        return len;
    }
}

int rcx_sendrecv (tty_t *tty, void *send, int slen,
		  void *recv, int rlen, int timeout, int retries, int use_comp)
{
    int status = 0;

    if (__comm_debug) printf("sendrecv %d:\n", slen);

    while (retries--) {
        if ((status = rcx_send(tty, send, slen, timeout, use_comp)) < 0) {
            if (__comm_debug) printf("status = %s\n", rcx_strerror(status));
                continue;
        }
        if ((status = rcx_recv(tty, recv, rlen, timeout, use_comp)) < 0) {
            if (__comm_debug) printf("status = %s\n", rcx_strerror(status));
                continue;
        }
        break;
    }

    if (__comm_debug) {
        if (status > 0)
            printf("status = %s\n", rcx_strerror(0));
        else
            printf("status = %s\n", rcx_strerror(status));
    }

    return status;
}

int rcx_is_alive (tty_t *tty, int timeout, int use_comp)
{
    unsigned char send[1] = { 0x10 };
    unsigned char recv[1];

    return (rcx_sendrecv(tty, send, 1, recv, 1, timeout, RCX_RETRIES, use_comp) == 1);
}

char *rcx_strerror (int error)
{
    switch (error) {
    case RCX_OK: return "no error";
    case RCX_NO_TOWER: return "tower not responding";
    case RCX_BAD_LINK: return "bad ir link";
    case RCX_BAD_ECHO: return "bad ir echo";
    case RCX_NO_RESPONSE: return "no response from rcx";
    case RCX_BAD_RESPONSE: return "bad response from rcx";
    default: return "unknown error";
    }
}

