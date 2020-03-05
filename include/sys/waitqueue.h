/*! \file include/semaphore.h
    \brief  Interface: POSIX 1003.1b semaphores for task synchronization
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
 *                  Henner Zeller <H.Zeller@acm.org> (sem_timedwait())
 */

#ifndef __waitqueue_h__
#define __waitqueue_h__

#ifdef  __cplusplus
extern "C" {
#endif
    
#include <config.h>
#include <tm.h> /* tid_t */

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

typedef struct waitqueue_t{
    struct waitqueue_t *next;
    struct waitqueue_t **prev;
    tid_t              thread;
} waitqueue_t;                                  //!< the semaphore data-type
    
/**
 * Add process to wait queue.  May only be called with kernel lock.
 */
extern void add_to_waitqueue(waitqueue_t **queue, waitqueue_t *entry);

/**
 * Remove process from wait queue.  May only be called with kernel lock.
 */
extern inline void remove_from_waitqueue(waitqueue_t *entry) {
    *entry->prev = entry->next;
}

/**
 * Wake up all processes on a queue.
 */
extern void wakeup(waitqueue_t *queue);

/**
 * Wake up a single process from the queue.
 */
extern void wakeup_single(waitqueue_t *queue);

extern void wait(void);
extern void wait_timeout(unsigned timeout);

#ifdef  __cplusplus
}
#endif

#endif // __waitqueue_h__
