/*! \file   mm.c
    \brief  Implementation: dynamic memory management
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

#include <sys/mm.h>

#ifdef CONF_MM

#include <stdlib.h>
#include <sys/tm.h>
#include <sys/handlers.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CONF_TM
//! dummy current process ID
/*! we need a non-null, non-0xffff current pid even if there is no
    task management.
*/
#define ctid (tid_t) 0x0001;
#endif

meminfo_t *mm_first_free;        //!< first free block

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

//
// memory block structure:
// 0 1       : pointer to previous block, if previous block is free.
// 2 3       : pointer to next data block.
//             low bit set if previous block free.
// 4 5       : flags (0=empty)
// 6 7       : pointer to next free block, if current block is free.
// 6 ... n+2 : data
//


//! memory from addr on can be allocated
/*! Macro for mm_init().
    Always alternate MM_BLOCK_FREE and MM_BLOCK_RESERVED.
*/
#define MM_BLOCK_FREE(addr) 					\
	next=(meminfo_t*)(addr-2);				\
        current->next = (meminfo_t*) ((size_t) next | pfree);	\
        lastfree->nextfree = next;				\
        next->flags = MM_FREE;                                  \
        pfree = free;						\
        free = 1;						\
	lastfree=next;                                          \
	current=next;


//! memory from addr on is reserved
/*! Macro for mm_init().
    Always alternate MM_BLOCK_FREE and MM_BLOCK_RESERVED.
*/
#define MM_BLOCK_RESERVED(addr) 				\
	next=(meminfo_t*)(addr - 6);				\
        current->next = (meminfo_t*) ((size_t) next | pfree);	\
	next->prev = current;					\
	next->flags = MM_RESERVED;				\
        pfree = free;						\
        free = 0;						\
	current=next

//! initialize memory management
/*!
*/
__TEXT_INIT__ void mm_init() {
    meminfo_t *current,*next,*lastfree;
    size_t pfree, free;
    
    current = (meminfo_t*) ((size_t)&mm_start - 2);
    mm_first_free = current;
    current->flags = MM_FREE;
    lastfree = current;
    pfree = 0; free = 1;

    // memory layout
    //
    MM_BLOCK_RESERVED(0xe000);      // init memory (freed later)
    MM_BLOCK_RESERVED(0xef30);      // lcddata
    MM_BLOCK_FREE    (0xef50);      // ram2
    MM_BLOCK_RESERVED(0xf000);      // high memory (motor) area
    MM_BLOCK_FREE    (0xfdc0);      // ram4 (fast ram, is there better use?)
    MM_BLOCK_RESERVED(0xff00);      // stack, onchip
    current->next = (meminfo_t*) (0 | pfree);
    lastfree->nextfree = (meminfo_t*) 0;
}


//! allocate a block of memory
/*! \param size requested block size
    \return 0 on error, else pointer to block.
*/
void *malloc(size_t size) {
    void *mem_block = NULL;
    meminfo_t *ptr;
    meminfo_t **pptr;
    
    size = (size+1) & ~1;       // only multiples of 2
    if (size < 4)
	size = 4;
    
    grab_kernel_lock();
    pptr = &mm_first_free;
    
    while((ptr = *pptr)) {
        int blocksize = ((void*)ptr->next - (void*)ptr);
        /* big enough? */
        if (blocksize >= size + MM_HEADER_SIZE) {
            /* set flags */
            ptr->flags = (ctid->tflags & T_KERNEL) ? MM_KERNEL : MM_USER;

            /* split this block? */
            if(blocksize >= size + MM_SPLIT_THRESH) {
                meminfo_t *next = (meminfo_t*) ((size_t)ptr + (size + 4));
                next->flags = MM_FREE;
                next->next = ptr->next;
                ptr->next->prev = next;
                next->nextfree = ptr->nextfree;
                ptr->next = next;
                *pptr = next;
            } else {
                ptr->next->next = (void*) ((size_t) ptr->next->next & ~1);
                *pptr = ptr->nextfree;
            }
            mem_block = (void*) &(ptr->nextfree);
            break; 
        }
        pptr = &ptr->nextfree;
    }
    release_kernel_lock();
    return mem_block;
}

//! free a previously allocated block of memory.
/*! \param the_ptr pointer to block
*/
void free(void *the_ptr) {
    meminfo_t *ptr = (meminfo_t*) ((size_t)the_ptr - 6);
    if (the_ptr==NULL)
	return;

    grab_kernel_lock();
    if (((size_t)ptr->next & 1)) {
	/* merge with previous free block */
	meminfo_t *next = (meminfo_t*) ((size_t) ptr->next & ~1);
	if (next->flags == MM_FREE) {
	    /* merge with next free block and dequeue that */
	    ptr->prev->next = next->next;
	    next->next->prev = ptr->prev;
	    ptr->prev->nextfree = next->nextfree;
	} else {
	    /* tell next block that previous block is now free */
	    next->next = (void*) ((size_t) next->next | 1);
	    ptr->prev->next = next;
	    next->prev = ptr->prev;
	}
    } else {
	/* enqueue block to free list */
	ptr->flags = MM_FREE;
	meminfo_t **prev = &mm_first_free;
	while (*prev && *prev < ptr)
	    prev = &((*prev)->nextfree);
	if (ptr->next->flags == MM_FREE) {
	    /* merge with next free block */
	    ptr->nextfree = ptr->next->nextfree;
	    ptr->next = ptr->next->next;
	} else {
	    /* New free block */
	    ptr->next->next = (void*) ((size_t) ptr->next->next | 1);
	    ptr->nextfree = *prev;
	}
	ptr->next->prev = ptr;
	*prev = ptr;
    }
    release_kernel_lock();
}

//! free all blocks allocated by the user processes.
/*! called by program_stop().
*/
void mm_reaper(void) {
    meminfo_t *ptr, *next;
    
    ptr = (meminfo_t*) (((size_t)&mm_start) - 2);
    while (ptr) {
	next = (meminfo_t*) ((size_t)ptr->next & ~1);
	if (ptr->flags == MM_USER)
	    free(&ptr->nextfree);
	ptr = next;
    }
}

//! return the number of bytes of unallocated memory
int mm_free_mem(void) {
    int free = 0;
    meminfo_t *ptr;
    
    grab_kernel_lock();

    // Iterate through the free list
    for (ptr = mm_first_free; ptr; ptr = ptr->nextfree)
	free += (size_t)ptr->next - (size_t)ptr - 4;
    
    release_kernel_lock();
    return free;
}

#endif
