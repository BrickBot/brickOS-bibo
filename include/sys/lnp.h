/*! \file   sys/lnp.h
    \brief  Internal LNP Interface: link networking protocol
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

#ifndef __sys_lnp_h__
#define __sys_lnp_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include "../lnp.h"

#ifdef CONF_LNP

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

#define LNP_HOSTADDR  CONF_LNP_HOSTADDR   //!< the LNP host address (config.h)
#define LNP_HOSTMASK  CONF_LNP_HOSTMASK   //!< the LNP host mask (config.h)

//! LNP port mask is derived from host mask
#define LNP_PORTMASK  (0x00ff & ~CONF_LNP_HOSTMASK)

#if defined(CONF_RCX_PROTOCOL) || defined(CONF_RCX_MESSAGE)
//! length of header from remote/rcx, -1 because first byte is used to id sequence
#define LNP_RCX_HEADER_LENGTH (3-1)

//! length of remote button op, -3 because first 3 bytes is used to id sequence
#define LNP_RCX_REMOTE_OP_LENGTH (5-3)

//! length of rcx message op, -3 because first 3 bytes is used to id sequence
#define LNP_RCX_MSG_OP_LENGTH (5-3)
#endif

//! states for the integrity layer state machine
typedef enum {
  LNPwaitHeader,
  LNPwaitLength,
  LNPwaitData,
  LNPwaitCRC,

  //! states when waiting for rcx protocol message
  LNPwaitLH1,  // note that first byte is consumed by LNPwaitHeader
  LNPwaitLH2,  // inverted header
  LNPwaitLO1,  // actually, RCX opcode dispatch
  LNPwaitLO2,  // remote opcode inverted
  LNPwaitLData,  // data fields
  LNPwaitLDataI, // data fields
  LNPwaitLChksum, // chksum 
  LNPwaitLChksumI, // inverted chksum
} lnp_integrity_state_t;


///////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////

//! the integrity layer state
extern lnp_integrity_state_t lnp_integrity_state;


///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

//! the LNP `copy and compute checksum' function.
extern unsigned char lnp_checksum_copy( unsigned char *dest,
          const unsigned char *data,
          unsigned length );

//! receive a byte from the physical layer, decoding integrity layer
//! packets.
/*! \param  b the byte to receive
*/
extern void lnp_integrity_byte(unsigned char b);

//! reset the integrity layer on error or timeout.
extern void lnp_integrity_reset(void);

//! return whether a packet is currently being received
/*! \return 1 if yes, else zero
*/
extern inline int lnp_integrity_active(void) {
  return lnp_integrity_state!=LNPwaitHeader;
}

//! Initialise protocol handlers
/*! Adressing port 0 is reserved for the program handler.
 *  DLL transfers programs to this port.
*/
extern void lnp_init(void);

//! Shut down protocol handlers
extern void lnp_shutdown();
#endif  // CONF_LNP

#ifdef  __cplusplus
}
#endif

#endif  // __sys_lnp_h__

