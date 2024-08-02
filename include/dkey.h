/*! \file   include/dkey.h
    \brief  Interface: debounced key driver
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

#ifndef __dkey_h__
#define __dkey_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>
#ifdef CONF_DKEY

#include <tm.h>

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

#define KEY_ONOFF 0x01		//!< the on/off key is pressed
#define KEY_RUN   0x02		//!< the run key is pressed
#define KEY_VIEW  0x04		//!< the view key is pressed
#define KEY_PRGM  0x08		//!< the program key is pressed

#define KEY_ANY   0x0f		//!< any of the keys
#define KEY_NONE  0x00      //!< none of the keys

#define KEYS_SYSTEM (KEY_ONOFF | KEY_RUN)

#define KEY_RELEASED 0
#define KEY_PRESSED  1

///////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////

//! the currently active keys
/*! this is a bitmask - multi-key combinations are admissible.
 */
extern volatile unsigned char dkey_multi;

//! the current key
/*! this is not a bitmask - at most one key at a time.
 */
extern volatile unsigned char dkey;

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////


//! wait until any of the given keys is pressed or released.
//
extern char dkey_wait(unsigned char pressed, unsigned char mask);

//! check if the given key is pressed.
#define dkey_pressed(key)  (dkey_multi & (key))
//! check if the given key is released.
#define dkey_released(key)  (!(dkey_multi & (key)))

//! wait for keypress and return key code.
/*! key combinations not admissible.
 */
extern unsigned char getchar();

#endif // CONF_DKEY

#ifdef  __cplusplus
}
#endif

#endif // __dkey_h__
