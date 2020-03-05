/*! \file   include/stdlib.h
    \brief  Interface: reduced standard C library

	This file describes the public programming interface for 
	memory management services and random number services
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

#ifndef __stdlib_h__
#define __stdlib_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <mem.h>

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

//
// Standard memory management.
// See the Linux man pages for details.
//

//! allocate and return pointer to initialized memory
/*! calloc() allocates memory for an array of {nmemb} elements of {size} bytes
 *  each and returns a pointer to the allocated memory.  The memory is filled
 *  with zero values.
 *  \param nmemb the number of members to allocate
 *  \param size the size (in bytes) of each member to be allocated
 *  \return a pointer to the allocated memory (or NULL if failed)
 *
 *  NOTE: content of the returned memory is initialized by this routine
 *  \bug multiplication overflow (elements * size) is not detected
 *
 */
extern void *calloc(size_t nmemb, size_t size);

//! allocate and return pointer to uninitialized memory
/*! malloc()  allocates  {size}  bytes of memory and returns a pointer to it.
 *  \param size the number of bytes of memory to be allocated
 *  \return a pointer to the allocated memory (or NULL if failed)
 *
 *  NOTE: content of the returned memory is not initialized by this routine
 */
extern void *malloc(size_t size);

//! return the allocated memory to memory management.
/*! free() frees the memory space pointed to by {ptr}, which must  have  been
 *  returned by a previous call to malloc(), or calloc().  Other-
 *  wise, or  if  free(ptr)  has  already  been  called  before,  undefined
 *  behaviour occurs.  If ptr is NULL, no operation is performed.
 *  \param ptr a pointer to previously allocated memory
 *  \return Nothing
 */
extern void free(void *ptr);

//! generate a random number
/*! The random() function returns successive pseudo-random numbers 
 *  \return a random number in the range from 0 to RAND_MAX
 */
extern long int random(void);
//! seed the random number generator
/*! The srandom() function sets its argument as the seed for a new sequence
 *  of pseudo-random integers to be returned by random().  These  sequences
 *  are  repeatable  by  calling srandom() with the same seed value.  If no
 *  seed value is provided, the random() function is  automatically  seeded
 *  with a value of 1.
 *  \param seed 
 *  \return Nothing
 */
extern void srandom(unsigned int seed);

#ifdef  __cplusplus
}
#endif

#endif // __stdlib_h__
