/*! \file   semaphore.c
    \brief  Implementation: POSIX 1003.1b semaphores for process synchronization.
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

#include <semaphore.h>

#ifdef CONF_SEMAPHORES

#include <unistd.h>
#include <sys/tm.h>
#include <sys/waitqueue.h>

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

//! wait on a semaphore
/*! \param sem a valid semaphore

    suspends the calling thread until the semaphore
    has non-zero count. It  then  atomically
    decreases the semaphore count.
    
    implemented with wait_event().                 
*/
int sem_wait(sem_t * sem) {
    waitqueue_t entry;
    // check if semaphore is available, if not, go to sleep
    grab_kernel_lock();
    if (sem->value == 0) {
	entry.thread = ctid;
	add_to_waitqueue(&sem->waiters, &entry);
	do {
	    wait();
	    if (shutdown_requested()) {
		remove_from_waitqueue(&entry);
		return -1;
	    }
	} while (sem->value == 0);
	remove_from_waitqueue(&entry);
    }
    sem->value--;
    release_kernel_lock();
    return 0;
}

//! non-blocking check on a semaphore
/*! \param sem a valid semaphore

    a non-blocking variant of sem_wait.  If the
    semaphore has non-zero count, the  count
    is   atomically   decreased  and  sem_trywait  immediately
    returns 0.  If the semaphore count  is  zero,  sem_trywait
    immediately returns with error EAGAIN.
   
    this is IRQ handler safe.
*/
int sem_trywait(sem_t * sem) {
    int result = EAGAIN;
    grab_kernel_lock();
    if (sem->value != 0) {
	sem->value--;
	result = 0;
    }
    release_kernel_lock();
    return result;
}

//! Post a semaphore
/*! sem_post()  atomically  increases the count of the semaphore
 *  pointed to by {sem}.  This function  never  blocks  and  can
 *  safely be used in asynchronous signal handlers.
 *  \param sem a pointer to the semaphore to be signaled
 *  \return (always returns 0)
*/
int sem_post(sem_t * sem) {
    grab_kernel_lock();
    sem->value++;
    if (sem->waiters)
	wakeup_single(sem->waiters);
    release_kernel_lock();
    return 0;
}

#endif
