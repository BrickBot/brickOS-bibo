/*! \file   include/sys/tm.h
    \brief  Internal Interface: task management
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

#ifndef __sys_tm_h__
#define __sys_tm_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_TM

#include "../tm.h"
#include "sys/handlers.h"

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////


#define SP_RETURN_OFFSET	2	//!< return address offset on stack in words.

#define IDLE_STACK_SIZE		128	//!< should suffice for IRQ service

//
//! task states
/*! \todo the following comments on the defined may cause problems in
 * when used in macros/expressions, etc.  FixEm?
*/

#define T_DEAD      0                           //!< dead and gone, stack freed
#define T_ZOMBIE    1                           //!< terminated, cleanup pending
#define T_WAITING   2                           //!< waiting for an event
#define T_RUNNING   3                           //!< running or wants to run



#ifndef DOXYGEN_SHOULD_SKIP_INTERNALS


//! task data type
/*! a shorthand
 */
typedef struct _tdata_t tdata_t;

#endif // DOXYGEN_SHOULD_SKIP_INTERNALS 

///////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////

extern tid_t ctid;			//!< ptr to current process data
extern tdata_t td_idle;			//!< ptr to idle task

extern volatile unsigned int nb_tasks;		//!< number of tasks

  // nb_system_tasks is maintained  in execi and kill/killall
  // T_KERNEL tasks are counted as they are started
extern volatile unsigned int nb_system_tasks;


/**
 * The kernel lock.  
 * The high bit of the kernel_lock is cleared if handlers
 * need to be called when kernel lock is released. 
 * The other 7 bits build a lock counter.
 *
 * Thus handlers need to be called as if kernel_lock == 0.
 */
extern volatile char kernel_lock;

///////////////////////////////////////////////////////////////////////
//
// Internal Functions
//
///////////////////////////////////////////////////////////////////////

/**
 * run handlers and determine next thread to run.
 * @return 0 if only idle thread is running, otherwise the running
 *           thread with highest priority.
 */
extern tid_t run_handlers(void);

//! init task management
/* called in single tasking mode before task setup.
*/
extern void tm_init(void);


//! Idle process
/*! called by idle thread to initiate task-management.
*/
extern void tm_idle(void);

//! the task switcher
/*! the task switcher saves active context and switches to the given
    thread.
*/
extern void tm_switcher(tid_t thread);

//! the idle task
/*! infinite sleep instruction to conserve power
*/
extern int tm_idle_task(int,char**);

/*! Switch to handler thread.
*/
extern void tm_switch_handlers(void);



/**
 * Memory barrier to prevent gcc from doing too many optimizations.
 */
#define MEM_BARRIER __asm__ ("":::"memory")


/**
 * Grab kernel lock.  Prevents the code to be preempted, except
 * by interrupts.
 */
/* There is no need to use any atomic operations: The code can only be
 * preempted if lock counter is zero. In that case the counter will be
 * restored to zero before returning.
 */
#define grab_kernel_lock() kernel_lock++; MEM_BARRIER

/**
 * Release the kernel lock.
 */
extern inline void release_kernel_lock() {
    /* Order is very important.  GCC doesn't optimize
     * it away since kernel_lock is volatile.
     */
    MEM_BARRIER;
    if (--kernel_lock == 0) {
	/* We just released the kernel lock and need to call handlers.
	 */
	tm_switch_handlers();
    }
}

#endif	// CONF_TM

#ifdef  __cplusplus
}
#endif

#endif	// __sys_tm_h__

