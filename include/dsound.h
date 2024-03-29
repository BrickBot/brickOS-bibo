/*! \file   include/dsound.h
    \brief  Interface: direct control of sound
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
 *                  Froods <froods@alphalink.com.au>
 */

#ifndef __dsound_h__
#define __dsound_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_DSOUND

#include <time.h>

#include <unistd.h>
#include <sys/waitqueue.h>

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

/// the note structure describing a single note.
/// \note a song to play is made up of an array of these structures
/// which is then handed to dsound_play() [in C] or Sound::play() [in C++].
///
/// The last entry in the list should have the {pitch} value set to \ref PITCH_END
///
/// Rests should be specified by placing \ref PITCH_PAUSE in the {pitch} value.
/// The duration of the rest is placed in {length}.
typedef struct {
  unsigned char pitch;      //!< note pitch: 0 = rest, 1 ^= A_0 (~55 Hz)
  unsigned char length;     //!< note length in 1/64ths
} note_t;


//! \defgroup noteDurations standard note/rest durations
//@{

/// Breve note in terms of 64th
#define BREVE                 128

/// Whole note in terms of 64th
#define WHOLE                  64

/// Half note in terms of 64th
#define HALF                   32

/// Quater note in terms of 64th
#define QUARTER                16

/// Eighth note in terms of 64th
#define EIGHTH                  8

/// Sixteenth note in terms of 64th
#define SIXTEENTH               4

/// Thirty-second note in terms of 64th
#define THIRTYSECOND            2

/// Sixty-fourth note in terms of 64th (PITCH_TEMPO)
#define SIXTYFOURTH             1

/// Dotted breve note in terms of 64th
#define BREVE_DOTTED          192

/// Dotted whole note in terms of 64th
#define WHOLE_DOTTED           96

/// Dotted half note in terms of 64th
#define HALF_DOTTED            48

/// Dotted quater note in terms of 64th
#define QUARTER_DOTTED         24

/// Dotted eighth note in terms of 64th
#define EIGHTH_DOTTED          12

/// Dotted sixteenth note in terms of 64th
#define SIXTEENTH_DOTTED        6

/// Thirty-second note in terms of 64th
#define THIRTYSECOND_DOTTED     3
//@}


//  PITCH_H is European equivalent to American B note.

//! \defgroup notePitches standard note pitch values
//@{

//! specify a pause (rest)
#define PITCH_PAUSE   0
#define PITCH_REST    0

/// Note Pitch Value 
#define PITCH_A0      1
#define PITCH_Am0     2
#define PITCH_Bb0     2
#define PITCH_B0      3
#define PITCH_H0      3
#define PITCH_C1      4
#define PITCH_Cm1     5
#define PITCH_Db1     5
#define PITCH_D1      6
#define PITCH_Dm1     7
#define PITCH_Eb1     7
#define PITCH_E1      8
#define PITCH_F1      9
#define PITCH_Fm1    10
#define PITCH_Gb1    10
#define PITCH_G1     11
#define PITCH_Gm1    12
#define PITCH_Ab1    12
#define PITCH_A1     13
#define PITCH_Am1    14
#define PITCH_Bb1    14
#define PITCH_B1     15
#define PITCH_H1     15
#define PITCH_C2     16
#define PITCH_Cm2    17
#define PITCH_Db2    17
#define PITCH_D2     18
#define PITCH_Dm2    19
#define PITCH_Eb2    19
#define PITCH_E2     20
#define PITCH_F2     21
#define PITCH_Fm2    22
#define PITCH_Gb2    22
#define PITCH_G2     23
#define PITCH_Gm2    24
#define PITCH_Ab2    24
#define PITCH_A2     25
#define PITCH_Am2    26
#define PITCH_Bb2    26
#define PITCH_B2     27
#define PITCH_H2     27
#define PITCH_C3     28
#define PITCH_Cm3    29
#define PITCH_Dd3    29
#define PITCH_D3     30
#define PITCH_Dm3    31
#define PITCH_Eb3    31
#define PITCH_E3     32
#define PITCH_F3     33
#define PITCH_Fm3    34
#define PITCH_Gb3    34
#define PITCH_G3     35
#define PITCH_Gm3    36
#define PITCH_Ab3    36
#define PITCH_A3     37
#define PITCH_Am3    38
#define PITCH_Bb3    38
#define PITCH_B3     39
#define PITCH_H3     39
#define PITCH_C4     40
#define PITCH_Cm4    41
#define PITCH_Db4    41
#define PITCH_D4     42
#define PITCH_Dm4    43
#define PITCH_Eb4    43
#define PITCH_E4     44
#define PITCH_F4     45
#define PITCH_Fm4    46
#define PITCH_Gb4    46
#define PITCH_G4     47
#define PITCH_Gm4    48
#define PITCH_Ab4    48
#define PITCH_A4     49
#define PITCH_Am4    50
#define PITCH_Bb4    50
#define PITCH_B4     51
#define PITCH_H4     51
#define PITCH_C5     52
#define PITCH_Cm5    53
#define PITCH_Db5    53
#define PITCH_D5     54
#define PITCH_Dm5    55
#define PITCH_Eb5    55
#define PITCH_E5     56
#define PITCH_F5     57
#define PITCH_Fm5    58
#define PITCH_Gb5    58
#define PITCH_G5     59
#define PITCH_Gm5    60
#define PITCH_Ab5    60
#define PITCH_A5     61
#define PITCH_Am5    62
#define PITCH_Bb5    62
#define PITCH_B5     63
#define PITCH_H5     63
#define PITCH_C6     64
#define PITCH_Cm6    65
#define PITCH_Db6    65
#define PITCH_D6     66
#define PITCH_Dm6    67
#define PITCH_Eb6    67
#define PITCH_E6     68
#define PITCH_F6     69
#define PITCH_Fm6    70
#define PITCH_Gb6    70
#define PITCH_G6     71
#define PITCH_Gm6    72
#define PITCH_Ab6    72
#define PITCH_A6     73
#define PITCH_Am6    74
#define PITCH_Bb6    74
#define PITCH_B6     75
#define PITCH_H6     75
#define PITCH_C7     76
#define PITCH_Cm7    77
#define PITCH_Db7    77
#define PITCH_D7     78
#define PITCH_Dm7    79
#define PITCH_Eb7    79
#define PITCH_E7     80
#define PITCH_F7     81
#define PITCH_Fm7    82
#define PITCH_Gb7    82
#define PITCH_G7     83
#define PITCH_Gm7    84
#define PITCH_Ab7    84
#define PITCH_A7     85
#define PITCH_Am7    86
#define PITCH_Bb7    86
#define PITCH_B7     87
#define PITCH_H7     87
#define PITCH_C8     88
#define PITCH_Cm8    89
#define PITCH_Db8    89
#define PITCH_D8     90
#define PITCH_Dm8    91
#define PITCH_Eb8    91
#define PITCH_E8     92
#define PITCH_F8     93
#define PITCH_Fm8    94
#define PITCH_Gb8    94
#define PITCH_G8     95
#define PITCH_Gm8    96
#define PITCH_A8     97

//@}

//! maximum pitch value
#define PITCH_MAX    98

//! repeat last {note->length} notes forever
#define PITCH_REPEAT     252

//! mark the end of a list of note_t entries
#define PITCH_END        255

//! set duration of 1/64th note
#define PITCH_TEMPO      254

//! set duration of internote spacing
#define PITCH_INTERNOTE  253

//! calculate duration based on beats per minute (bpm)
#define TEMPO_FROM_BPM(note_type,beats_per_minute)  (60000 / (note_type * beats_per_minute))

//! system sounds
#define DSOUND_BEEP        0

//! max system sound
#define DSOUND_SYS_MAX     1


//! default duration of 1/64th note in ms
#define DSOUND_DEFAULT_64th_ms  50

//! default duration internote spacing in ms
#define DSOUND_DEFAULT_internote_ms  15

///////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////

#ifndef DOXYGEN_SHOULD_SKIP_INTERNALS

extern unsigned dsound_64th_ms;                 //!< length of 1/16 note in ms
extern unsigned dsound_internote_ms;            //!< length of internote spacing in ms
const note_t volatile * dsound_next_note;       //!< pointer to current note
extern volatile unsigned dsound_next_time;      //!< when to play next note

extern const note_t *dsound_system_sounds[];    //!< system sound data

#endif // DOXYGEN_SHOULD_SKIP_INTERNALS


///////////////////////////////////////////////////////////////////////
//
// User functions
//
///////////////////////////////////////////////////////////////////////

//! play a sequence of notes
extern void dsound_play(const note_t *notes);

//! play a system sound
static inline void dsound_system(unsigned nr) {
    if(nr < DSOUND_SYS_MAX) {
        dsound_play(dsound_system_sounds[nr]);
    }
}

//! set duration of a 64th note in ms; return the previous duration.
/*! only for compatibility with brickOS */
#define dsound_set_duration(duration) dsound_set_tick((duration)>>2)

//! set duration of a 64th note in ms; return the previous duration.
/*! @deprecated use PITCH_TEMPO instead */
static inline unsigned dsound_set_tick(unsigned duration) {
    unsigned orig_duration = dsound_64th_ms;
    dsound_64th_ms=duration;
    return orig_duration;
}

//! set duration of inter-note spacing (subtracted from note duration)
/*! set to 0 for perfect legato.
 * @deprecated use PITCH_INTERNOTE instead
 */
static inline void dsound_set_internote(unsigned duration) {
    dsound_internote_ms=duration;
}

//! returns nonzero value if a sound is playing
static inline int dsound_playing(void) {
    return dsound_next_note!=0;
}

//! Wait until sound stops playing.
extern int dsound_wait(void);

//! stop playing sound
extern void dsound_stop(void);

#endif // CONF_DSOUND

#ifdef  __cplusplus
}
#endif

#endif  /* __dsound_h__ */
