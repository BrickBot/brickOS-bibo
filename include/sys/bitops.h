/*! \file   include/sys/bitops.h
    \brief  Internal Interface: H8/300 bit operations
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

#ifndef __sys_bitops_h__
#define __sys_bitops_h__

#ifdef  __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

// g++ warns about __asm__ const. we define the problem away.
// the CXX symbol is predefined in the Makefile
//
#if defined(CXX) || __GNUC__ >= 3
# define ASMCONST
#else
# define ASMCONST const
#endif

//
// the H8/300 processor offers many useful  bit operations.
// sometimes, we have to help gcc to realize their full
// potential.
//
// the bit operations don't alter cc except for bld
//

//! set a single bit in memory
/*! *((char*)byte)|=(1<<bit)
*/
#define bit_set(byte,bit)	\
__asm__ ASMCONST ( "bset %0,@%1\n" : : "i" (bit),"r" (byte))

//! clear a single bit in memory
/*! ((char*)byte)&=~(1<<bit)
*/
#define bit_clear(byte,bit)	\
__asm__ ASMCONST ( "bclr %0,@%1\n" : : "i" (bit),"r" (byte))

//! load a single bit from a mask to the carry flag
/*! carry=mask & (1<<bit)
*/
#define bit_load(mask,bit)	\
__asm__ ASMCONST ( "bld %0,%1l\n" : : "i" (bit),"r" (mask):"cc")

//! load the inverse of a single bit from a mask to the carry flag
/*! carry=mask & inverse of (1<<bit)
*/
#define bit_iload(mask,bit)	\
__asm__ ASMCONST ( "bild %0,%1l\n" : : "i" (bit),"r" (mask):"cc")

//! store the carry flag to a single single bit in memory
/*! *((char*)byte)= ( (*((char*)byte)) & (~(1<<bit)) ) | (carry<<bit)
*/
#define bit_store(byte,bit)	\
__asm__ ASMCONST ( "bst %0,@%1\n" : : "i" (bit),"r" (byte))

#ifdef  __cplusplus
}
#endif

#endif // __sys_bitops_h__
