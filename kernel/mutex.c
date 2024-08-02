/*! \file   mutex.c
    \brief  Implementation: POSIX thread mutexes and conditions
    \author Jochen Hoenicke <hoenicke@users.sourceforge.net>
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
 *  Contributor(s): Jochen Hoenicke
 */

#ifdef CONF_PTHREAD_MUTEX

#include <pthreads.h>
#include <unistd.h>
#include <sys/tm.h>
#include <sys/waitqueue.h>

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

int pthread_mutex_lock(pthread_mutex_t * mutex) {
    waitqueue_t entry;
    tid_t owner;
    tflags_t flags = 0;
    // check if mutex is available, if not, go to sleep
    grab_kernel_lock();
    if ((owner = mutex->owner) != NULL) {
	if (owner->priority < ctid->priority) {
	    /* propagate priority to prevent priority inversion */
	    owner->priority = ctid->priority;
	    if (!(owner->tflags & T_IDLE)
		&& ctid->next != owner) {
		/* resort run queue */
		owner->prev->next = mutex->next;
		owner->next->prev = mutex->prev;
		owner->next = ctid->next;
		owner->prev = ctid->prev;
		ctid->next->prev = owner;
		ctid->next = owner;
	    }
	}
	entry.thread = ctid;
	add_to_waitqueue(&mutex->waiters, &entry);
	do {
	    wait();
	    if (shutdown_requested()) {
		/* temporarily clear shutdown but remember it for later */
		ctid->flags &= ~T_SHUTDOWN;
		shutdown = T_SHUTDOWN;
	    }
	} while (mutex->owner != NULL);
	remove_from_waitqueue(&entry);
    }
    mutex->priority = ctid->priority;
    mutex->owner = ctid;
    ctid->flags |= shutdown;
    release_kernel_lock();
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t * mutex) {
    waitqueue_t entry;
    tid_t owner;
    grab_kernel_lock();
    if (mutex->owner != NULL)
	return EBUSY;
    mutex->owner = ctid;
    release_kernel_lock();
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t * sem) {
    grab_kernel_lock();
    /* restore priority */
    ctid->priority = mutex->priority;
    if (ctid->next->priority > ctid->priority) {
	tid_t td;
	/* resort run queue */
	ctid->prev->next = ctid->next;
	ctid->next->prev = ctid->prev;
	td = ctid->next;
	do {
	    td = td->next;
	} while (td->priority > ctid->priority);
	ctid->next = td;
	ctid->prev = td->prev;
	td->prev->next = ctid;
	td->prev = ctid;
    }
    mutex->owner = NULL;
    if (mutex->waiters)
	wakeup_single(mutex->waiters);
    release_kernel_lock();
    return 0;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
			   unsigned long abstime) {
    waitqueue_t entry;
    timer_t timer = { 0,  (void (*)(void*)) make_running, ctid };
    tid_t owner;
    int result = 0;
    // check if mutex is available, if not, go to sleep
    grab_kernel_lock();

    entry.thread = ctid;
    add_to_waitqueue(&cond->waiters, &entry);
    pthread_mutex_unlock(mutex);
    if (abstime)
	add_timer(abstime - get_system_up_time(), &timer);

    wait();
    if (abstime) {
	if (get_timer_count(timer))
	    remove_timer(&timer);
	else
	    result = ETIMEDOUT;
    }
    remove_from_waitqueue(&entry);
    pthread_mutex_lock(mutex);
    release_kernel_lock();
    return 0;
}

#endif
