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


#ifdef CONF_EXTENDED_MUSIC
//! note pitch -> frequency generator lookup table, index 0 = rest and 1 ^= A0
static const unsigned short pitch2freq[]={   /*  rest ( */ 0x0000, /* )  */            FREQ_A0, FREQ_Am0, FREQ_B0,  // Octave 0
  FREQ_C1, FREQ_Cm1, FREQ_D1, FREQ_Dm1, FREQ_E1, FREQ_F1, FREQ_Fm1, FREQ_G1, FREQ_Gm1, FREQ_A1, FREQ_Am1, FREQ_B1,  // Octave 1
  FREQ_C2, FREQ_Cm2, FREQ_D2, FREQ_Dm2, FREQ_E2, FREQ_F2, FREQ_Fm2, FREQ_G2, FREQ_Gm2, FREQ_A2, FREQ_Am2, FREQ_B2,  // Octave 2
  FREQ_C3, FREQ_Cm3, FREQ_D3, FREQ_Dm3, FREQ_E3, FREQ_F3, FREQ_Fm3, FREQ_G3, FREQ_Gm3, FREQ_A3, FREQ_Am3, FREQ_B3,  // Octave 3
  FREQ_C4, FREQ_Cm4, FREQ_D4, FREQ_Dm4, FREQ_E4, FREQ_F4, FREQ_Fm4, FREQ_G4, FREQ_Gm4, FREQ_A4, FREQ_Am4, FREQ_B4,  // Octave 4
  FREQ_C5, FREQ_Cm5, FREQ_D5, FREQ_Dm5, FREQ_E5, FREQ_F5, FREQ_Fm5, FREQ_G5, FREQ_Gm5, FREQ_A5, FREQ_Am5, FREQ_B5,  // Octave 5
  FREQ_C6, FREQ_Cm6, FREQ_D6, FREQ_Dm6, FREQ_E6, FREQ_F6, FREQ_Fm6, FREQ_G6, FREQ_Gm6, FREQ_A6, FREQ_Am6, FREQ_B6,  // Octave 6
  FREQ_C7, FREQ_Cm7, FREQ_D7, FREQ_Dm7, FREQ_E7, FREQ_F7, FREQ_Fm7, FREQ_G7, FREQ_Gm7, FREQ_A7, FREQ_Am7, FREQ_B7,  // Octave 7
  FREQ_C8, FREQ_Cm8, FREQ_D8, FREQ_Dm8, FREQ_E8, FREQ_F8, FREQ_Fm8, FREQ_G8, FREQ_Gm8, FREQ_A8                      // Octave 8
//   C      C#/Db       D      D#/Eb       E        F      F#/Gb       G      G#/Ab       A      A#/Bb       B      // Note Columns
};
#endif

//! single beep
static const note_t sys_beep[]={
  {PITCH_TEMPO, TEMPO_FROM_BPM(QUARTER, 75)}, {PITCH_Bb4 , SIXTEENTH}, {PITCH_END, 0}
};

//! system sound data
const note_t *dsound_system_sounds[]={
    sys_beep
};
    
unsigned char dsound_64th_ms;     	      	//!< length of 1/16 note in ms
unsigned char dsound_internote_ms; 	    	//!< length of internote spacing in ms
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
static inline void play_freq(unsigned short freq) {
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
        pitch_conf_t pitch = dsound_next_note->pitch;
        DEBUG("note: %d,%d (%d)", pitch, dsound_next_note->length,
              dsound_64th_ms * dsound_next_note->length - dsound_internote_ms);

#ifdef CONF_EXTENDED_MUSIC
        if (pitch<=PITCH_MAX)
#else
        if (((pitch & 0xFF00) != 0) || pitch == PITCH_REST)
#endif
        {
#ifdef CONF_EXTENDED_MUSIC
            play_freq(pitch2freq[pitch]);
#else
            play_freq(pitch);
#endif

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
char dsound_wait(void) {
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
