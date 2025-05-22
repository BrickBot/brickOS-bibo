/*! \file   include/lnp/lnp.h
    \brief  LNP Interface: link networking protocol
    \author Markus L. Noga <markus@noga.de>
 */

/*
 *  The contents of this file are subject to the Mozilla Public License
 *  Version 1.0 (the "License"); you may not use this file except in
 *  compliance with the License. You may obtain a copy of the License
 *  at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 *  the License for the specific language governing rights and
 *  limitations under the License.
 *
 *  The Original Code is legOS code, released October 17, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 */

#ifndef __lnp_h__
#define __lnp_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>
#include <string.h>

#ifdef CONF_LNP

#ifdef CONF_RCX_MESSAGE
#include <unistd.h>
#endif

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

#define LNP_MAX_MSG_LEN  255    //!< Maximum LNP message length

//! the integrity layer packet handler type
/*! arguments are (data,length).
 */
typedef void (*lnp_integrity_handler_t) (const unsigned char *, unsigned char);

//! the addressing layer packet handler type
/*! arguments are (data,length,src_address).
 */
typedef void (*lnp_addressing_handler_t) (const unsigned char *, unsigned char, unsigned char);

//! the addressing layer packet handler type
/*! arguments are (data,length,src_address,dest_address).
 */
typedef void (*lnp_addressing_sniffer_handler_t) (const unsigned char *, unsigned char, unsigned char, unsigned char);

//! dummy integrity layer packet handler
#define LNP_DUMMY_INTEGRITY ((lnp_integrity_handler_t)0)

//! dummy addressing layer packet handler
#define LNP_DUMMY_ADDRESSING ((lnp_addressing_handler_t)0)

//! dummy addressing layer packet handler
#define LNP_DUMMY_ADDRESSING_SNIFFER ((lnp_addressing_sniffer_handler_t)0)

#ifdef CONF_RCX_PROTOCOL
//! handler for remote
/*! arguments are (buttonstate)
*/
typedef void (*lnp_remote_handler_t) (unsigned int);

//! dummy remote packet handler
#define LNP_DUMMY_REMOTE ((lnp_remote_handler_t)0)
#endif

#if defined(CONF_RCX_MESSAGE) || defined(CONF_HOST)
//! handler for standard firmware messages
/*! arguments are (message)
*/
typedef void (*lnp_message_handler_t) (unsigned char);

//! dummy remote packet handler
#define LNP_DUMMY_MESSAGE ((lnp_message_handler_t)0)
#endif

///////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////

//! there are no ports for integrity layer packets, so there's just one handler.
extern volatile lnp_integrity_handler_t lnp_integrity_handler;

//! addressing layer packets may be directed to a variety of ports.
extern volatile lnp_addressing_handler_t lnp_addressing_handler[];

//! handle an addressing layer packet regardless of the destination host and port.
extern volatile lnp_addressing_sniffer_handler_t lnp_addressing_sniffer_handler;


//! LNP host address
extern unsigned char lnp_hostaddr;

#ifdef CONF_RCX_PROTOCOL
//! packets from remote have no ports
extern volatile lnp_remote_handler_t lnp_remote_handler;
#endif

#if defined(CONF_RCX_MESSAGE) || defined(CONF_HOST)
//! handler for standard firmeware messages
extern volatile lnp_message_handler_t lnp_message_handler;
#endif

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

#ifndef CONF_HOST
extern
#endif
//! set the integrity layer packet handler
inline void lnp_integrity_set_handler(lnp_integrity_handler_t handler)
{
  lnp_integrity_handler = handler;
}

#ifndef CONF_HOST
extern
#endif
//! set an addressing layer packet sniffer handler
inline void lnp_addressing_sniffer_set_handler(lnp_addressing_sniffer_handler_t handler){
   lnp_addressing_sniffer_handler = handler;
}

#ifndef CONF_HOST
extern
#endif
//! set an addressing layer packet handler for a port.
inline void lnp_addressing_set_handler(unsigned char port, lnp_addressing_handler_t handler)
{
  if (!(port & CONF_LNP_HOSTMASK))  // sanity check.

    lnp_addressing_handler[port] = handler;
}

#ifndef CONF_HOST
extern
#endif
//! set new LNP host address
inline void lnp_set_hostaddr(unsigned char host)
{
    lnp_hostaddr = ((host << 4) & CONF_LNP_HOSTMASK);
}

#ifdef CONF_RCX_PROTOCOL
#ifndef CONF_HOST
extern
#endif
//! set the remote packet handler
inline void lnp_remote_set_handler(lnp_remote_handler_t handler)
{
  lnp_remote_handler = handler;
}
#endif


#if defined(CONF_RCX_MESSAGE) || defined(CONF_HOST)
#ifndef CONF_HOST
extern
#endif
//! set the standard firmware message handler
inline void lnp_message_set_handler(lnp_message_handler_t handler)
{
  lnp_message_handler = handler;
}

//! send a standard firmware message
extern int send_msg(unsigned char msg);

//! send a remote message code
extern int send_code(unsigned int code);
#endif

#ifdef CONF_RCX_MESSAGE
//! message variable
extern unsigned char lnp_rcx_message;

//! clear last message from standard firmware
extern inline void clear_msg(void)
{
   lnp_rcx_message = 0;
}

//! wait until receive a message
wakeup_t msg_received(wakeup_t m);

//! read received message from standard firmware
extern unsigned char get_msg(void);

#endif // CONF_RCX_MESSAGE

//! send a LNP integrity layer packet of given length
/*! \return 0 on success.
*/
extern int lnp_integrity_write(const unsigned char *data,unsigned char length);

//! send a LNP addressing layer packet of given length
/*! \return 0 on success.
*/
extern int lnp_addressing_write(const unsigned char *data,unsigned char length,
                         unsigned char dest,unsigned char srcport);

//! send a LNP integrity layer packet in "puts" fashion
/*! \return 0 on success.
*/
#define lnp_integrity_puts(msg)  lnp_integrity_write((unsigned char*)msg, strlen(msg))

//! send a LNP addressing layer packet in "puts" fashion
/*! \return 0 on success.
*/
#define lnp_addressing_puts(msg,dest,srcport)  lnp_addressing_write((unsigned char*)msg, strlen(msg), dest, srcport);

#if defined(CONF_PRINTF) || defined(CONF_HOST)
//! send a LNP integrity layer packet in "printf" fashion
/*! \return 0 on success.
*/
extern int lnp_integrity_printf(const char *fmt, ...);

//! send a LNP addressing layer packet in "printf" fashion
/*! \return 0 on success.
*/
extern int lnp_addressing_printf(unsigned char dest,
        unsigned char srcport, const char *fmt, ...);
#endif // CONF_PRINTF

#endif // CONF_LNP

#ifdef  __cplusplus
}
#endif

#endif // _lnp_h__
