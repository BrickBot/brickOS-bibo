/*! \file   dkey.c
    \brief  Implementation: debounced key driver
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

#include <dkey.h>

#ifdef CONF_DKEY

#include <unistd.h>
#include <sys/tm.h>
#include <sys/handlers.h>
#include <sys/waitqueue.h>
#include <sys/program.h>

///////////////////////////////////////////////////////////////////////////////
//
// Global Variables
//
///////////////////////////////////////////////////////////////////////////////

volatile unsigned char dkey_multi;   	//! multi-key state
volatile unsigned char dkey;          	//! single key state

static void dkey_callback(void);
handler_t dkey_handler_info = {0, dkey_callback};
waitqueue_t *dkey_system_waitqueue  = NULL;
waitqueue_t *dkey_program_waitqueue = NULL;
waitqueue_t *dkey_user_waitqueue    = NULL;

///////////////////////////////////////////////////////////////////////////////
//
// Internal Variables
//
///////////////////////////////////////////////////////////////////////////////

char dkey_timer __attribute__ ((unused)); //! debouncing timer

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////
#ifndef DOXYGEN_SHOULD_SKIP_THIS
__asm__("\n\
.text\n\
.align 1\n\
.global _dkey_interrupt\n\
_dkey_interrupt:\n\
   mov.b @_dkey_timer,r6l	   ; check debouncing timer==0\n\
   beq dkey_check\n\
\n\
     dec r6l\n\
     mov.b r6l,@_dkey_timer\n\
     rts\n\
\n\
dkey_check:\n\
   sub.b r6l,r6l      	      	   ; generate button codes\n\
      	      	      	      	   ; from PORT4/PORT7 in r6l\n\
   mov.b @_PORT4,r6h\n\
   bld #1,r6h\n\
   bist #0,r6l\n\
   bld #2,r6h\n\
   bist #1,r6l\n\
\n\
   mov.b @_PORT7,r6h\n\
   bld #6,r6h\n\
   bist #2,r6l\n\
   bld #7,r6h\n\
   bist #3,r6l\n\
\n\
   mov.b @_dkey_multi,r6h\n\
   xor.b r6l,r6h      	      	  ; create mask of changed positions in r6h\n\
   beq dkey_same\n\
\n\
     mov.b r6l,@_dkey_multi\n\
\n\
     and.b r6h,r6l    	      	  ; mask out unchanged positions\n\
     mov.b r6l,@_dkey\n\
\n\
     mov.b #100,r6l  	      	  ; set debouncing timer\n\
     mov.b r6l,@_dkey_timer\n\
\n"
     enqueue_handler_irq(dkey_handler_info)
     
"\n\
dkey_same:\n\
   rts\n\
");
#endif // DOXYGEN_SHOULD_SKIP_THIS

char dkey_wait(unsigned char is_depressed, unsigned char keymask) {
    waitqueue_t entry;
    grab_kernel_lock();
    if (is_program_running()) {
      add_to_waitqueue(&dkey_user_waitqueue, &entry);
    } else {
      add_to_waitqueue(&dkey_system_waitqueue, &entry);
    }
    while ((dkey_multi & keymask) ? !is_depressed : is_depressed
        && !shutdown_requested()) {
      wait();
    }
    if (is_program_running()) {
      // For dkey_user_waitqueue
      remove_from_waitqueue(&entry);
    } else {
      // For dkey_system_waitqueue
      remove_from_waitqueue(&entry);
    }
    release_kernel_lock();
    return !shutdown_requested();
}

//! get and return a single key press, after waiting for it to arrive
unsigned char getchar(void) {
    unsigned char dkey_old, dkey_new;
    waitqueue_t entry;
    grab_kernel_lock();
    if (is_program_running()) {
      add_to_waitqueue(&dkey_user_waitqueue, &entry);
    } else {
      add_to_waitqueue(&dkey_system_waitqueue, &entry);
    }
    do {
      dkey_old = dkey;
      wait();
    } while (!(dkey & ~dkey_old) && !shutdown_requested());
    dkey_new = dkey;
    if (is_program_running()) {
      remove_from_waitqueue(&entry);
    } else {
      remove_from_waitqueue(&entry);
    }
    release_kernel_lock();
    return dkey_new & ~dkey_old;
}

void dkey_callback(void) {
    if (is_program_running()) {
      wakeup(dkey_user_waitqueue);
      wakeup(dkey_program_waitqueue);
    } else {
      wakeup(dkey_system_waitqueue);
    }
}

#endif // CONF_DKEY
