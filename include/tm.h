/*! \file include/tm.h
    \brief  Interface: task management
    \author Markus L. Noga <markus@noga.de>

    Defines types and flags used in task management.
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
 *                  Lou Sortman <lou (at) sunsite (dot) unc (dot) edu>
 */

#ifndef __tm_h__
#define __tm_h__

#ifdef  __cplusplus
extern "C" {
#endif // __cplusplus

#include <config.h>
#include <mem.h>

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

//
//! task flags
//

#define T_STATE         7
#define T_USER    	0                           //!< user task
#define T_KERNEL  	(1 << 5)                    //!< kernel task
#define T_IDLE    	(1 << 6)                    //!< idle task
#define T_SHUTDOWN	(1 << 7)                    //!< shutdown requested
#define DEFAULT_STACK_SIZE  512                 //!< that's enough.

typedef unsigned char tflags_t;                 //!< task flags type
typedef unsigned char priority_t;               //!< task priority type

/** task data structure
 * @internal
 */
struct _tdata_t {
    size_t *sp_save;               //!< saved stack pointer (must be first)
    
    tflags_t tflags;               //!< task state and flags
    priority_t priority;           //!< numeric priority level
    
    struct _tdata_t *next;         //!< next task in queue
    struct _tdata_t *prev;         //!< previous task in queue
};

//! The lowest possible task priority
#define PRIO_LOWEST        1
//! The priority of most tasks
#define PRIO_NORMAL       10
//! The highest possible task priority
#define PRIO_HIGHEST      20

typedef unsigned long wakeup_t;                 //!< wakeup data area type


//! task id type
/*! In effect, the kernel simply typecasts *tdata_t to tid_t.
 */
typedef struct _tdata_t *tid_t;

#if defined(CONF_TM)
//! test to see if task has been asked to shutdown
/*! Check task shutdown flag.  If set, the task should shutdown
 *  as soon as possible.  If clear, continue running.
 */
#define shutdown_requested() ((char)((ctid->tflags & T_SHUTDOWN) != 0))
extern tid_t ctid;
#else // CONF_TM
#define shutdown_requested() ((char)0)
#endif // CONF_TM

/*!  start task executing (with passed paramters)
 *    called from user code
 *  \param code_start the entry-point of the new task
 *  \param argc the count of arguments passed (0 if none)
 *  \param argv an array of pointers each pointing to an argument (NULL if none)
 *  \param priority the priority at which to run this task
 *  \param stack_size the amount of memory in bytes to allocate to this task for its call stack
 *  \param tflags flags: T_KERNEL for kernel threads, T_USER for user threads.
 *  \return -1 if failed to start, else tid (task-id)
 */
extern tid_t kexeci(int (*code_start) (int, void **), int argc, void **argv,
		    priority_t priority, size_t stack_size, int tflags);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif
