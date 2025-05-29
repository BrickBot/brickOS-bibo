/*
 *  rcx_comm.h
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

#ifndef RCX_COMM_H_INCLUDED
#define RCX_COMM_H_INCLUDED

#include "config.h"

#if defined(_WIN32)
  #include <windows.h>
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

#define RCX_OK             0
#define RCX_NO_TOWER      -1
#define RCX_BAD_LINK      -2
#define RCX_BAD_ECHO      -3
#define RCX_NO_RESPONSE   -4
#define RCX_BAD_RESPONSE  -5

#if defined(_WIN32)
  #define FILEDESCR	HANDLE
  #define BADFILE	NULL
#else
  #define FILEDESCR	int
  #define BADFILE	-1
#endif

// port settings
//
#define TTY_VARIABLE	"RCXTTY"	//!< environment variable to override defaults

#if defined(linux) || defined(LINUX)
#define DEFAULTTTY   "/dev/ttyS0"	//!< Linux  - COM1
#elif defined(_WIN32)
#define DEFAULTTTY   "com1"	      	//!< Cygwin - COM1
#elif defined(sparc)
#define DEFAULTTTY   "/dev/ttyb"	//!< Sun - serial port B
#else
#define DEFAULTTTY   "/dev/ttyd2"	//!< IRIX - second serial port
#endif

#define RCX_BUFFERSIZE      4096
#define RCX_RETRIES         10

#define RCX_KEEPALIVE_BYTE  0xFF
#define RCX_WAKEUP_MESSAGE  { 0x10, 0xfe, 0x10, 0xfe }

// Timeouts are in thousandths of a second
#define RCX_WAKEUP_TIMEOUT  4000    //!< Serial IR tower wakeup timeout
#define RCX_KEEPALIVE_INTERVAL 4    //!< Serial IR tower keepalive interval in seconds
#define RCX_DEFAULT_TIMEOUT 250

#define RCX_DEFAULT_BAUD    2400

#if defined(_WIN32)
#define RCX_PLATFORM_DEVICE_OPTIONS \
	"                 serial:<device name>  (e.g. serial:COM1)\n" \
	"                 usb[:<device name>]   (e.g. usb or usb:\\\\\\\\.\\\\legotower1)\n"
#else
#define RCX_PLATFORM_DEVICE_OPTIONS \
	"                 serial:<device name>  (e.g. serial:/dev/ttyS0)\n" \
	"                 usb[:<device name>]   (e.g. usb:" DEFAULT_USB_TOWER_NAME_LINUX ")\n"
#endif 

#define RCX_COMM_OPTIONS	"Options:\n" \
	"  -t<device>   , --tty=<device>        set the IR device to <device>\n" \
	"    <device> may take one of the following formats:\n" \
    RCX_PLATFORM_DEVICE_OPTIONS \
	"                 tcp[:<host>[:<port>]] (e.g. tcp:localhost:" DEFAULT_IR_SERVER_BROADCAST_PORT_AS_STRING ")\n" \
	"                 ncd[:<host>[:<port>]] (e.g. tcp:localhost:87)\n" \
    "                                       default host:port is localhost:" DEFAULT_IR_SERVER_BROADCAST_PORT_AS_STRING "\n"\
    "                 com:<device name>     non-echo, non-keepalive serial device\n" \
    "                                       (such as bluetooth)\n"\
	"  -b<rate>     , --baud=<rate>         baud rate (e.g. 2400, 4800)\n" \
	"  -o<timeout>  , --timeout=<mseconds>  timeout in milliseconds\n" \
	"  -v           , --verbose             verbose mode\n" \
	"  -h           , --help                display this help and exit\n"

#define RCX_COMM_OPTIONS_INFO \
	"Default tty device can be set using one of the following methods,\n" \
	"listed in order of precedence:\n" \
	" * Environment variable RCXTTY.\n" \
	"      Eg:\tset RCXTTY=tcp:localhost:" DEFAULT_IR_SERVER_BROADCAST_PORT_AS_STRING "\n" \
	" * Configuration file ~/.rcx/device.conf\n" \
	" * Configuration file /etc/rcx/device.conf\n" \
	"      Specify <device> as the first line of the conf file\n"


/* Cygwin, in addition to NAN, doesn't have cfmakeraw */
#if defined(__CYGWIN__) & ! defined(_WIN32)
extern void cfmakeraw(struct termios *);
#endif /* defined(__CYGWIN__) */


// Bitmask the tty types
typedef enum {  tty_t_undefined = 0,
                tty_t_serial = 1,
                tty_t_usb = 2,
                tty_t_tcp = 4,
                tty_t_ncd = 8,
                tty_t_com = 16, // serial connection without the IR tower and the requisite wakeup/keepalive transmissions
                tty_types_echo = tty_t_serial | tty_t_ncd,   // or types together
                tty_types_tcp = tty_t_tcp | tty_t_ncd,
                tty_types_local_serial = tty_t_serial | tty_t_com,
                tty_types_keepalive = tty_t_serial | tty_t_ncd
             } tty_type_t;

// tty information
typedef struct {
    FILEDESCR fd;
    tty_type_t type;
} tty_t;

/* Set debug mode flag */
extern void rcx_set_debug (int comm_debug);

/* Get a file descriptor for the named tty, exits with message on error */
/* If true, start_keepalive starts the keepalive timer for tower types needing that functionality */
extern void rcx_init (tty_t *tty, const char *tty_name, int baud, int timeout, int start_keepalive);

#if ! ( ( defined(_WIN32) & ( (! defined(__CYGWIN__)) | defined(__MINGW32__) ) ) )
// initialize RCX communications port via tcp
void rcx_init_tcp(tty_t *tty, const char *tty_name, int baud);
#endif

/* Close a file descriptor allocated by rcx_init */
extern void rcx_close (tty_t *tty);

/* Try to wakeup the tower for timeout ms, returns error code; skips wakeup for tower types that do not need woken up */
extern int rcx_wakeup_tower (tty_t *tty, int timeout, int wakeup_timeout);

/* Initialize the IR tower keepalive timer; does nothing if tty is not a keepalive type */
void rcx_keepalive_init(tty_t *tty);

/* Reset the IR tower keepalive timer; does nothing if tty is not a keepalive type */
extern void rcx_keepalive_reset (tty_t *tty);

/* Send a keepalive, resetting the keepalive timer; does nothing if tty is not a keepalive type */
extern void rcx_keepalive_send(tty_t *tty);

/* Turn off the IR tower keepalive timer; does nothing if tty is not a keepalive type */
extern void rcx_keepalive_end(tty_t *tty);

/* Try to send a message, returns error code */
/* Set use_comp=1 to send complements, use_comp=0 to suppress them */
extern int rcx_send (tty_t *tty, void *buf, int len, int timeout, int use_comp);

/* Try to receive a message, returns error code */
/* Set use_comp=1 to expect complements */
/* Waits for timeout ms before detecting end of response */
extern int rcx_recv (tty_t *tty, void *buf, int maxlen, int timeout, int use_comp);

/* Perform lnp_integrity_byte handling on read operations */
extern int rcx_recv_lnp(tty_t *tty, void *buf, int maxlen, int timeout);

/* Try to send a message and receive its response, returns error code */
/* Set use_comp=1 to send and receive complements, use_comp=0 otherwise */
/* Waits for timeout ms before detecting end of response */
extern int rcx_sendrecv (tty_t *tty, void *send, int slen, void *recv, int rlen, int timeout, int retries, int use_comp);

/* Test whether or not the rcx is alive, returns 1=yes, 0=no */
/* Set use_comp=1 to send complements, use_comp=0 to suppress them */
extern int rcx_is_alive (tty_t *tty, int timeout, int use_comp);

/* Convert an error code to a string */
extern char *rcx_strerror(int error);

/* perror() wrapper */
void rcx_perror(char *str);

/* Hexdump routine */
extern void rcx_hexdump(char *prefix, const void *data, unsigned length, char show_ASCII);

/* Timeout read routine */
int rcx_nbread(tty_t *tty, void *buf, int maxlen, int timeout);

// Write with timeout and echo check
int rcx_nbwrite(tty_t *tty, void *buf, int len, int timeout);

#endif /* RCX_COMM_H_INCLUDED */

