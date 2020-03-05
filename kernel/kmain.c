/*! \file kmain.c
    \brief  Implementaion: Main kernel loop
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
 *                  Kekoa Proudfoot <kekoa@graphics.stanford.edu>
 *                  Lou Sortman <lou (at) sunsite (dot) unc (dot) edu>
 */

#include <sys/mm.h>
#include <sys/time.h>
#include <sys/tm.h>
#include <sys/irq.h>
#include <sys/lcd.h>
#include <sys/dsensor.h>
#include <sys/dmotor.h>
#include <sys/dsound.h>
#include <sys/lnp.h>
#include <sys/lnp-logical.h>
#include <sys/program.h>
#include <sys/vis.h>
#include <rom/system.h>

#include <dbutton.h>

#include <string.h>
#include <conio.h>
#include <unistd.h>
///////////////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////////////

extern char __bss;		//!< the start of the uninitialized data segment
extern char __bss_end;	//!< the end of the uninitialized data segment

//! the start address of high memory segment (normally 0xf000)
extern void __text_hi;
extern void __text_hi_load;
//! the end address of high memory segment
extern void __text_hi_size;
//! the start address of init text segment
extern void __text_init;
extern void __text_init_load;
//! the end address of init text segment
extern void __text_init_size;

#if defined(CONF_DSOUND) && defined(CONF_ON_OFF_SOUND)
static const note_t on_sound[]={{PITCH_TEMPO, 50}, {PITCH_G4, 1}, {PITCH_G5, 1}, {PITCH_END, 0}};
static const note_t off_sound[]={{PITCH_TEMPO, 50}, {PITCH_C4, 1}, {PITCH_C3, 1}, {PITCH_END, 0}};
#endif
#ifdef CONF_ASCII
static const char on_text[] = "ON";
static const char off_text[] = "OFF";
#else
#ifdef CONF_CONIO
static const char on_text[] = { 0, CHAR_O, CHAR_N, 0 };  // ON
static const char off_text[] = { CHAR_O, CHAR_F, CHAR_F, 0 };  // OFF
#else
static const char on_text[] = { '\x01' };
static const char off_text[] = { '\x00' };
#endif
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CONF_PROGRAM
extern int main(int argc, char **argv);     //!< the user main()
#endif

void kmain(void) __attribute__((__section__ (".text.ma"), noreturn));

//! show ON/OFF string
static int show_on_off(const char* display, const note_t *sound);

__TEXT_INIT__ void init_all(void);

//! the beginning of everything
/*! initially called by ROM
 */
void kmain(void)
{
    int reset_after_shutdown;
    /* Install the init segment in the correct place.  The
     * firmware loader puts it in the bss segment, we copy it 
     * to it's final location.
     */
    __asm__("orc #0x80, ccr"); /* disable IRQ*/
    memcpy(&__text_init, &__text_init_load, (size_t)&__text_init_size);
    init_all();
    while (1) {
	power_init();
	lcd_init();
	
#ifdef CONF_DSOUND
	dsound_init();
#endif
#ifdef CONF_DSENSOR
	ds_init();
#endif
#ifdef CONF_DMOTOR
	dm_init();
#endif
#ifdef CONF_LNP
	lnp_init();
	lnp_logical_init();
#endif
#ifdef CONF_TIME
	systime_init();
#endif
#ifdef CONF_VIS
	vis_init();
#endif
#ifdef CONF_PROGRAM
	program_init();
#endif
	
	dkey_multi = KEY_ONOFF;
#if defined(CONF_DSOUND) && defined(CONF_ON_OFF_SOUND)
	show_on_off(on_text, on_sound);
#else
	show_on_off(on_text, NULL);
#endif
	
	cls();
#ifndef CONF_PROGRAM
	lcd_show(man_run);
#ifndef CONF_LCD_REFRESH
	lcd_refresh();
#endif
#endif
	
	// run app
	//
#ifdef CONF_TM
#  ifndef CONF_PROGRAM
	execi(&main,0,0,PRIO_NORMAL,DEFAULT_STACK_SIZE);
#  endif
	/* the task system will start on first timer interrupt */
	while (nb_system_tasks > 1)
	    tm_idle();
#else
	main(0, (void*)0);
#endif

	// ON/OFF + VIEW -> erase firmware
#if defined(CONF_DSOUND) && defined(CONF_ON_OFF_SOUND)
	reset_after_shutdown = show_on_off(off_text, off_sound);
#else
	reset_after_shutdown = show_on_off(off_text, NULL);
#endif	
	
#ifdef CONF_PROGRAM
	program_shutdown();
#endif
#ifdef CONF_VIS
	vis_shutdown();
#endif
#ifdef CONF_LNP
	lnp_shutdown();
	lnp_logical_shutdown();
#endif
#ifdef CONF_DMOTOR
	dm_shutdown();
#endif
#ifdef CONF_DSENSOR
	ds_shutdown();
#endif
#ifdef CONF_TIME
	systime_shutdown();
#endif
	
	if (reset_after_shutdown)
	    rom_reset();
	
	lcd_clear();
	lcd_power_off();
	
	__asm__("andc #0x7f, ccr"); /* enable IRQ*/
	power_off();
	__asm__("orc #0x80, ccr"); /* disable IRQ*/
    }
}

static int show_on_off(const char* display, const note_t *sound) {
    int reset_after_shutdown=0;
    cls();

#ifdef CONF_ASCII
    cputs((char*)display);
#else
#ifdef CONF_CONIO
    cputc_native_user(display[0], display[1], display[2], display[3]);
#else
    lcd_digit(display[0]);
#endif
#endif
#ifndef CONF_LCD_REFRESH
    lcd_refresh();
#endif
#if defined(CONF_DSOUND) && defined(CONF_ON_OFF_SOUND)
    dsound_play(sound);
    while (dsound_next_note != 0) {
	if(dkey_multi & KEY_VIEW)
	    reset_after_shutdown=1;
	tm_idle();
    }
#endif

#ifdef CONF_DKEY
    while ((dkey_multi & KEY_ONOFF)) {
	if(dkey_multi & KEY_VIEW)
	    reset_after_shutdown=1;
	tm_idle();
    }
#else
    while (PRESSED(dbutton(), BUTTON_ONOFF)) {
	if (PRESSED(dbutton(), BUTTON_VIEW))
	    reset_after_shutdown=1;
	tm_idle();
    }
    int deadline = ((int)sys_time) + 100;
    while (deadline - (int)sys_time > 0)
	tm_idle();
#endif
    return reset_after_shutdown;
}

void debug_printf(const char* format, ...) {
    __asm__("nop");
}

__TEXT_INIT__ void init_all(void) {
    unsigned short dummy;
    /* Install the text.hi segment in the correct place.  The
     * firmware loader puts it in the bss segment, we copy it 
     * to it's final location.
     *
     * We use a faster version instead of memcpy.  This version
     * assumes word align addresses.
     */
    __asm__ __volatile__ ("bra init_loop_test\n"
			  "init_loop_head: mov.w @%1+,%0\n"
			  "   mov.w %0,@%3\n"
			  "   adds #2,%3\n"
			  "init_loop_test: cmp %1,%2\n"
			  "   bne init_loop_head\n"
			  : "=&r" (dummy)                     // dummy reg
			  : "r" (&__text_hi_load),            // input
			  "r" (&__text_hi_load + (size_t)&__text_hi_size), 
			  "r" (&__text_hi)
			  : "cc","memory"	                // clobbered
	);

    /* Turn off motor, since writing to hitext manipulates motors */
    motor_controller = 0;

    memset(&__bss, 0, &__bss_end - &__bss);
    tm_init();

#ifdef CONF_MM
    mm_init();
#endif
    kernel_lock = 1;

    free(&__text_init);
}  

