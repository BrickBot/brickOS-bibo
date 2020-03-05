/*! \file   memcpy.c
    \brief  memcpy function
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

//!  copy memory block from src to dest.
/*! \param dest destination
    \param src  source
    \param size number of bytes to copy

    \warning behaviour is undefined in case source and destination blocks
             overlap.
*/
void memcpy(void* dest,void* src,size_t size) {
    char *end=((char*)src)+size;
    int dummy;
__asm__ __volatile__(
#ifdef CONF_ROM_MEMCPY
        "\n\
; memcpy == [r1,r1+r2) -> [r0,r0+r2)\n\
; rom == [r0,r1) -> [r2,r2+r1-r0)\n\
        mov.w r0,r3\n\
        mov.w r1,r0\n\
        add.w r2,r1\n\
        mov.w r3,r2\n\
        jmp @rom_memcpy\n\
	"
#else
	"\n\
           bra 1f\n\
         0:mov.b @%1+,%0l\n\
           mov.b %0l,@%3\n\
           adds #1,%3\n\
         1:cmp %1,%2\n\
           bne 0b\n"
#endif
	: "=&r" (dummy) 			// output
	: "r" (src), "r" (end), "r" (dest)      // input
	: "cc","memory"	                        // clobbered
	);
}
