/*! \file   include/lnp/lnp-logical.h
    \brief  LNP Interface: link networking protocol logical layer
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

#ifndef __lnp_logical_h__
#define __lnp_logical_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_LNP

#ifdef CONF_HOST
#include <stddef.h>
#include "../host/rcx_comm.h"
#else
#include <mem.h>
#include <sys/h8.h>
#endif

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

#ifndef CONF_HOST

//! Set the IR transmitter range
/*! Configure the INFRARED transmitter power
 *  \param far:  0: sets short range, 1: sets long range
 *  \return Nothing
 *
 *  NOTE1: this setting remains in effect until changed or 
 *  the RCX power is turned off.
 *
 *  NOTE2: toggles port 4 bit 0
 *  \todo determine what clears this and then correct NOTE1
*/
extern inline void lnp_logical_range(int far) {
  if(far)
    *((char*)&PORT4) &=~1;
  else
    *((char*)&PORT4) |=1;
}

//! Test the IR transmitter range setting
/*! Determine if the INFRARED transmitter power is set to long range
 *  \return T/F where TRUE means transmitter is set to long range
*/
extern inline int lnp_logical_range_is_far(void) {
  return !(*((char*)&PORT4)&1);
}

//! disable IR carrier frequency.
extern inline void carrier_shutdown(void);

//! enable IR carrier frequency.
extern void carrier_init(void);

#endif  // ! CONF_HOST

//! Write buffer to IR port
/*! Write {len} chars (starting at {buf}) to IR port.  
 * \param buf pointer to array of chars to be written
 * \param len number of chars in array (to be written)
 * \return 0 if OK, else collision
 *
 *  NOTE: doesn't return until all are written
 *  (blocking write)
*/
extern int lnp_logical_write(const void *buf, size_t len);

#ifdef CONF_HOST
//! write to IR port, blocking; specific to the "host" implementation.
/*! \param file descriptor to write to
    \param tty type
    \param buf data to transmit
    \param len number of bytes to transmit
    \return 0 on success, else collision
*/
extern int lnp_logical_write_host(tty_t *tty, const void* buf, size_t len);

#endif  // CONF_HOST

#endif  // CONF_LNP

#ifdef  __cplusplus
}
#endif

#endif  // __lnp_logical_h__

