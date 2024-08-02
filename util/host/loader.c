/*! \file   loader.c
    \brief  BrickOS task downloading
    \author Markus L. Noga <markus@noga.de>
*/

/*
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
 *  The Original Code is legOS code, released October 2, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): everyone discussing LNP at LUGNET
 */

/*  2000.05.01
 *
 *  Modifications to the original loader.c file for LegOS 0.2.2 include:
 *
 *  Paolo Masetti's patches to eliminate "Invalid Argument" error and
 *  to get rid of several compiler warnings:
 *     <paolo.masetti@itlug.org>
 *     http://www.lugnet.com/robotics/rcx/legos/?n=619
 *
 *  Markus L. Noga's solution to Cygwin's failure to define the O_ASYNC symbol:
 *     <markus@noga.de>
 *     http://www.lugnet.com/robotics/rcx/legos/?n=439
 *
 *  Paolo Masetti's adaptation for definitive porting to Win32. No more errors in
 *  serial communication and conditional compile for Winnt (cygnwin).
 *  Several addition to dll option to better support user. Execute dll without
 *  arguments to get help.
 *     <paolo.masetti@itlug.org>
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#if defined(_WIN32)
  #include <windows.h>
#endif

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#include <sys/lnp.h>
#include <sys/lnp-logical.h>

#include "rcx_comm.h"
#include <lx.h>

#define MAX_DATA_CHUNK 0xf8   	  //!< maximum data bytes/packet for boot protocol
#define XMIT_RETRIES   5      	  //!< number of packet transmit retries
//#define BYTE_TIME      (1000*LNP_BYTE_TIME) //!< time to transmit a byte.
#define BYTE_TIME      5         //!< milliseconds to transmit a byte (default to 2400 baud timing; non-critical, only used in a timeout calculation).

#define PROG_MIN	1
#define PROG_MAX	8

#define ADDR_MIN	0
#define ADDR_MAX	15

#define DEFAULT_DEST  	0
#define DEFAULT_PROGRAM	PROG_MIN
#define DEFAULT_SRCPORT 0
#define DEFAULT_PRIORITY 10

typedef enum {
  CMDacknowledge,     		//!< 1:
  CMDdelete, 	      	//!< 1+ 1: b[nr]
  CMDcreate, 	      	//!< 1+12: b[nr] s[textsize] s[datasize] s[bsssize] s[stacksize] s[start] b[prio]
  CMDoffsets, 	      	//!< 1+ 7: b[nr] s[text] s[data] s[bss]
  CMDdata,   	      	//!< 1+>3: b[nr] s[offset] array[data]
  CMDrun,     	      	//!< 1+ 1: b[nr]
  CMDirmode,			//!< 1+ 1: b[0=near/1=far]
  CMDsethost,			//!< 1+ 1: b[hostaddr]
  CMDlast     	      	//!< ?
} packet_cmd_t;

#if (defined(__sun__) && defined(__svr4__)) || defined(BSD)	// Solaris||BSD
#undef HAVE_GETOPT_LONG
#else
#define HAVE_GETOPT_LONG
#endif

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>

static const struct option long_options[]={
  {"program", required_argument,0,'p'},
  {"delete",  required_argument,0,'d'},
  {"destaddr",required_argument,0,'a'},
  {"destport",required_argument,0,'r'},
  {"tty",     required_argument,0,'t'},
  {"baud",    required_argument,0,'b'},
  {"timeout", required_argument,0,'o'},
  {"irmode",  required_argument,0,'i'},
  {"node",    required_argument,0,'n'},
  {"execute", no_argument      ,0,'e'},
  {"help",    no_argument      ,0,'h'},
  {"verbose", no_argument      ,0,'v'},
  {0         ,0                ,0,0  }
};

#else // HAVE_GETOPT_LONG

#define getopt_long(ac, av, opt, lopt, lidx) (getopt((ac), (av), (opt)))

#endif // HAVE_GETOPT_LONG

volatile int receivedAck=0;

volatile unsigned short relocate_to=0;

unsigned int  rcxaddr = DEFAULT_DEST,
              prog    = DEFAULT_PROGRAM,
              srcport = DEFAULT_SRCPORT,
              hostaddr = CONF_LNP_HOSTADDR,
		      irmode  = -1;
  
int verbose_flag=0;


//! send a LNP layer 0 packet of given length
/*! \return 0 on success.
*/
int lnp_assured_write(tty_t *tty, const unsigned char *data, unsigned char length,
                      int timeout, unsigned char dest, unsigned char srcport) {
  int i;
  struct timeval tv_timeout,tv_now;
  unsigned long total,elapsed;
  unsigned char buffer[RCX_BUFFERSIZE];
  
  for(i=0; i<XMIT_RETRIES; i++) {
    receivedAck=0;
    
    lnp_addressing_write(data,length,dest,srcport);
    
    gettimeofday(&tv_timeout,0);
    total = (timeout * 1000) + (length * BYTE_TIME);
    elapsed=0;

    do {
      rcx_recv_lnp(tty, buffer, sizeof(buffer), timeout);

      gettimeofday(&tv_now,0);
      elapsed=(1000000*(tv_now.tv_sec  - tv_timeout.tv_sec )) +
	       	       tv_now.tv_usec - tv_timeout.tv_usec;
      
    } while((!receivedAck) && (elapsed < total));
    
    if(i || !receivedAck)
      if(verbose_flag)
        fprintf(stderr,"try %d: ack:%d\n",i,receivedAck);
    
    if(receivedAck)
      return 0;    
   }
  
  return -1;
}

    
void ahandler(const unsigned char *data,unsigned char len,unsigned char src) {
  if(*data==CMDacknowledge) {
    receivedAck=1;
    if(len==8) {
      // offset packet
      //
      relocate_to=(data[2]<<8)|data[3];
    }
  }
}
    
void lnp_download(tty_t *tty, const lx_t *lx, int timeout) {
  unsigned char buffer[256+3];
  
  size_t i,chunkSize,totalSize=lx->text_size+lx->data_size;

  if(verbose_flag)
    fputs("\ndata ",stderr);

  buffer[0]=CMDdata;
  buffer[1]=prog-1;

  for(i=0; i<totalSize; i+=chunkSize) {
    chunkSize=totalSize-i;
    if(chunkSize>MAX_DATA_CHUNK)
      chunkSize=MAX_DATA_CHUNK;

    buffer[2]= i >> 8;
    buffer[3]= i &  0xff;
    memcpy(buffer+4,lx->text + i,chunkSize);
    if(lnp_assured_write(tty,buffer,chunkSize+4,timeout,rcxaddr,srcport)) {
      fputs("error downloading program\n",stderr);
      exit(-1);
    }
  }
}

int main(int argc, char **argv) {
  tty_t tty = { BADFILE, tty_t_undefined };
  lx_t lx;    	    // the brickOS executable
  char *filename;
  int opt;
#ifdef HAVE_GETOPT_LONG
  int option_index;
#endif
  unsigned char buffer[256+3]="";
  char *tty_name=NULL;
  
  // command options
  int usage = 0;
  int run_flag = 0;
  int baud = RCX_DEFAULT_BAUD;
  int timeout = RCX_DEFAULT_TIMEOUT;
  int pdelete_flag = 0;
  int hostaddr_flag = 0;

  while((opt=getopt_long(argc, argv, "p:d:a:r:t:b:o:i:n:ehv",
                        (struct option *)long_options, &option_index) )!=-1) {
    switch(opt) {
      case 'b':
        baud = atoi(optarg);
        break;
      case 'e':
        run_flag=1;
        break;
      case 'a':
        sscanf(optarg,"%d",&rcxaddr);
		if (rcxaddr > ADDR_MAX || rcxaddr < ADDR_MIN) {
			fprintf(stderr, "LNP host address not in range 0..15\n");
			return -1;
		}
		rcxaddr = (rcxaddr << 4) & CONF_LNP_HOSTMASK;
        break;
      case 'o':
        timeout = atoi(optarg);
        break;
      case 'r':
        sscanf(optarg,"%d",&srcport);
		if (srcport > ADDR_MAX || srcport < ADDR_MIN) {
			fprintf(stderr, "LNP port number not in range 0..15\n");
			return -1;
		}
        break;
      case 'p':
        sscanf(optarg,"%d",&prog);
        break;
      case 't':
        tty_name = optarg;
        break;
      case 'i':
        sscanf(optarg,"%x",&irmode);
        break;
      case 'd':
        sscanf(optarg,"%d",&prog);
        pdelete_flag=1;
        break;
      case 'n':
        sscanf(optarg,"%d",&hostaddr);
		if (hostaddr > ADDR_MAX || hostaddr < ADDR_MIN) {
			fprintf(stderr, "LNP host address not in range 0..15\n");
			return -1;
		}
        hostaddr_flag=1;
        break;
      case 'h':
        usage=1;
        break;
      case 'v':
        verbose_flag=1;
        rcx_set_debug(1);
        break;
    }
  }           
  
  if (prog > PROG_MAX || prog < PROG_MIN) {
    fprintf(stderr, "Program not in range 1..8\n");
    return -1;
  }

  // load executable
  //      
  if(usage ||
     ((argc-optind < 1) && !(pdelete_flag || hostaddr_flag)) ||
	 ((argc-optind > 0 ) && (pdelete_flag || hostaddr_flag)))
  {
    char *usage_string =
    RCX_COMM_OPTIONS
	"  -p<prognum>  , --program=<prognum>   set destination program to <prognum>\n"
	"  -a<destaddr> , --destaddr=<destaddr> send to RCX host address <destaddr>\n"
	"  -r<destport> , --destport=<destport> send to RCX source port <destport>\n"
	"  -i<0/1>      , --irmode=<0/1>        set IR mode near(0)/far(1) on RCX\n"
	"  -e           , --execute             execute program after download\n"
    "\n"
	"Commands:\n"
	"  -d<prognum>  , --delete=<prognum>    delete program <prognum> from memory\n"
	"  -n<hostaddr> , --node=<hostaddr>     set LNP host address in brick\n"
	"\n"
    RCX_COMM_OPTIONS_INFO
	"\n"
	;

	fprintf(stderr, "usage: %s [options] [command | file.lx]\n", argv[0]);
	fputs(usage_string, stderr);

    return -1;
  }

  // Ignore filename if -dn or -na given
  if (!(pdelete_flag || hostaddr_flag)) {
    filename=argv[optind++];
    if(lx_read(&lx,filename)) {
      fprintf(stderr,"unable to load brickOS executable from %s.\n",filename);
      return -1;
	}
  }

  rcx_init(&tty, tty_name, baud, timeout, FALSE);

  if (verbose_flag)
    fputs("\nLNP Initialized...\n", stderr);

  lnp_addressing_set_handler(0, ahandler);

  if(verbose_flag)  
    fprintf(stderr, "loader hostaddr=0x%02x hostmask=0x%02x portmask=0x%02x\n",
            rcxaddr & 0x00ff, LNP_HOSTMASK & 0x00ff, srcport & 0x00ff);

  // Set IR mode
  if (irmode != -1) {
    buffer[0]=CMDirmode;
    buffer[1]=irmode;
    if(lnp_assured_write(&tty,buffer,2,timeout,rcxaddr,srcport)) {
      fputs("error setting IR mode to far\n", stderr);
      return -1;
    }
  }
  
  // Set LNP host address
  if (hostaddr_flag) {
    if(verbose_flag)
	  fputs("\nset LNP host address", stderr);
	buffer[0] = CMDsethost;
	buffer[1] = hostaddr;
	if(lnp_assured_write(&tty,buffer,2,timeout,rcxaddr,srcport)) {
      fputs("error setting host address\n", stderr);
      return -1;
	}
	fprintf(stderr, "LNP host address set to %d\n", hostaddr);
	return 0;
  }
  
  if(verbose_flag)
    fputs("\ndelete",stderr);
  buffer[0]=CMDdelete;
  buffer[1]=prog-1; //       prog 0
  if(lnp_assured_write(&tty,buffer,2,timeout,rcxaddr,srcport)) {
    fputs("error deleting program\n", stderr);
    return -1;
  }

  // All done if -dn given
  if (pdelete_flag) {
    fprintf(stderr, "P%d deleted\n", prog);
	return 0;
  }

  if(verbose_flag)
    fputs("\ncreate ", stderr);
  buffer[ 0]=CMDcreate;
  buffer[ 1]=prog-1; //       prog 0
  buffer[ 2]=lx.text_size>>8;
  buffer[ 3]=lx.text_size & 0xff;
  buffer[ 4]=lx.data_size>>8;
  buffer[ 5]=lx.data_size & 0xff;
  buffer[ 6]=lx.bss_size>>8;
  buffer[ 7]=lx.bss_size & 0xff;
  buffer[ 8]=lx.stack_size>>8;
  buffer[ 9]=lx.stack_size & 0xff;
  buffer[10]=lx.offset >> 8;  	// start offset from text segment
  buffer[11]=lx.offset & 0xff; 
  buffer[12]=DEFAULT_PRIORITY;
  if(lnp_assured_write(&tty,buffer,13,timeout,rcxaddr,srcport)) {
    fputs("error creating program\n", stderr);
    return -1;
  }

  // relocation target address in relocate_to
  //
  lx_relocate(&lx,relocate_to);

  lnp_download(&tty, &lx, timeout);

  fprintf(stderr, "\n");

  if (run_flag) {
    if(verbose_flag)
      fputs("\nrun ",stderr);
    buffer[0]=CMDrun;
    buffer[1]=prog-1; //       prog 0
    if(lnp_assured_write(&tty,buffer,2,timeout,rcxaddr,srcport)) {
      fputs("error running program\n", stderr);
      return -1;
    }
  }
      
  return 0;
}
