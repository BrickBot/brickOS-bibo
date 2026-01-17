/*! \file tm.c
    \brief  Implementation: Task management
    \author Jochen Hoenicke <hoenicke@users.sourceforge.net>
    
    The implementation of the multitasking scheduler as
    well as library functions relating to task management.
*/

//#define DEBUG(x...) debug_printf(x)    
#define DEBUG(x...) 

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
 *  Contributor(s): Jochen Hoenicke <hoenicke@users.sourceforge.net>
 *                  Markus L. Noga <markus@noga.de>
 *                  Ben Laurie <ben@algroup.co.uk>
 *                  Lou Sortman <lou (at) sunsite (dot) unc (dot) edu>
 */

#include <sys/tm.h>
#include <sys/handlers.h>
#include <sys/waitqueue.h>

#ifdef CONF_TM

#include <sys/mm.h>
#include <sys/time.h>
#include <sys/irq.h>
#include <sys/bitops.h>
#include <sys/handlers.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef CONF_VIS
# include <sys/lcd.h>
# include <conio.h>
# include <sys/battery.h>
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////////////


/*! The task decriptor of the idle task.  
    Also used as starting point for the running tasks.
    The idle task is also the task that invokes handlers.
 */
tdata_t td_idle;            //!< idle and handler task.  
tdata_t waiters;            //!< waiter task queue
tdata_t *ctid;              //!< ptr to current task data

volatile unsigned int nb_tasks;                 //!< number of tasks
volatile unsigned int nb_system_tasks;          //!< number of system (kernel) tasks

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////


//! the task switcher
/*! the task switcher saves active context and switches to the given
    thread.
*/
void tm_switcher(tid_t thread);
#ifndef DOXYGEN_SHOULD_SKIP_THIS
__asm__("\n\
.text\n\
.align 1\n\
.globl _tm_switcher\n\
_tm_switcher:\n\
      ; r0 .. r3 saved by caller\n\
\n\
      push r4\n\
      push r5\n\
      push r6\n\
\n\
      mov.w @_ctid, r1\n\
      mov.w r7,@r1                            ; save old sp\n\
      mov.w r0,@_ctid                         ; set new ctid\n\
      mov.w @r0,r7                            ; set new sp\n\
\n\
      pop r6\n\
      pop r5\n\
      pop r4\n\
\n\
      ; r0..r3 will be restored by caller\n\
\n\
      rts                                     ; return to new task\n\
");
#endif  // DOXYGEN_SHOULD_SKIP_THIS

//! initialize task management
/*! initialize tasking variables and start the system tasks
!*! (called in single tasking mode before task setup.)
*/
__TEXT_INIT__ void tm_init(void) {
    // only idle tasks right now.
    //
    nb_tasks = 1;
    nb_system_tasks = 1;
    
    // the idle task is an institution
    //
    td_idle.tflags = T_RUNNING | T_KERNEL | T_IDLE;
    td_idle.priority = 0;
    td_idle.next = &td_idle;
    td_idle.prev = &td_idle;

    waiters.next = &waiters;
    waiters.prev = &waiters;
    ctid = &td_idle;
}


//! idle and call handlers appropriately
/*! May only be called from idle task with kernel lock set and interrupts off
*/
void tm_idle(void) {
    if (kernel_lock & 0x80) {
	__asm__("andc #0x7f, ccr"); /* enable IRQ*/
	__asm__("sleep");
	__asm__("orc #0x80, ccr"); /* disable IRQ*/
    }

    for (;;) {
	if (handler_queue) {
	    handler_t *handler = handler_queue;
	    if (handler_tail == &handler->next)
		handler_tail = &handler_queue;
	    handler_queue = handler->next;
	    __asm__("andc #0x7f, ccr"); /* enable IRQ*/
	    
	    handler->next = 0;
	    handler->code();
	    __asm__("orc #0x80, ccr"); /* disable IRQ*/
	} else if (&td_idle != td_idle.next) {
	    kernel_lock = 0x80;
	    tm_switcher(td_idle.next);
	    kernel_lock = 1;
	} else {
	    kernel_lock = 0x81;
	    return;
	}
    }
}

void tm_switch_handlers(void) {
    __asm__("orc #0x80, ccr"); /* disable IRQ*/
    tm_switcher(&td_idle);
    __asm__("andc #0x7f, ccr"); /* enable IRQ*/
}

/**
 * Enqueue a process on the run queue
 */
static void make_running(tid_t process) {
    tid_t td;

    if ((process->tflags & T_STATE) != T_WAITING)
	return;

    /* dequeue from waiters queue */
    process->next->prev = process->prev;
    process->prev->next = process->next;

    process->tflags ^= (T_WAITING ^ T_RUNNING);
    td = td_idle.next; 
    if (td->priority > process->priority) {
	do {
	    td = td->next;
	} while (td->priority > process->priority);
    } else {
	/* The new process takes priority.  Reschedule asap */
	kernel_lock &= ~0x80;
    }
    process->next = td;
    process->prev = td->prev;
    td->prev->next = process;
    td->prev = process;
}


//! schedule execution of a new task
/*! \param code_start start address of code to execute
    \param argc first argument passed, normally number of strings in argv
    \param argv second argument passed, normally pointer to argument pointers.
    \param priority new task's priority
    \param stack_size stack size for new task
    \return -1: fail, else tid.
    
    will return to caller in any case.
*/
tid_t kexeci(int (*code_start)(int, void**),int argc, void **argv,
	     priority_t priority, size_t stack_size, int tflags) {
    tdata_t *td;
    size_t * sp;

    // get memory
    //
    // task & stack area belong to parent task
    // they aren't freed by mm_reaper()
  
    td =malloc(sizeof(tdata_t) + stack_size);
    if (td == NULL)
    {
	free(td);
	return (tid_t)-1;
    }
    
    td->priority = priority;
    td->tflags = tflags;
    if ((tflags & T_KERNEL))
	nb_system_tasks++;
    
    sp= (size_t*) (td + 1);    
    sp+=(stack_size>>1);                // setup initial stack
    
    // when main() returns a value, it passes it in r0
    // as r0 is also the register to pass single int arguments by
    // gcc convention, we can just put the address of exit on the stack.
    
    *(--sp)=(size_t) &exit;
    
    // we have to construct a stack stub so tm_switcher,
    // systime_handler and the ROM routine can fill the 
    // right values on startup.
    
    *(--sp)=(size_t) code_start;        // entry point   < these two are for
    *(--sp)=0;                          // ccr           < rte in ROM
    *(--sp)=0;                          // r6      < pop r6 in ROM
    *(--sp)=(size_t)
	&rom_ocia_return;         // ROM return < rts in systime_handler
  
    *(--sp)=(size_t) argc;              // r0      < pop r0 in systime handler
    *(--sp)=(size_t) argv;              // r1..r3  < pop r1 in systime handler
    *(--sp)=0;
    *(--sp)=0;
    *(--sp)=(size_t)              
	&systime_tm_return;       // systime return < rts in tm_switcher
    
    *(--sp)=0;                        // r4..r6  in tm_switcher
    *(--sp)=0;
    *(--sp)=0;

    td->sp_save=sp;                   // save sp for tm_switcher
    td->tflags |= T_WAITING;          // task is ready for execution
    td->next = td->prev = td;
    
    grab_kernel_lock();
    make_running(td);
    nb_tasks++;
    release_kernel_lock();
    
    return td;
}

//! exit task, returning code
/*! \param code The return code
 
    \todo save return code; temporarily?
    \bug  ignores return code.
*/
void exit(int code) {
    kernel_lock = 0;
    kill (ctid);
    for(;;);
}

//! delay execution allowing other tasks to run.
/*! \param msec sleep duration in milliseconds
    \return number of milliseconds left if interrupted, else 0.
 */
unsigned int msleep(unsigned int msec)
{
    grab_kernel_lock();
    wait_timeout(msec);
    release_kernel_lock();
    return 0;
}

//! delay execution allowing other tasks to run.
/*! \param sec sleep duration in seconds
    \return number of seconds left if interrupted, else 0.
 */
unsigned int sleep(unsigned int sec)
{
    return msleep(1000*sec)/1000;
}

//! request that the specified task shutdown as soon as possible.
/*! \param td id of the task to be shutdown
 */
void shutdown_task(tid_t td) {
    if ((td->tflags & T_STATE) == T_WAITING)
	make_running(td);
    td->tflags |= T_SHUTDOWN;
}

//! request that tasks with any of the specified flags shutdown.
/*! \param flags T_USER to shutdown user tasks, T_USER | T_KERNEL for all tasks
 */
void shutdown_tasks(tflags_t flags) {
    tdata_t *td, *nexttd;
    
    grab_kernel_lock();
    DEBUG("sd:%d",flags);
    
    /* Iterate running queue */
    td = td_idle.next;
    while (td != &td_idle) {
	DEBUG("tdr:%04x",td);
	if ((td->tflags & ~flags & T_KERNEL) == 0) {
	    td->tflags |= T_SHUTDOWN;
	}
	td = td->next;
    }

    /* Now iterate waiting queue (this adds them to running queue) */
    td = waiters.next;
    while (td != &waiters) {
	DEBUG("tdw:%04x",td);
	nexttd = td->next;
	if ((td->tflags & ~flags & T_KERNEL) == 0) {
	    make_running(td);
	    td->tflags |= T_SHUTDOWN;
	}
	td = nexttd;
    }
    
    release_kernel_lock();
}

static void reap_to_death(tid_t tid) {
    /* dequeue thread from run queue */
    tid->prev->next = tid->next;
    tid->next->prev = tid->prev;
    if ((tid->tflags & T_KERNEL)==T_KERNEL)
        --nb_system_tasks;
    --nb_tasks;
    free(tid);                               // free task data
}

tid_t victim;
static void killer(void) {
    reap_to_death(victim);
}
static handler_t killer_handler = {0, killer};

//! kill specified task
/*! \param tid must be valid process ID, or undefined behaviour will result!
*/
void kill(tid_t tid) {
    grab_kernel_lock(); 
    switch (tid->tflags & T_STATE) {
	case T_WAITING:
	    /* Mark as zombie.  
	     * It will be killed by the wait method after releasing lock.
	     */
	    make_running(tid);
	    tid->tflags = (tid->tflags & ~T_STATE) | T_ZOMBIE;
	    break;

	case T_RUNNING:
	    if (tid == ctid) {
		/* delay killing until this thread frees the kernel lock */
		victim = tid;
		enqueue_handler(&killer_handler);
	    } else {
		reap_to_death(tid);
	    }
	    break;

	default:
	    /* thread was already killed */
	    break;
    }
    release_kernel_lock();    
}

//! kill all tasks of prio or lower
/*! \param flags T_USER to shutdown user tasks, T_USER | T_KERNEL for all tasks
*/
void killall(tflags_t flags) {
    tdata_t *td, *nexttd;
    
    grab_kernel_lock();
    
    /* Now iterate running queue */
    td = td_idle.next;
    while (td != &td_idle) {
	nexttd = td->next;
	if ((td->tflags & ~flags & T_KERNEL) == 0) {
	    if (td != ctid) {
		reap_to_death(td);
	    } else {
		victim = td;
		enqueue_handler(&killer_handler);
	    }
	}
	td = nexttd;
    }

    /* iterate waiting queue */
    td = waiters.next;
    while (td != &waiters) {
	nexttd = td->next;
	if ((td->tflags & ~flags & T_KERNEL) == 0) {
	  make_running(td);
	  /* Mark as zombie.  
	   * It will be killed by the wait method after releasing lock.
	   */
	  td->tflags = (td->tflags & ~T_STATE) | T_ZOMBIE;
	}
	td = nexttd;
    }

    release_kernel_lock();
}

//! yield the rest of the current timeslice
/*! (does not speed up the system clock)
*/
void yield(void) {
    grab_kernel_lock();
    if (ctid->next->priority == ctid->priority) {
	tid_t td = ctid->next;
	td->prev = ctid->prev;
	ctid->prev->next = td;
	while (td->priority == ctid->priority)
	    td = td->next;

	ctid->next = td;
	ctid->prev = td->prev;
	td->prev->next = ctid;
	td->prev = ctid;
	/* trigger scheduler */
	kernel_lock &= ~0x80;
    }
    release_kernel_lock();
}

void wait() {
    unsigned char oldlock;

    if (shutdown_requested())
	return;

    ctid->tflags = (ctid->tflags & ~T_STATE) | T_WAITING;
    /* dequeue current thread from run queue */
    ctid->prev->next = ctid->next;
    ctid->next->prev = ctid->prev;

    /* enqueue on waiters queue */
    ctid->next = waiters.next;
    ctid->prev = &waiters;
    waiters.next->prev = ctid;
    waiters.next = ctid;

    unsigned char flags = cli();
    oldlock = kernel_lock;
    kernel_lock = 0;
    tm_switcher(&td_idle);
    kernel_lock = oldlock |0x80;
    sti(flags);
    if ((ctid->tflags & T_STATE) == T_ZOMBIE) {
        ctid->tflags = (ctid->tflags & ~T_STATE) | T_RUNNING;
        kill(ctid);
    }
}

void wait_timeout(unsigned msec) {
    timer_t timer = { 0,  (void (*)(void*)) make_running, ctid };
    add_timer(msec, &timer);
    wait();
    remove_timer(&timer);
}

void wakeup(waitqueue_t *queue) {
    while (queue) {
        make_running(queue->thread);
        queue = queue->next;
    }
}

void wakeup_single(waitqueue_t *queue){
    if (!queue)
	return;
    tid_t best = queue->thread;
    queue = queue->next;
    while (queue) {
	if (best->priority > queue->thread->priority)
	    best = queue->thread;
    }
    make_running(best);
}

/**
 * Add process to wait queue.  May only be called with kernel lock.
 */
void add_to_waitqueue(waitqueue_t **queue, waitqueue_t *entry) {
    entry->thread = ctid;
    entry->next = *queue;
    if (*queue)
        (*queue)->prev = &entry->next;
    *queue = entry;
    entry->prev = queue;
}

wakeup_t wait_event(wakeup_t (*func)(wakeup_t), wakeup_t data)
{
    wakeup_t result = 0;
    while (!shutdown_requested() && !(result = func(data)))
        msleep(5);
    return result;
}

#endif // CONF_TM
