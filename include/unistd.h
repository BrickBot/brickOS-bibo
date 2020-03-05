/*! \file   include/unistd.h
    \brief  Interface: reduced UNIX standard library
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

#ifndef __unistd_h__
#define __unistd_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <tm.h>
#include <time.h>

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

extern void debug_printf(const char* format, ...);

#ifdef CONF_TM
/*!  start task executing (with passed paramters)
 *    called from user code
 *  \param code_start the entry-point of the new task
 *  \param argc the count of arguments passed (0 if none)
 *  \param argv an array of pointers each pointing to an argument (NULL if none)
 *  \param priority the priority at which to run this task
 *  \param stack_size the amount of memory in bytes to allocate to this task for its call stack
 *  \return -1 if failed to start, else tid (task-id)
 */
extern inline tid_t execi(int (*code_start) (int, void **), int argc, void **argv,
			  priority_t priority, size_t stack_size)
{
    return kexeci(code_start, argc, argv, priority, stack_size, T_USER);
}

/*! signal shutdown for a task
 * \param tid TaskId of task to be notified
 */
extern void shutdown_task(tid_t tid);

/*! signal shutdown for many tasks
 * \param flags T_USER for user tasks, T_USER|T_KERNEL for kernel tasks
 * \todo research {flags}, then fix this documentation
 */
extern void shutdown_tasks(tflags_t flags);

/*! kill specified (tid) task
 * \param tid TaskId of task to be killed
 * \todo  FIXME: this belongs in a different header
 */
extern void kill(tid_t tid);

/*! kill all tasks with priority less than or equal equal to p, excluding self.
 * \param flags T_USER for user tasks, T_USER|T_KERNEL for kernel tasks
 *  \sideeffect All tasks meeting this criteria are killed
 */
extern void killall(tflags_t flags);

/*!  exit task, returning code
 * \param code the exit code to return to the caller
 */
extern void exit(int code) __attribute__((noreturn));

//
//!  current task yields the rest of timeslice
//
extern void yield(void);

/*! suspend task until wakeup function returns non-null
 *  \param wakeup the function to be called when woken up
 *  \param data the wakeup_t structure to be passed to the called function
 *  \return wakeup() return value
 *  \note wakeup function is called in task scheduler context
 */
extern wakeup_t wait_event(wakeup_t(*wakeup) (wakeup_t), wakeup_t data);


//! delay execution allowing other tasks to run
/*! \param sec sleep duration in seconds
 *  \return number of seconds left if interrupted, else 0
 *  \bug interruptions not implemented
 */
extern unsigned int sleep(unsigned int sec);

/*! delay execution allowing other tasks to run
 *  \param msec sleep duration in milliSeconds
 *  \return number of milliSeconds left if interrupted, else 0
 *  \bug interruptions not implemented
 */
extern unsigned int msleep(unsigned int msec);

#else
extern inline wakeup_t wait_event(wakeup_t(*wakeup) (wakeup_t), wakeup_t data)
{
  wakeup_t res;

  while (!(res = wakeup(data)))
	/* wait */;
  return res;
}

// Replacement for sleep/msleep if no TM 
#define	sleep(s)	delay(1000*(s))
#define msleep(s)	delay(s)
#endif

#ifdef  __cplusplus
}
#endif

#endif // __unistd_h__
