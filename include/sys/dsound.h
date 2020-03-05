/*! \file   include/sys/dsound.h
    \brief  Internal Interface: direct sound control
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
 *                  Froods <froods@alphalink.com.au>
 */

#ifndef __sys_dsound_h__
#define __sys_dsound_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include "../dsound.h"

#ifdef CONF_DSOUND

extern waitqueue_t *dsound_finished;

///////////////////////////////////////////////////////////////////////
//
// System functions
//
///////////////////////////////////////////////////////////////////////

//! initialize sound driver
extern void dsound_init();

//! shutdown sound driver
extern void dsound_shutdown();

#endif // CONF_DSOUND

#ifdef  __cplusplus
}
#endif

#endif	/* __sys_dsound_h__ */
