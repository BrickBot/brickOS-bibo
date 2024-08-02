/*! \file   include/sys/dkey.h
    \brief  Internal Interface: direct sensor access
    \author Jochen Hoenicke
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
 */

#ifndef	__sys_dkey_h__
#define __sys_dkey_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_DKEY

#include <sys/waitqueue.h>

///////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////

extern waitqueue_t *dkey_system_waitqueue;
extern waitqueue_t *dkey_program_waitqueue;
extern waitqueue_t *dkey_user_waitqueue;

#endif	// CONF_DKEY

#ifdef  __cplusplus
}
#endif

#endif	// __sys_dkey_h__

