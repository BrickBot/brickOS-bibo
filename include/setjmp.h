/*! \file include/setjmp.h
    \brief  Non-local goto as specified in ANSI C
    \author Torsten Landschoff <tla@informatik.uni-kiel.de>
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

#ifndef __setjmp_h__
#define __setjmp_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_SETJMP

/*----[ Data type definitions ]-----------------------------------------*/

//! Information stored for non-local jumps
/*!
 *  Stores the machine status for non-local jumps. 
 *  The H8/300 CPU has the following state information:
 *  - 8 registers of 16 bit each (r0, r1, ..., r7)
 *  - the program counter
 *  - a condition code register of a single byte
 *  To allow execution of a longjmp we have to return to the state saved
 *  in a jmp_buf just like from a subroutine invocation. The machine 
 *  description of gcc for the H8 says that r0,r1,r2,r3 are destroyed
 *  by a subroutine call. Therefore we need to save the condition code, 
 *  the program counter of the caller (where we want to return), and the
 *  registers r4, r5, r6, r7. We are using 6 words for that and ignore 
 *  a byte at restore time.
 */

typedef int     jmp_buf[6];


/*----[ Function prototypes ]-------------------------------------------*/

//! Save execution context for non-local goto
/*!
 *  Records the current machine status for a non-local goto. The saved 
 *  status will be invalidated when returning from the stack frame 
 *  where it was set.
 *  @param env buffer for saved machine status
 *  @return 0 when returning directly (after saving the state), or 
 *  whatever was given to longjmp when returning via a non-local goto
 */
extern int  setjmp(jmp_buf env);

//! Non-local jump to saved machine context
/*!
 *  Restores a machine context as saved via setjmp.
 *  @param env the buffer of the saved machine status
 *  @param val the value to return from the setjmp function
 */
extern void longjmp(jmp_buf env, int val);

#endif // CONF_SETJMP

#ifdef  __cplusplus
}
#endif

#endif // __setjmp_h__
