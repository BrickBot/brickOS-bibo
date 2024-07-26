/*
 *  firmdl.c
 *
 *  A firmware downloader for the RCX.  Version 3.0.  Supports single and
 *  quad speed downloading.
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
 *                  Laurent Demailly
 *                  Allen Martin
 *                  Markus Noga
 *                  Gavin Smyth
 *                  Luis Villa
 *                  Taiichi added fflush(stderr) in function image_dl.
 */

/*
 *  usage: firmdl [options] srecfile  (e.g. firmdl Firm0309.lgo)
 *
 *  Acknowledgements:
 *
 *     Laurent Demailly, Allen Martin, Markus Noga, Gavin Smyth, and Luis
 *     Villa all contributed something to some version of this program.
 *
 *  Version history:
 *
 *     1.x: single speed downloading plus many small revisions
 *     2.0: double speed downloading, improved comm code, never released
 *     3.0: quad speed downloads, misc other features, version numbering
 *     3.1: fixed checksum algorithm, increased max file size
 *  Kekoa Proudfoot
 *  kekoa@graphics.stanford.edu
 *  10/3/98, 10/3/99
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
 * - Removed rcx wakeup tower calls; task is now included in rcx init
 *
 * Matthew Sheets - 2009-06-13
 * - Support greater flexibility in selecting the baud rate
 * - Enable non-complement-byte mode for other than 4800 baud
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32)
  #include <windows.h>
#else
  #include <termios.h>
#endif

#include "rcx_comm.h"
#include "srec.h"
#include "srecload.h"

#include "fastdl_2400.h"
#include "fastdl_4800.h"
#include "fastdl_9600.h"
#include "fastdl_19200.h"
#include "fastdl_38400.h"

/* Machine-dependent defines */

#if (defined(__sun__) && defined(__svr4__)) || defined(BSD)	// Solaris||BSD
#undef HAVE_GETOPT_LONG
#else
#define HAVE_GETOPT_LONG
#endif

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>

static const struct option long_options[]={
  {"tty",     required_argument,0,'t'},
  {"baud",    required_argument,0,'b'},
  {"timeout", required_argument,0,'o'},
  {"help",    no_argument      ,0,'h'},
  {"verbose", no_argument      ,0,'v'},
  {"standard",no_argument      ,0,'s'},
  {0         ,0                ,0,0  }
};

#else // HAVE_GETOPT_LONG

#define getopt_long(ac, av, opt, lopt, lidx) (getopt((ac), (av), (opt)))

#endif // HAVE_GETOPT_LONG

/* Global variables */

char *progname;

/* Defines */

#define TRANSFER_SIZE   200

/* Stripping zeros is not entirely legal if firmware expects trailing zeros */
/* Define FORCE_ZERO_STRIPPING to force zero stripping for all files */
/* Normally you do not want to do this */
/* Possibly useful only if you explicitly zero pad for OCX compatiblity */
/* Since zero stripping is okay for Firm0309.lgo, that is done automatically */

#if 0
#define FORCE_ZERO_STRIPPING
#endif

/* Functions */

// Returns 0 on success; non-zero on fail
int image_dl(tty_t *tty, unsigned char *image, int len, unsigned short start,
	      int timeout, int use_comp, char *filename)
{
    unsigned short cksum = 0;
    unsigned char send[RCX_BUFFERSIZE];
    unsigned char recv[RCX_BUFFERSIZE];
    int addr, index, size, i;

    /* Compute image checksum */
    int cksumlen = (start + len < 0xcc00) ? len : 0xcc00 - start;
    assert(len > 0);
    for (i = 0; i < cksumlen; i++)
        cksum += image[i];

    /* Delete firmware */
    send[0] = 0x65;
    send[1] = 0x01;
    send[2] = 0x03;
    send[3] = 0x05;
    send[4] = 0x07;
    send[5] = 0x0b;

    if (rcx_sendrecv(tty, send, 6, recv, 1, timeout, RCX_RETRIES, use_comp) != 1) {
        fprintf(stderr, "%s: delete firmware failed\n", progname);
        fflush(stderr);
        return 2;
    }

    /* Start firmware download */
    send[0] = 0x75;
    send[1] = (start >> 0) & 0xff;
    send[2] = (start >> 8) & 0xff;
    send[3] = (cksum >> 0) & 0xff;
    send[4] = (cksum >> 8) & 0xff;
    send[5] = 0;

    if (rcx_sendrecv(tty, send, 6, recv, 2, timeout, RCX_RETRIES, use_comp) != 2) {
        fprintf(stderr, "%s: start firmware download failed\n", progname);
        fflush(stderr);
        return 3;
    }

    /* Transfer data */
    fprintf(stderr, "\rTransferring \"%s\" to RCX...\n", filename);
    addr = 0;
    index = 1;
    for (addr = 0, index = 1; addr < len; addr += size, index++) {
        fprintf(stderr,"\r%3d%%        \r",(100*addr)/len);
        size = len - addr;
        send[0] = 0x45;
        if (index & 1)
            send[0] |= 0x08;
        if (size > TRANSFER_SIZE)
            size = TRANSFER_SIZE;
        else if (0)
            /* Set index to zero to make sound after last transfer */
            index = 0;
        send[1] = (index >> 0) & 0xff;
        send[2] = (index >> 8) & 0xff;
        send[3] = (size >> 0) & 0xff;
        send[4] = (size >> 8) & 0xff;
        memcpy(&send[5], &image[addr], size);
        for (i = 0, cksum = 0; i < size; i++)
            cksum += send[5 + i];
        send[size + 5] = cksum & 0xff;

        if (rcx_sendrecv(tty, send, size + 6, recv, 2, timeout, RCX_RETRIES,
                 use_comp) != 2 || recv[1] != 0) {
            fprintf(stderr, "%s: transfer data failed\n", progname);
            return 4;
        }
    }
    fputs("100%        \n",stderr);

    /* Unlock firmware */
    send[0] = 0xa5;
    send[1] = 76;    // 'L'
    send[2] = 69;    // 'E'
    send[3] = 71;    // 'G'
    send[4] = 79;    // 'O'
    send[5] = 174;   // '®'

    /* Use longer timeout so ROM has time to checksum firmware */
    if (rcx_sendrecv(tty, send, 6, recv, 26, timeout * 3, RCX_RETRIES, use_comp) != 26) {
        fprintf(stderr, "%s: unlock firmware failed\n", progname);
        return 5;
    }
    
    return 0;
}


int main (int argc, char **argv)
{
    unsigned char image[IMAGE_MAXLEN];
    unsigned short image_start;
    unsigned int image_len;
    char *filename = NULL;
    char *tty_name = NULL;
    int opt;
#ifdef HAVE_GETOPT_LONG
    int option_index;
#endif
    int baud = RCX_DEFAULT_BAUD;
    int timeout = RCX_DEFAULT_TIMEOUT;
    int use_complements = FALSE;
    int usage = 0;
    tty_t tty = { BADFILE, tty_t_undefined };
    int baud_modes[] = { 2400, 4800, 9600, 19200, 38400 };
    int dl_result = 0;
    
    // fastdl image properites, to be set based on selected baud rate
    unsigned char *fastdl_image = NULL;
    int fastdl_len = 0;
    unsigned short fastdl_start= 0;

    progname = argv[0];

    /* Parse command line */

    while((opt=getopt_long(argc, argv, "t:b:o:hv",
                     (struct option *)long_options, &option_index) )!=-1) {
        switch(opt) {
          case 'b':
            baud = atoi(optarg);
            break;
          case 'o':
            timeout = atoi(optarg);
            break;
          case 's':
            use_complements = TRUE;
            break;
          case 't':
            tty_name = optarg;
            break;
          case 'h':
            usage=1;
            break;
          case 'v':
            rcx_set_debug(1);
            break;
        }
    }

    if (usage || argc - optind != 1) {
        char *usage_string =
            RCX_COMM_OPTIONS
            "  -s           , --standard            standard mode download (2400 baud only)\n"
            "\n"
            RCX_COMM_OPTIONS_INFO
            "\n"
            ;

        fprintf(stderr, "usage: %s [options] filename\n", progname);
        fputs(usage_string, stderr);
        exit(1);
    }
    
    if (use_complements && 2400 != baud) {
        fprintf(stderr, "%s: baud rate must be 2400 for standard mode\n", progname);
        exit(1);
    }

    filename = argv[optind++];

    // Get the proper fastdl image based on the selected baud rate
    switch (baud) {
        case 2400:
            fastdl_image = fastdl_2400_image;
            fastdl_len   = fastdl_2400_len;
            fastdl_start = fastdl_2400_start;
            break;
        case 4800:
            fastdl_image = fastdl_4800_image;
            fastdl_len   = fastdl_4800_len;
            fastdl_start = fastdl_4800_start;
            break;
        case 9600:
            fastdl_image = fastdl_9600_image;
            fastdl_len   = fastdl_9600_len;
            fastdl_start = fastdl_9600_start;
            break;
        case 19200:
            fastdl_image = fastdl_19200_image;
            fastdl_len   = fastdl_19200_len;
            fastdl_start = fastdl_19200_start;
            break;
        case 38400:
            fastdl_image = fastdl_38400_image;
            fastdl_len   = fastdl_38400_len;
            fastdl_start = fastdl_38400_start;
            break;
    }

    // Load the s-record file
    image_len = srec_load(filename, image, IMAGE_MAXLEN, &image_start);

    // Initialize the tty type (and fd)
    rcx_init(&tty, tty_name, 2400, timeout, FALSE);	// Slow startup seems better with low batteries...
    if (BADFILE == tty.fd) {
        exit(1);
    } else {
        rcx_close(&tty);
    }
    
    if (tty_t_usb == tty.type && 2400 != baud) {
        fprintf(stderr, "%s: baud rate must be 2400 when using USB tower\n", progname);
        exit(1);
    }

    int currently_uses_complements;
    int i = 0;
    int max_mode = (tty_t_usb == tty.type ? 1 : sizeof(baud_modes)/sizeof(int));
    int found_rcx = FALSE;

    if(max_mode > 1) {
        fprintf(stdout, "Attempting to determine current baud rate...\n");
    }
    
    // Loop through the baud modes to try to find the current baud rate
    for (i = 0; i < max_mode; i++) {
        fprintf(stdout, "  ...checking %d baud\n", baud_modes[i]);
        rcx_init(&tty, tty_name, baud_modes[i], timeout, FALSE);
        if (BADFILE == tty.fd) {
            fprintf(stderr, "Failure attempting to open at baud rate %d\n", baud_modes[i]);
            exit(1);
        }

        // Check if alive, distinguishing between complement modes
        if (rcx_is_alive(&tty, timeout, TRUE)) {
            found_rcx = TRUE;
            currently_uses_complements = TRUE;
        } else if (rcx_is_alive(&tty, timeout, FALSE)) {
            found_rcx = TRUE;
            currently_uses_complements = FALSE;
        }
        
        if (found_rcx) {
            // set the fastdl firmware image properties
            fprintf(stdout, "Detected baud rate of %d, ", baud_modes[i]);
            if (currently_uses_complements) {
                fprintf(stdout, "using complement bytes.\n");
            } else {
                fprintf(stdout, "not using complement bytes.\n");
            }

            if (baud_modes[i] != baud || currently_uses_complements) {
                image_dl(&tty, fastdl_image, fastdl_len, fastdl_start, timeout, currently_uses_complements, "Fast Download Image");
            }

            break;
        } else {
            rcx_close(&tty);
        }
    }

    if (NULL == fastdl_image) {
        fprintf(stderr, "%s: no response from rcx\n", progname);
        exit(1);
    }

    if (use_complements && baud_modes[i] != 2400) {
        fprintf(stderr, "%s: reset the rcx to use standard mode\n", progname);
        exit(1);
    }

    // Switch to the selected IR mode
    rcx_close(&tty);
    fprintf(stdout, "Connecting at %d baud.\n", baud);
    rcx_init(&tty, tty_name, baud, timeout, FALSE);
    if (BADFILE == tty.fd) { exit(1); }

    // Download the firmware image in the selected IR mode
    dl_result = image_dl(&tty, image, image_len, image_start, timeout, use_complements, filename);

    if (BADFILE != tty.fd) { rcx_close(&tty); }
    
    return dl_result;
}
