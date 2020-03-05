/*! \file   include/sys/mm.h
    \brief  Internal Interface: memory management
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

#ifndef __sys_mm_h__
#define __sys_mm_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_MM

#include <mem.h>
#include <stdlib.h>


///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

typedef struct _meminfo_t {
    struct _meminfo_t *prev; //!< pointer to previous block, 
                             //   only valid if previous block is free
    struct _meminfo_t *next; //!< pointer to next block, 
                             //   if low bit set, previous block is free
    unsigned short    flags; //!< flags of memory block.
    struct _meminfo_t *nextfree;
                             //!< pointer to next free block, 
                             //   only valid if block is free
} meminfo_t;
      
#define MM_FREE		0		//!< block free
#define MM_RESERVED	1		//!< block reserved
#define MM_USER		2		//!< block allocated by user
#define MM_KERNEL	3		//!< block allocated by kernel
#define MM_PREVFREE	4		//!< flag set if previous block free

// as data generally nees to be word aligned, 1 unit ^= 2 bytes
//

#define MM_HEADER_SIZE	4			//!< 2 words header: pid, next
//! split off if there is space for another block
#define MM_SPLIT_THRESH	(MM_HEADER_SIZE+sizeof(meminfo_t))

extern size_t mm_start;				//!< end of kernel code + data

extern meminfo_t* mm_first_free;		//!< ptr to first free block.

// Macros for mm_init()
// Always alternate FREE and RESERVED.
//



///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

//! initialize memory management
extern void mm_init();

//! free all blocks allocated by the current process
extern void mm_reaper(void);

//! how many bytes of memory are free?
extern int mm_free_mem(void);

#endif	// CONF_MM

#ifdef  __cplusplus
}
#endif

#endif  // __sys_mm_h__
