/*! \file   include/persistent.h
    \brief  Interface: Definitions for persistent data
    \author Eddie C. Dost <ecd@skynet.be>
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

#ifndef __persistent_h_
#define __persistent_h_

#ifdef  __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

//! Define storage persistent across different runs of a program
/*! This macro is used to mark initialized data (doesn't apply to
    uninitialized data) as `persistent' data.
    This data will be saved across different runs of the program.

    Usage:
    You should insert __persistent between the variable name and equal
    sign followed by value, e.g:

    static int counter __persistent = 0;
    static char data[] __persistent = { 0x32, 0x36, ... };
*/
#define __persistent	__attribute__ ((__section__ (".persist")))

#ifdef  __cplusplus
}
#endif

#endif // __persistent_h__
