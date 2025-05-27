/* IR Server for LEGO RCX Brick, Copyright (C) 2003 Jochen Hoenicke.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; see the file COPYING.LESSER.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Author: Dr. Jochen Hoenicke
 *
 * Contributors:
 *      Matthew Sheets - 2009-05-09
 *          - Added tty downlink connection option
 *          - Added tcp uplink connection option
 *          - Enabled setting the broadcast port via the command line
 *
 * $Id: ir-server.c 97 2004-08-17 16:33:38Z hoenicke $
 */


#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "lnp-logical.h"
#include "rcx_comm.h"

// Command-line argument processing
#if (defined(__sun__) && defined(__svr4__)) || defined(BSD)	// Solaris||BSD
#undef HAVE_GETOPT_LONG
#else
#define HAVE_GETOPT_LONG
#endif

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>

static const struct option long_options[]={
  {"port",   required_argument,0,'p'},
  {"tty",    required_argument,0,'t'},
  {"tcp",    required_argument,0,'c'},
  {"baud",   required_argument,0,'b'},
  {"timeout",required_argument,0,'o'},
  {"help",   no_argument      ,0,'h'},
  {"verbose",no_argument      ,0,'v'},
  {0        ,0                ,0,0  }
};

#else // HAVE_GETOPT_LONG
  #define getopt_long(ac, av, opt, lopt, lidx) (getopt((ac), (av), (opt)))
#endif // HAVE_GETOPT_LONG

#define MAX_CLIENTS 255

int sockets[MAX_CLIENTS];

static void check_exit(tty_t *tty) {
    int i;
    for (i = 1; i < MAX_CLIENTS; i++) {
        if (sockets[i] != -1)
            return;
    }
    
    if (tty->fd != BADFILE) {
        rcx_close(tty);
    }
    
    exit(0);
}

int main(int argc, char **argv) {
    tty_t tty = { BADFILE, tty_t_undefined };
    tty_t tty_tcp_uplink = { BADFILE, tty_t_tcp };
    char *tty_name = NULL;
    char *tcp_uplink = NULL;
    int baud = RCX_DEFAULT_BAUD;
    int timeout = RCX_DEFAULT_TIMEOUT;
    int opt;
#ifdef HAVE_GETOPT_LONG
    int option_index;
#endif
    
    fd_set rfds;
//    struct sigaction sigact;
    struct sockaddr addr;
    struct sockaddr_in* addr_in = (struct sockaddr_in*) &addr;
    struct timeval tv;
    long port = DEFAULT_IR_SERVER_BROADCAST_PORT;
    int slot_len = 0;
    int val = 1;
    int len = 0;
    char buff[RCX_BUFFERSIZE];
    char buff_echo[RCX_BUFFERSIZE];
    int len_echo = 0;
    int i, j;

    char *usage_string =
    "usage: %s [options]\n\n"
    RCX_COMM_OPTIONS
    "  -p<port>     , --port=<port number>  server broadcast port (default: " DEFAULT_IR_SERVER_BROADCAST_PORT_AS_STRING ")\n"
    "  -c<tcp>      , --tcp=<server>:<port> uplink to another TCP IR server\n"
	"\n"
    "Note: There is no functional difference between TCP uplink and TCP downlink\n"
	"\n"
    RCX_COMM_OPTIONS_INFO
	"\n"
	;

    // Process command-line arguments
    while((opt=getopt_long(argc, argv, "p:t:b:o:c:hv",
                        (struct option *)long_options, &option_index) )!=-1) {
        switch(opt) {
          case 'b':
            baud = atoi(optarg);
            break;
          case 'c':
            tcp_uplink = optarg;
            break;
          case 'h':
            fprintf(stderr, usage_string, argv[0]);
            return 1;
            break;
          case 'o':
            timeout = atoi(optarg);
            break;
          case 'p':
            port = atol(optarg);
            break;
          case 't':
            tty_name = optarg;
            break;
          case 'v':
            rcx_set_debug(1);
            break;
        }
    }

    // Set the slot length based on the baud
    if (2400 == baud) {
        slot_len = 11;
    } else {
        slot_len = 10;
    }
    
    // initialize the tty downlink (if specified)
    if (tty_name != NULL) {
        rcx_init(&tty, tty_name, baud, timeout, TRUE);
        if (BADFILE == tty.fd) {
            perror("tty downlink");
            return 1;
        }
    }
    
    // initialize the socket array
    memset(sockets, -1, sizeof(sockets));
    
    // initialize the tcp uplink (if specified) to socket "1"
    if (tcp_uplink != NULL) {
        rcx_init_tcp(&tty_tcp_uplink, tcp_uplink, baud);
        if (BADFILE == tty_tcp_uplink.fd) {
            perror("tcp uplink");
            return 1;
        } else {
            sockets[1] = (int)tty_tcp_uplink.fd;
        }
    }

    // initialize the server
    sockets[0] = socket(PF_INET, SOCK_STREAM, 0);
    setsockopt(sockets[0], SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    memset(&addr, 0, sizeof(addr));
    addr_in->sin_family = AF_INET;
    addr_in->sin_port = htons(port);
    addr_in->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(sockets[0], &addr, sizeof(addr)) < 0) {
      perror("bind");
      return 1;
    }
    listen(sockets[0], 10);
    printf("Server started.\n");
//    if (fork())
//      return 0;

//    sigact.sa_handler = SIG_IGN;
//    sigemptyset(&sigact.sa_mask);
//    sigact.sa_flags = SA_RESTART;
//    sigaction(SIGPIPE, &sigact, NULL);
    // close stdin/out/err
//    close(0);
//    close(1);
//    close(2);
    
    while (1) {
        int max = 0;
        FD_ZERO(&rfds);
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (sockets[i] >= 0) {
                if (sockets[i] >= max)
                    max = sockets[i] + 1;
                FD_SET(sockets[i], &rfds);
            }
        }
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        select(max, &rfds, NULL, NULL, &tv);
        if (FD_ISSET(sockets[0], &rfds)) {
            socklen_t addr_len = sizeof(addr);
            int client;
            memset(&addr, 0, sizeof(addr));
            client = accept(sockets[0], &addr, &addr_len);
//            if (client == -1)
//                continue;
            if (client != -1) {
                // assign the connection an index in the socket array--indexes 0 and 1 are reserved
                for (i = 2; i < MAX_CLIENTS; i++) {
                    if (sockets[i] == -1) {
                        sockets[i] = client;		    
                        break;
                    }
                }
                if (i == MAX_CLIENTS) {
                    // too many open connections 
                    close(client);
                }
            }
        }
        
        // Process sockets
        // Socket index 0 is reserved for the server--do not read from or write to it
        // Socket index 1 is reserved for the tcp uplink--do not echo to it
        for (i = 1; i < MAX_CLIENTS; i++) {
            if (sockets[i] >= 0 && FD_ISSET(sockets[i], &rfds)) {
                len = read(sockets[i], buff, sizeof(buff));
                if (len <= 0) {
                    close(sockets[i]);
                    sockets[i] = -1;
                    check_exit(&tty);
                } else {
        #if 0
                    usleep(1000000 * slot_len / baud * len);
        #endif
        #if 0
                    {
                        struct timeval current_time;
                        gettimeofday(&current_time, NULL);
                        if (next_time.tv_sec > current_time.tv_sec
                            || (next_time.tv_sec == current_time.tv_sec &&
                                next_time.tv_usec > current_time.tv_usec)) {
                            usleep((next_time.tv_sec - current_time.tv_sec)
                                   * 1000000
                                   + next_time.tv_usec - current_time.tv_usec);
                        }
                        gettimeofday(&current_time, NULL);
                        next_time = current_time;
                        next_time.tv_usec += 1000000 * slot_len * len / baud;
                        if (next_time.tv_usec > 1000000) {
                            next_time.tv_sec++;
                            next_time.tv_usec -= 1000000;
                        }
                    }
        #endif
                    // write to tty downlink
                    if (tty.fd != BADFILE) {
                        rcx_nbwrite(&tty, buff, len, timeout);
                        //lnp_logical_write(buff, len);
                    }
                    // write to other sockets
                    for (j = 1; j < MAX_CLIENTS; j++) {
                        if (i != j && sockets[j] >= 0) {
                            if (write(sockets[j], buff, len) < 0) {
                                close(sockets[j]);
                                sockets[j] = -1;
                                check_exit(&tty);
                            }
                        }
                    }
                    // handle the echo on the uplink socket
                    if (sockets[1] >= 0) {
                        len_echo = rcx_nbread(&tty_tcp_uplink, buff_echo, len, timeout);
                    }
                    
                    // echo back to self if not the uplink socket
                    if ((i != 1) && (write(sockets[i], buff, len) < 0)) {
                        close(sockets[i]);
                        sockets[i] = -1;
                        check_exit(&tty);
                    }
                }
            }
        } // end socket loop
        
        // Process tty downlink
        // NOTE: do NOT echo to the tty downlink
        if (tty.fd != BADFILE) {
            len = rcx_nbread(&tty, buff, sizeof(buff), timeout);
            if (len > 0) {
                // write to sockets
                for (j = 1; j < MAX_CLIENTS; j++) {
                    if (sockets[j] >= 0) {
                        if (write(sockets[j], buff, len) < 0) {
                            close(sockets[j]);
                            sockets[j] = -1;
                            check_exit(&tty);
                        }
                    }
                }
                // handle the echo on the uplink socket
                if (sockets[1] >= 0) {
                    len_echo = rcx_nbread(&tty_tcp_uplink, buff_echo, len, timeout);
                }
            }
        }
    }
}
