/*! \file   include/string.h
    \brief  Interface: string functions
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

#ifndef __string_h__
#define __string_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <mem.h>

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

//!  copy memory block from src to dest.
/*! \param dest destination
    \param src  source
    \param size number of bytes to copy

    \warning behaviour is undefined in case source and destination
    blocks overlap.
 */
extern void *memcpy(void *dest, const void *src, size_t size);

//! fill memory block with a byte value.
/*! \param s start
    \param c byte fill value
    \param n number of bytes to fill
 */
extern void *memset(void *s, int c, size_t n);

//! Copy null-terminated string from src to dest
/*! \param  src source
    \param  dest destination
    \return pointer to dest
 */
extern char *strcpy(char *dest, const char *src);

//! Determine string length
/*! \param  s string
    \return string length
 */
extern int strlen(const char *s);

//! Compare two strings
/*! \param  s1 first string
    \param  s2 second string
    \return <0: s1<s2, ==0: s1==s2, >0: s1>s2
 */
extern int strcmp(const char *s1, const char *s2);

#ifdef  __cplusplus
}
#endif

#endif // __string_h__
