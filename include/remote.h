/*! \file   include/remote.h
    \brief  Interface: LEGO Infrared Remote Control 
    \author Ross Crawford <rcrawford@csi.com>
 */
 
/*
 * Copyright (c) 2001 Ross Crawford
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
 */

#ifndef _REMOTE_H
#define _REMOTE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_LR_HANDLER


//! timeout value in mSec
#define LR_TIMEOUT 100

//! The message #1 key
#define LRKEY_M1 0x1
//! The message #2 key
#define LRKEY_M2 0x2
//! The message #3 key
#define LRKEY_M3 0x4
//! Motor-A forward (up)
#define LRKEY_A1 0x8
//! Motor-B forward (up)
#define LRKEY_B1 0x10
//! Motor-C forward (up)
#define LRKEY_C1 0x20
//! Motor-A reverse (down)
#define LRKEY_A2 0x40
//! Motor-B reverse (down)
#define LRKEY_B2 0x80
//! Motor-C reverse (down)
#define LRKEY_C2 0x100
//! Run Program 1
#define LRKEY_P1 0x200
//! Run Program 2
#define LRKEY_P2 0x400
//! Run Program 3
#define LRKEY_P3 0x800
//! Run Program 4
#define LRKEY_P4 0x1000
//! Run Program 5
#define LRKEY_P5 0x2000
//! Stop key
#define LRKEY_STOP 0x4000
//! Beep Key
#define LRKEY_BEEP 0x8000

//! enumerate our event types
enum _evt {
  LREVT_KEYON = 1,	//!< a key on the remote was pressed
  LREVT_KEYOFF = 2	//!< a key on the remote was released
} EventType; 	//!< the LEGO IR Remote event types

//! the remote key handler type
/*! \param event_type was key pressed or released?
 *  \param key the key, on the remote, that was pressed/released
 *  \return 0 if not processed, otherwise non-zero.
 */
typedef int (*lr_handler_t) (unsigned int, unsigned int);

//! remote handler
extern lr_handler_t lr_handler;

//! start the LEGO IR Remote subsystem
void lr_startup(void);
//! initialize the LEGO IR Remote subsystem
void lr_init(void);
//! stop the LEGO IR Remote subsystem
void lr_shutdown(void);

//! set a new handler for LEGO IR Remote messages
/*! \param handler pointer to a function which is to be the 
 *   processing function for received messages
 *  \return Nothing
 */
extern inline void lr_set_handler(lr_handler_t handler) {
  lr_handler = handler;
}

//! dummy remote event handler
#define LR_DUMMY_HANDLER ((lr_handler_t)0)

#endif	// CONF_TM

#ifdef  __cplusplus
}
#endif

#endif
