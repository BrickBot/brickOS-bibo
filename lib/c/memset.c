/*! \file   memset.c
    \brief  memset function
    \author Markus L. Noga <markus@noga.de>
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
 *  The Original Code is legOS code, released October 17, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 */

typedef unsigned	size_t;			// data type for memory sizes


///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

//! fill memory block with a byte value.
/*! \param s start
    \param c byte fill value
    \param n number of bytes to fill
*/
void *memset(void* s,int c,size_t n) {
	void *res;
__asm__ __volatile__(
	"  add.w %1,%0\n"
	"  bra 1f\n"
	"0:mov.b %2l,@-%0\n"
	"1:cmp.w %1,%0\n"
	"  bne 0b\n"
	: "=&r" (res)				// output
	: "r" (s), "r" (c), "0" (n)		// input
	: "cc","memory"				// clobbered (final)
	);
	return res;					
}
