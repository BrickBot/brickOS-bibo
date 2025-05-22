/*! \file   lnp.c
    \brief  Implementation: link networking protocol
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
 *  The Original Code is legOS code, released October 17, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 */

/*
 *  2000.05.01 - Paolo Masetti <paolo.masetti@itlug.org>
 *
 *  - IR lcd now reflect IR mode (near/far)
 *
 *  2001.09.10 - Zhengrong Zang <mikezang@iname.com>
 *
 *  - Remote control buttons
 *  - Standard firmware async message
 *
 *  2002.04.23 - Ted Hess <thess@kitschensync.net>
 *
 *  - Integrate Ross Crawford/Zhengrong Zang remote control message parser
 *    and RCX op-code dispatcher
 *
 * 2009.05.09-23 - Matthew Sheets
 *
 * - Added LNP addressing sniffer handler
 * - Added standard firmware message handler
 * - Added ability to send "remote" codes
 * - Enabled some CONF_RCX_MESSAGE functionality when CONF_HOST is defined
 */

#include <sys/lnp.h>

#ifdef CONF_LNP

#include <sys/lnp-logical.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef CONF_RCX_MESSAGE
#include <sys/tm.h>
#include <sys/waitqueue.h>
#endif

#ifdef CONF_VIS
#include <dlcd.h>
#include <sys/lcd.h>
#endif

#ifndef __TEXT_HI__
#define __TEXT_HI__
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Internal Types
//
///////////////////////////////////////////////////////////////////////////////

//! The supported LNP modes
//! The enum value indicates the number of bytes the message size is increased
typedef enum {
  lnp_integrity_mode = 0,
  lnp_addressing_mode = 2
} lnp_mode_t;

///////////////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////////////

//! LNP host address (may be changed by dll utility --node=a option)
//! Default vaule in config.h may be overidden at compile type
unsigned char lnp_hostaddr = (CONF_LNP_HOSTADDR << 4) & CONF_LNP_HOSTMASK;

//! the integrity layer state
lnp_integrity_state_t lnp_integrity_state;

//! there are no ports for integrity layer packets, so just one handler.
volatile lnp_integrity_handler_t lnp_integrity_handler = LNP_DUMMY_INTEGRITY;

//! addressing layer packet sniffer
volatile lnp_addressing_sniffer_handler_t lnp_addressing_sniffer_handler = LNP_DUMMY_ADDRESSING_SNIFFER;

//! addressing layer packets may be directed to a variety of ports.
/*! FIXME: uninitialized
    FIXME: inefficient if LNP_PORT_MASK doesn't adhere to 0...01...1 scheme.
*/
volatile lnp_addressing_handler_t lnp_addressing_handler[LNP_PORTMASK+1];

#if !defined(CONF_MM)
// Max message length + 1 packet byte + 1 length byte + 1 dest addr byte (opt)
// + 1 source addr byte (opt) + 1 checksum byte
static unsigned char lnp_buffer[LNP_MAX_MSG_LEN + 5];
#endif // CONF_MM

#if defined(CONF_RCX_PROTOCOL)
//! remote handler
volatile lnp_remote_handler_t lnp_remote_handler = LNP_DUMMY_REMOTE;

//! checksum from RCX protocol
unsigned char lnp_rcx_checksum;

#endif

#if defined(CONF_RCX_MESSAGE) || defined(CONF_HOST)
//! standard firmware message handler
volatile lnp_message_handler_t lnp_message_handler = LNP_DUMMY_MESSAGE;
#endif

#if defined(CONF_RCX_MESSAGE)
//! message from standard firmware
unsigned char lnp_rcx_message;
static waitqueue_t *msg_waitqueue = NULL;   //!< wait queue for message

#endif

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

#define lnp_checksum_init(sum)  (unsigned char)((sum) = 0xff)
#define lnp_checksum_step(sum,d)  (unsigned char)((sum) += (d))

#ifdef CONF_HOST
__TEXT_HI__ unsigned char lnp_checksum_copy( unsigned char *dest,
					     const unsigned char *data, 
					     unsigned length )
{
  unsigned char a = 0xff;
  unsigned char t;

  do {
    t = *data++;
    a += t;
    *dest++ = t;
    length--;
  } while (length > 0);

  return a;
}
#else
__asm__(
	".text\n"
	"_lnp_checksum_copy:\n"
	";; r0: dest, r1: data, r2: length;\n"
	
	"    add.w r0,r2         ; r2: end \n"
	"    mov.b #0xff,r3l     ; r3l: a  \n"
	
	"0:\n"
	"    mov.b @r1+,r3h      ; r3h = *data++ \n"
	"    add.b r3h,r3l       ; a += r3h      \n"
	"    mov.b r3h,@r0       ; *dest++ = r3h \n"
	"    adds  #1,r0 \n"
	"    cmp.w r0,r2 \n"
	"    bne   0b \n"
	
	"    sub.w r0,r0 \n"
	"    mov.b r3l,r0l \n"
	"    rts \n"
	);
#endif

//! send a LNP integrity layer packet of the given length
/*! \return 0 on success.
*/
__TEXT_HI__ int lnp_integrity_write(const unsigned char *data,unsigned char length) {
  int r;
#if defined(CONF_MM)
  unsigned char* buffer_ptr = malloc(length+3);
#else // CONF_MM
  unsigned char* buffer_ptr = lnp_buffer;
#endif // CONF_MM
  unsigned char c = lnp_checksum_copy( buffer_ptr+2, data, length);
  lnp_checksum_step( c, buffer_ptr[0]=0xf0 );
  lnp_checksum_step( c, buffer_ptr[1]=length );
  buffer_ptr[length+2] = c;
  r = lnp_logical_write(buffer_ptr,length+3);
#if defined(CONF_MM)
  free(buffer_ptr); 
#endif // CONF_MM
  return r;
}

//! send a LNP addressing layer packet of given length
/*! \return 0 on success.
*/
__TEXT_HI__ int lnp_addressing_write(const unsigned char *data,unsigned char length,
                         unsigned char dest,unsigned char srcport) {
  int r;
#if defined(CONF_MM)
  unsigned char* buffer_ptr = malloc(length+5);
#else // CONF_MM
  unsigned char* buffer_ptr = lnp_buffer;
#endif // CONF_MM
  unsigned char c = lnp_checksum_copy( buffer_ptr+4, data, length );
  lnp_checksum_step( c, buffer_ptr[0]=0xf1 );
  lnp_checksum_step( c, buffer_ptr[1]=length+2 );
  lnp_checksum_step( c, buffer_ptr[2]=dest );
  lnp_checksum_step( c, buffer_ptr[3]=
                   (lnp_hostaddr | (srcport & LNP_PORTMASK)) );
  buffer_ptr[length+4] = c;
  r = lnp_logical_write(buffer_ptr,length+5);
#if defined(CONF_MM)
  free(buffer_ptr); 
#endif // CONF_MM
  return r;
}

#if defined(CONF_PRINTF) || defined(CONF_HOST)
//! send a LNP integrity layer packet in "printf" fashion
/*! \return 0 on success.
*/
__TEXT_HI__ int lnp_integrity_printf(const char *fmt, ...)
{
  int len, writeResult;
  va_list arg;
#if defined(CONF_MM)
  unsigned char* buf = malloc(LNP_MAX_MSG_LEN);
#else // CONF_MM
  unsigned char* buf = lnp_buffer;
#endif // CONF_MM

  va_start(arg, fmt);
  len = vsnprintf((char*)buf, LNP_MAX_MSG_LEN, fmt, arg);
  va_end(arg);

  writeResult = lnp_integrity_write(buf, len);
  
#if defined(CONF_MM)
  free(buf);
#endif // CONF_MM

  return writeResult;
}

//! send a LNP addressing layer packet in "printf" fashion
/*! \return 0 on success.
*/
__TEXT_HI__ int lnp_addressing_printf(unsigned char dest,
        unsigned char srcport, const char *fmt, ...)
{
  int len, writeResult;
  va_list arg;
#if defined(CONF_MM)
  unsigned char* buf = malloc(LNP_MAX_MSG_LEN);
#else // CONF_MM
  unsigned char* buf = lnp_buffer;
#endif // CONF_MM

  va_start(arg, fmt);
  len = vsnprintf((char*)buf, LNP_MAX_MSG_LEN, fmt, arg);
  va_end(arg);

  writeResult = lnp_addressing_write(buf, len, dest, srcport);
  
#if defined(CONF_MM)
  free(buf);
#endif // CONF_MM

  return writeResult;
}
#endif // CONF_PRINTF

//! handle LNP packet from the integrity layer
void lnp_receive_packet(const unsigned char *data) {
  unsigned char header=*(data++);
  unsigned char length=*(data++);

  // only handle non-degenerate packets in boot protocol 0xf0
  //
  switch(header) {
    case 0xf0:        // raw integrity layer packet, no addressing.
      if(lnp_integrity_handler) {
        lnp_integrity_handler(data, length);
      }
      break;

    case 0xf1:        // addressing layer.
      if(length>2) {
        unsigned char dest=*(data++);
        unsigned char src=*(data++);
        
        if(lnp_addressing_sniffer_handler) {
          lnp_addressing_sniffer_handler(data, length-2, src, dest);
        }

        if(lnp_hostaddr == (dest & LNP_HOSTMASK)) {
          unsigned char port=dest & LNP_PORTMASK;
          if(lnp_addressing_handler[port]) {
            lnp_addressing_handler[port](data, length-2, src);
          }
        }
      }

  } // switch(header)
}

//! receive a byte, decoding LNP packets with a state machine.
__TEXT_HI__ void lnp_integrity_byte(unsigned char b) {
    static unsigned char buffer[LNP_MAX_MSG_LEN+4];
    static int bytesRead,endOfData;
    static unsigned char chk;
    
    if (lnp_integrity_state == LNPwaitHeader)
	bytesRead=0;
    
    buffer[bytesRead++]=b;
    
    switch(lnp_integrity_state) {
    case LNPwaitHeader:
	// valid headers are 0xf0 .. 0xf7
	//   or RCX header 0x55
	if(((b & 0xf8) == 0xf0) || b == 0x55) {
#ifdef CONF_VIS
	    if (lnp_logical_range_is_far()) {
		dlcd_show(LCD_IR_UPPER);
		dlcd_show(LCD_IR_LOWER);
	    } else {
		dlcd_hide(LCD_IR_UPPER);
		dlcd_show(LCD_IR_LOWER);
	    }
#ifndef CONF_LCD_REFRESH
	    lcd_refresh();
#endif
#endif

	    if (b == 0x55) {
		// 0x55 is header for standard firmware message
		lnp_integrity_state = LNPwaitLH1;
	    } else {
		// Init checksum
		chk = 0xff + b;
	    
		// switch on protocol header
		lnp_integrity_state++;
	    }
	    
	}
	break;

    case LNPwaitLength:
	/* Accumulate checksum */
	chk += b;

	endOfData=b+2;
	lnp_integrity_state++;
	break;
	
    case LNPwaitData:
	/* Accumulate checksum */
	chk += b;
	if(bytesRead==endOfData)
	    lnp_integrity_state++;
	break;
	
    case LNPwaitCRC:
	if(b==chk) {
	    lnp_receive_packet(buffer);
	}
	lnp_integrity_reset();
	break;

	/* state machine to standard lego messages */
    case LNPwaitLH1:
	// waiting for header bytes
	if ( b == 0xff )
	    lnp_integrity_state++;
	else
	    lnp_integrity_reset();
	break;

    case LNPwaitLO1:
	lnp_integrity_state++;
	/* get packet length from opcode */
	endOfData = b & 7;
	if (endOfData >= 6)
	    endOfData -= 6;
	endOfData = bytesRead + 2 * endOfData;
	chk = b;
	break;

    case LNPwaitLH2:
    case LNPwaitLO2:
	if ( b != (buffer[bytesRead-2]^0xff) )
	    lnp_integrity_reset();
	else
	    lnp_integrity_state++;
	break;

    case LNPwaitLData:
	chk += b;
	lnp_integrity_state++;
	break;

    case LNPwaitLDataI:
	if ( b != (buffer[bytesRead-2]^0xff) )
	    lnp_integrity_reset();
	else if (bytesRead >= endOfData)
	    lnp_integrity_state++;
	else
	    lnp_integrity_state--;
	break;

    case LNPwaitLChksum:
	if (b != chk)
	    lnp_integrity_reset();
	else
	    lnp_integrity_state++;
	break;

    case LNPwaitLChksumI:
	if (b != (chk ^ 0xff)) {
#if defined(CONF_RCX_PROTOCOL)
	} else if (buffer[3] == 0xd2) {

#if defined(CONF_RCX_MESSAGE)
	    // if a message, set message number and exit
	    if (buffer[5] & 0x07) {
		lnp_rcx_message = (buffer[5] > 2) ? 3 : buffer[5];
		wakeup(msg_waitqueue);
	    }
#endif
	    // Invoke remote handler, if any
	    if (lnp_remote_handler)
            lnp_remote_handler( (buffer[5]<<8)+buffer[7] );
#endif
#if defined(CONF_RCX_MESSAGE) || defined(CONF_HOST)
	} else if (buffer[3] == 0xf7) {
#if defined(CONF_RCX_MESSAGE)
	    lnp_rcx_message = buffer[5];
	    wakeup(msg_waitqueue);
#endif
        // Invoke message handler, if any
        if (lnp_message_handler) {
            lnp_message_handler(buffer[5]);
        }
#endif
        }
	lnp_integrity_reset();
	break;
    }
}

//! reset the integrity layer on error or timeout.
__TEXT_HI__ void lnp_integrity_reset(void) {
  if(lnp_integrity_state!=LNPwaitHeader) {
    lnp_integrity_state=LNPwaitHeader;

#ifdef CONF_VIS
    dlcd_hide(LCD_IR_LOWER);
    dlcd_hide(LCD_IR_UPPER);
#ifndef CONF_LCD_REFRESH
    lcd_refresh();
#endif
#endif
  }
}

//! Initialise protocol handlers
/*! Addressing port 0 is reserved for the program handler.
*/
__TEXT_HI__ void lnp_init(void) {
  int k;
  
  lnp_integrity_handler=LNP_DUMMY_INTEGRITY;
  lnp_addressing_sniffer_handler=LNP_DUMMY_ADDRESSING_SNIFFER;
  for(k=1; k<=LNP_PORTMASK; k++)
    lnp_addressing_handler[k]=LNP_DUMMY_ADDRESSING;

#if defined(CONF_RCX_PROTOCOL)
  lnp_remote_handler=LNP_DUMMY_REMOTE;
#endif
#if defined(CONF_RCX_MESSAGE)
  clear_msg();
#endif
}

//! Shut down protocol handlers
/*! Adressing port 0 is reserved for the program handler.
*/
__TEXT_HI__ void lnp_shutdown(void) {
  // At the present, the code is identical to lnp_init
  lnp_init();
}

#if defined(CONF_RCX_MESSAGE) || defined(CONF_HOST)

//! send a standard firmware message
/*! \return 0 on success.
 */
__TEXT_HI__ int send_msg(unsigned char msg)
{
  unsigned char buffer_ptr[9];
  buffer_ptr[0]=0x55;
  buffer_ptr[1]=0xff;
  buffer_ptr[2]=0x00;
  buffer_ptr[3]=0xf7;
  buffer_ptr[4]=0x08;
  buffer_ptr[5]=msg;
  buffer_ptr[6]=(~msg);
  buffer_ptr[7]=(unsigned char) (0xf7+msg);
  buffer_ptr[8]=(~buffer_ptr[7]);
  return lnp_logical_write(buffer_ptr, sizeof(buffer_ptr));
}

//! send a remote message code
/*! \return 0 on success.
 */
__TEXT_HI__ int send_code(unsigned int code)
{
  unsigned char buffer_ptr[11];
  buffer_ptr[0]=0x55;
  buffer_ptr[1]=0xff;
  buffer_ptr[2]=0x00;
  buffer_ptr[3]=0xd2;
  buffer_ptr[4]=0x2d;
  buffer_ptr[5]=(unsigned char) (code >> 8);
  buffer_ptr[6]=(~buffer_ptr[5]);
  buffer_ptr[7]=(unsigned char) (0xff & code);
  buffer_ptr[8]=(~buffer_ptr[7]);
  buffer_ptr[9]=(unsigned char) (buffer_ptr[3]+buffer_ptr[5]+buffer_ptr[7]);
  buffer_ptr[10]=(~buffer_ptr[9]);
  return lnp_logical_write(buffer_ptr, sizeof(buffer_ptr));
}
#endif 

#ifdef CONF_RCX_MESSAGE
__TEXT_HI__ unsigned char get_msg(void)
{
  waitqueue_t entry;
  unsigned char result;

  grab_kernel_lock();
  add_to_waitqueue(&msg_waitqueue, &entry);
  wait();
  result = lnp_rcx_message;
  remove_from_waitqueue(&entry);
  release_kernel_lock();
  return result;
}

#endif

#endif // CONF_LNP
