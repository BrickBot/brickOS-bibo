/*! \file   dsound.c
    \brief  Implementation: direct sound control
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
 *                  Michael Nielsen <mic@daimi.aau.dk>
 *    	      	    Kyosuke Ishikawa <kyosuke@da2.so-net.ne.jp>
 *                  Martin Cornelius <Martin.Cornelius@t-online.de>
 */

//#define DEBUG(x...) debug_printf(x)
#define DEBUG(x...)
#include <sys/dsound.h>

#ifdef CONF_DSOUND

#include <sys/bitops.h>
#include <sys/h8.h>
#include <sys/irq.h>
#include <sys/time.h>

#include <conio.h>
#include <sys/tm.h>

///////////////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////////////

//! note pitch -> frequency generator lookup table, index 0 = rest and 1 ^= A0
static const unsigned pitch2freq[]={ /* rest ( */ 0x0000, /* ) */         0x8d03, 0x8603, 0x7d03,  // Octave 0
  0x7703, 0x7003, 0x6a03, 0x6303, 0x5e03, 0x5903, 0x5403, 0x4f03, 0x4a03, 0x4603, 0x4203, 0xfd83,  // Octave 1
  0xee83, 0xe083, 0xd483, 0xc783, 0xbc83, 0xb283, 0xa883, 0x9e83, 0x9583, 0x8d83, 0x8583, 0x7e83,  // Octave 2
  0x7683, 0x7083, 0x6983, 0x6383, 0x5e83, 0x5983, 0x5383, 0x4f83, 0x4a83, 0x4683, 0x4283, 0xfc02,  // Octave 3
  0xee02, 0xe102, 0xd402, 0xc802, 0xbd02, 0xb202, 0xa802, 0x9e02, 0x9502, 0x8d02, 0x8502, 0xfc82,  // Octave 4
  0xee82, 0xe082, 0xd482, 0xc882, 0xbd82, 0xb282, 0xa882, 0x9e82, 0x9682, 0x8d82, 0x8582, 0x7e82,  // Octave 5
  0x7682, 0x7082, 0x6982, 0x6382, 0x5e82, 0x5882, 0x5382, 0x4f82, 0x4a82, 0x4682, 0x4282, 0xfc01,  // Octave 6
  0xee01, 0xe001, 0xd401, 0xc801, 0xbd01, 0xb201, 0xa801, 0x9e01, 0x9501, 0x8d01, 0x8501, 0x7e01,  // Octave 7
  0x7601, 0x7001, 0x6901, 0x6301, 0x5e01, 0x5801, 0x5301, 0x4f01, 0x4a01, 0x4601                   // Octave 8
//  C     C#/Db     D     D#/Eb     E       F     F#/Gb     G     G#/Ab     A     A#/Bb     B      // Note Columns
};

//! single beep
static const note_t sys_beep[]={
  {PITCH_TEMPO, 50}, {PITCH_A4 , 4}, {PITCH_END, 0}
};

//! system sound data
const note_t *dsound_system_sounds[]={
    sys_beep
};
    
unsigned dsound_64th_ms;   	      	      	//!< length of 1/16 note in ms
unsigned dsound_internote_ms; 		      	//!< length of internote spacing in ms
const note_t volatile * dsound_next_note;  	//!< pointer to current note

static volatile int internote; 	      	      	//!< internote delay
static int repcnt = 0;
waitqueue_t *dsound_finished = NULL;


//////////////////////////////////////////////////////////////////////////////
//
// Internal Functions
//
///////////////////////////////////////////////////////////////////////////////

//! start playing a pause/rest (aka stop playing freq)
static inline void play_pause() {
  T0_CR  = 0x00;      	      	 // timer 0 off
}

//! start playing a given frequency
static inline void play_freq(unsigned freq) {
  if (0 == freq) {
    play_pause();
  } else {
    unsigned char CKSmask = freq & 0xff;
    unsigned char match = freq >> 8;

    T0_CR  = 0x00;                 // timer off
    T0_CNT = 0x00;	         // counter reset
  
#if 0  
    bit_load(CKSmask,0x7);      	 // set ICKS0
    bit_store(&STCR,0x0);
#else  
    if (CKSmask & 0x80)
      STCR |=  0x01;             // ICKS0 = 1
    else
      STCR &= ~0x01;             // ICKS0 = 0
#endif
  
    T0_CORA = match;               // set compare match A
    T0_CR   = CR_CLEAR_ON_A | (CKSmask &0x3);
  }
}

 
//////////////////////////////////////////////////////////////////////////////
//
// System functions
//
///////////////////////////////////////////////////////////////////////////////

static void dsound_handler(void *data);

timer_t dsound_note_timer = {
    0, dsound_handler, 0, 0
};

//! sound handler, called from system timer interrupt
static void dsound_handler(void *data) {
    if(internote) {
	play_pause();
	add_timer(internote, &dsound_note_timer);
	internote=0;
	return;
    }

    for (;;) {
	unsigned char pitch = dsound_next_note->pitch;
	DEBUG("note: %d,%d (%d)", pitch, dsound_next_note->length,
	      dsound_64th_ms * dsound_next_note->length - dsound_internote_ms);

        if (pitch<=PITCH_MAX) {
	    if(pitch!=PITCH_REST)
		play_freq(pitch2freq[pitch]);
	    else
		play_pause();
	    
	    add_timer(dsound_64th_ms * dsound_next_note->length
		      - dsound_internote_ms, &dsound_note_timer);
	    dsound_next_note++;
	    internote = dsound_internote_ms;
	    return;
	} else if (pitch == PITCH_TEMPO) {
	    dsound_64th_ms = dsound_next_note->length;
	    dsound_next_note++;
	} else if (pitch == PITCH_INTERNOTE) {
	    dsound_internote_ms = dsound_next_note->length;
	    dsound_next_note++;
	} else if (pitch == PITCH_REPEAT) {
	    if (dsound_next_note->length
		&& repcnt++ == dsound_next_note->length) {
		repcnt = 0;
		dsound_next_note += 2;
	    } else {
		dsound_next_note -= ((dsound_next_note[1].pitch) << 8) 
		    | (dsound_next_note[1].length);
	    }
	} else { /* PITCH_END or broken sound */
	    play_pause();
	    dsound_next_note=0;  
	    internote=0;
	    wakeup(dsound_finished);
	    return;
	}
    }
}

//! initialize sound driver
void dsound_init() {
    dsound_64th_ms=DSOUND_DEFAULT_64th_ms;
    dsound_internote_ms=DSOUND_DEFAULT_internote_ms;
    dsound_stop();
    T0_CSR  = CSR_TOGGLE_ON_A;     // Output toggles on compare Match A
}

//! shutdown sound driver
void dsound_shutdown() {
    dsound_stop();
}

//////////////////////////////////////////////////////////////////////////////
//
// User functions
//
///////////////////////////////////////////////////////////////////////////////

//! start playing a sound
void dsound_play(const note_t *notes) {
    grab_kernel_lock();
    remove_timer(&dsound_note_timer);
    dsound_next_note = notes;
    internote = 0;
    repcnt = 0;
    add_timer(1, &dsound_note_timer);
    release_kernel_lock();
}

//! stop playing sound
void dsound_stop(void) {
    grab_kernel_lock();
    remove_timer(&dsound_note_timer);
    play_pause();
    dsound_next_note=0;  
    internote=0;
    wakeup(dsound_finished);
    release_kernel_lock();
}

//! wait until sound has finished
int dsound_wait(void) {
    waitqueue_t entry;
    grab_kernel_lock();
    add_to_waitqueue(&dsound_finished, &entry);
    while (dsound_next_note && !shutdown_requested())
        wait();
    remove_from_waitqueue(&entry);
    release_kernel_lock();
    return !shutdown_requested();
}
#endif // CONF_DSOUND
