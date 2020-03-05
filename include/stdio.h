/*! \file   include/stdio.h
    \brief  Interface: sprintf-related functions from stdio/stdarg
 */
    
/*
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
 *  The Original Code is from the Brown Computer Science Department,
 *  Weenix support code.
 *
 *  The Initial Developer of the Original Code is David Powell.
 *
 *  Contributor(s): Matthew Ahrens <mahrens@acm.org>,
 *                  Carl Troein <troein@users.sourceforge.net>
 */

#include <stdarg.h>

#ifndef __stdio_h__
#define __stdio_h__

#ifdef  __cplusplus
extern "C" {
#endif

/* This function takes a buffer, its length, a format specification,
 * and a va_list, formats the provided data according the the format
 * spec, and places the result in the buffer.  It returns the length of
 * the output.  The only format options supported by this function
 * are:
 *   %d - decimal unsigned int
 *   %i -    "        "     "
 *   %x - hexidecimal unsigned int (lowercase letters)
 *   %X -      "          "     "   (uppercase letters)
 *   %c - character
 *   %s - string
 *   %% - '%' character
 */
extern int vsnprintf(char *dst, int len, const char *fmt, va_list arg);

/* This function prints the specified format into the provided buffer
 * taking care not to access beyond the end of the string. 
 */
extern char *snprintf(char *dst, int len, const char *fmt, ...);

#ifdef  __cplusplus
}
#endif

#endif // __stdio_h__
