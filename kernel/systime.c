/*! \file systime.c
    \brief  Implementation: system time services
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
 *                  David Van Wagner <davevw@alumni.cse.ucsc.edu>
 */

/*
 *  2000.05.01 - Paolo Masetti <paolo.masetti@itlug.org>
 *
 * - Added battery indicator handler
 *
 *  2000.08.12 - Rossz Vámos-Wentworth <rossw@jps.net>
 *
 * - Added idle shutdown handler
 *
 */

#include <config.h>

#ifdef CONF_TIME

#include <mem.h>
#include <sys/time.h>
#include <sys/h8.h>
#include <sys/irq.h>
#include <sys/dmotor.h>
#include <sys/handlers.h>
#include <sys/tm.h>

///////////////////////////////////////////////////////////////////////////////
//
// Global Variables
//
///////////////////////////////////////////////////////////////////////////////

//! current system time in ms
/*! \warning This is a 32 bit value which will overflow after 49.7 days
             of continuous operation.
*/
volatile time_t sys_time;

///////////////////////////////////////////////////////////////////////////////
//
// Internal Variables
//
///////////////////////////////////////////////////////////////////////////////

static void run_timers(void);

#define MAX_TIMER 20
handler_t timers_handler = { 0, &run_timers };
static volatile unsigned int first_delay;
static timer_t *timer_queue;


///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

void add_timer(unsigned delay, timer_t *timer) {
    timer_t *prev = timer_queue;
    if (!prev) {
	first_delay = delay;
	timer->next = 0;
	timer_queue = timer;
    } else if (delay < first_delay) {
	/* We need to lock out timer interrupt to guarantee that the
	 * previously enqueued timers will be triggered at the expected time.
	 */
	unsigned char flags = cli();
	prev->delay = first_delay - delay;
	first_delay = delay;
	sti(flags);
	timer->next = prev;
	timer_queue = timer;
    } else {
	timer_t *next;

	delay -= first_delay;
	while ((next = prev->next) && next->delay < delay) {
	    delay -= next->delay;
	    prev = next;
	}
	prev->next = timer;
	timer->delay = delay;
	timer->next = next;
	if (next)
	    next->delay -= delay;
    }
}

/* Get the time until timer will expire.
 * @param timer the timer to query.
 * @returns time until timer will expire, or -1 if timer is not enqueued
 */
int get_timer_count(timer_t *timer) {
    timer_t *t = timer_queue;
    unsigned int delay = 0;
    while (t != NULL) {
	delay += t->delay;
	if (t == timer)
	    return delay + first_delay;
	t = t->next;
    }
    return -1;
}

void remove_timer(timer_t *timer) {
    unsigned char flags;
    timer_t *prev = timer_queue, *next;
    if (prev == timer) {
	flags = cli();
	if ((next = timer_queue->next)) {
	    first_delay += next->delay;
	} else
	    first_delay = 0;
	sti(flags);
	timer_queue = next;
    } else if (prev) {
	while ((next = prev->next)) {
	    if (next == timer) {
		if ((next = timer->next))
		    next->delay += timer->delay;
		prev->next = next;
		return;
	    }
	    prev = next;
	}
    }
}

void run_timers(void) {
    timer_t *ptr, *next;
    /* Sometimes a timer may vanish at the moment it would be
     * triggered.  In this case first_delay is non-zero, or 
     * timer_queue is null.
     */
    while (!first_delay && (ptr = timer_queue)) {
	timer_queue = next = ptr->next;
	if (next)
	    first_delay = next->delay;
	
	/* Now timer_queue is sane, again. Call handler. */
	(*ptr->code)(ptr->data);
    }
}


//! subsystem handler for every msec
/*! this is the pulse of the system (subsystems).
    It just increases sys_time and turns off active sensors to
    settle down.  The remaining functions are done in secondary
    handler.
*/
extern void pretimer_interrupt(void);
//! subsystem handler for every msec
/*! this is the other part of the pulse of the system.
    sound, motor and lcd driver calls are initiated here.
    task_switch_handler is called from here as well.
*/
extern void timer_interrupt(void);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
__asm__("\n\
.text\n\
.align 1\n"
#ifdef CONF_DSENSOR
#ifndef CONF_DSENSOR_FAST
".global _pretimer_interrupt\n\
_pretimer_interrupt:\n\
	; r6 saved by ROM\n\
\n\
	; disable active sensor so it can be read.\n\
	mov.b @_ds_channel,r6l	; r6l = current channel\n\
	bclr r6l,@_PORT6:8		; set output inactive for reading\n\
	; the actual reading is startet in timer_interrupt\n\
	; this allows the sensor to settle\n\
        bclr  #3,@0x91:8                ; reset compare A IRQ flag\n\
        rts\n\n"
#endif
#endif
	"\n\
.text\n\
.align 1\n\
.global _timer_interrupt\n\
.global _systime_tm_return\n\
.global _irq_return\n\
\n\
\n\
_timer_interrupt:\n\
\n"
#if defined(CONF_DSENSOR) && !defined(CONF_DSENSOR_FAST)
"	bclr  #2,@0x91:8                ; reset compare B IRQ flag\n"
"	; The settle time for reading the value from active sensor\n\
	; finish here.  Now start sampling the sensor.\n\
\n\
	;\n\
	bset #0x5,@_AD_CSR:8		; go!\n\
\n"
#else
"	bclr  #3,@0x91:8                ; reset compare A IRQ flag\n"
#endif
"\n\
	push r0                         ; this reg isn't saved by rom\n\
"
#ifdef CONF_DMOTOR
"	jsr _dm_interrupt               ; call motor driver\n"
#endif // CONF_DMOTOR

#ifdef CONF_DKEY
"	jsr _dkey_interrupt             ; call key handler\n"
#endif // CONF_DKEY
        "\n\
	mov.w @_sys_time+2,r6           ; lower 16 bits\n\
	adds  #1,r6                     ; inc lower 16 bits\n\
	mov.w r6,@_sys_time+2\n\
	bne   sys_nohigh                ; if carry, inc upper 8 bits\n\
	mov.w @_sys_time,r6             ; \n\
	adds  #1,r6                     ; inc upper 16 bits\n\
	mov.w r6,@_sys_time\n\
sys_nohigh: \n\
\n\
	mov.w @_first_delay,r6          ; check first timer\n\
	beq   no_enqueue                ; is timer running at all?\n\
        subs  #1,r6\n\
        mov.w r6,@_first_delay\n\
	bne   no_enqueue\n\
"
	enqueue_handler_irq(timers_handler)
"\nno_enqueue:\n"

#ifdef CONF_DSENSOR_FAST
"	; Start sampling the third sensor and set zeroth inactive.\n\
\n\
	;\n\
	bclr #0x0,@_PORT6:8		; set output 0 inactive for reading\n\
	bset #0x5,@_AD_CSR:8		; poll third sensor!\n"
#endif

#ifdef CONF_TM
        "\n\
_irq_return:\n\
	mov.b @_kernel_lock,r6l	      ; check critical section\n\
	bne no_handlers               ; is locked? -> do nothing\n\
\n\
	push r1\n\
	push r2\n\
	push r3\n\
        mov.w #_td_idle, r0           ; switch to the idle thread\n\
        jsr _tm_switcher\n\
_systime_tm_return:\n\
	pop  r3\n\
	pop  r2\n\
	pop  r1\n\
no_handlers:\n"
#endif // CONF_TM

        "\n\
	pop  r0\n\
        rts\n"
);

#endif // DOXYGEN_SHOULD_SKIP_THIS


//! initialize system timer
/*! task switcher initialized to empty handler
    motors turned off
*/
void systime_init(void) {
  systime_shutdown();                           // shutdown hardware

  sys_time=0l;                                  // init timer

#if defined(CONF_DSENSOR) && !defined(CONF_DSENSOR_FAST)
  // configure 16-bit timer
  // compare B IRQ will fire after   10 usec
  // compare A IRQ will fire after 1000 usec
  // counter is then reset
  //
  T_CSR  = TCSR_RESET_ON_A;
  T_CR   = TCR_CLOCK_32;
  T_OCR &= ~TOCR_OCRB;  /* select register A */
  T_OCRA = 500;
  T_OCR |= TOCR_OCRB;   /* select register B */
  T_OCRB =   5;
  T_CNT  =   6;
  T_CSR &= ~TCSR_OCA;
  T_CSR &= ~TCSR_OCB;

  ocia_vector = &pretimer_interrupt;
  ocib_vector = &timer_interrupt;
  T_IER |= (TIER_ENABLE_OCB | TIER_ENABLE_OCA);
#else
  // configure 16-bit timer
  // compare A IRQ will fire after 1000 usec
  // counter is then reset
  //
  T_CSR  = TCSR_RESET_ON_A;
  T_CR   = TCR_CLOCK_32;
  T_OCR &= ~TOCR_OCRB;  /* select register A */
  T_OCRA = 500;
  T_CNT  =   1;
  T_CSR &= ~TCSR_OCA;

  ocia_vector = &timer_interrupt;
  T_IER |= (TIER_ENABLE_OCA);
#endif
}

//! shutdown system timer
/*! will also stop task switching and motors.
*/
void systime_shutdown(void) {
  T_IER &= ~(TIER_ENABLE_OCA | TIER_ENABLE_OCB);  // unhook compare A/B IRQs
  WDT_CSR &= ~WDT_CSR_ENABLE;                     // disable wd timer
}

//! retrieve the current system time
/*! \return number of msecs the system has been running
 *  Since sys_time is 32bits, it takes more than one
 *  instruction to retrieve; the timer interrupt can fire mid
 *  retrieval; causing the upper and lower 16bits to be
 *  unmatched (lower 16bits could overflow and reset to
 *  0, while upper 16bits were already read).  So if we see
 *  0 in the lower half, we reread the upper half to be sure.
 */
extern time_t get_system_up_time(void);
__asm__("\n\
.text\n\
.align 1\n\
.global _get_system_up_time\n\
_get_system_up_time:\n\
    mov.w @_sys_time+2, r1\n\
    mov.w @_sys_time,   r0\n\
    bne  1f\n\
    mov.w @_sys_time+2, r1\n\
1:\n\
    rts\n\
");




#endif // CONF_TIME
