/*! \file   include/sys/dmotor.h
    \brief  Internal Interface: direct motor control
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

#ifndef	__sys_dmotor_h__
#define __sys_dmotor_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include "../dmotor.h"

#ifdef CONF_DMOTOR

///////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////

// the RCX-specific motor driver I/O address
extern unsigned char motor_controller;	//!< RCX Motor Controller port

// Raw motor register
#define MOTOR_CONTROLLER (*(unsigned char*)0xFF80)

// bits controlled by motor driver
extern unsigned char dm_mask;

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

//! initialize motors
//
void dm_init(void);

//! shutdown motors
//
void dm_shutdown(void);

#endif // CONF_DMOTOR

#ifdef  __cplusplus
}
#endif

#endif // __sys_dmotor_h__
