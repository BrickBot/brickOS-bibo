/*! \file   include/dmusic.h
    \brief  Interface: playing sound musically
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
 *                  Matthew Sheets
 */

#ifndef __dmusic_h__
#define __dmusic_h__

#ifdef  __cplusplus
extern "C" {
#endif


///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////


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

//! \defgroup notePitches standard note pitch index values
//@{

//! specify a pause (rest)
#define PITCH_PAUSE   0
#define PITCH_REST    0

/// If CONF_EXTENDED_MUSIC is defined, then use note indexes and the pitch2freq array.
///   - This enables dynamic transposition, but unless music of more than ~100 pitch
///       elements is used in a program, then this ends up consuming MORE space.
/// If CONF_EXTENDED_MUSIC is NOT defined, then use the frequencies directly.
///   - This does not readily facilitate dynamic transposition, and as long as music
///       of less than ~100 pitch elements is used in a program, then this consumes LESS space.
#ifdef CONF_EXTENDED_MUSIC

/// Note Pitch Index Value 
#define PITCH_A0      1
#define PITCH_Am0     2
#define PITCH_Bb0     PITCH_Am0
#define PITCH_Hb0     PITCH_Am0
#define PITCH_B0      3
#define PITCH_H0      PITCH_B0
#define PITCH_C1      4
#define PITCH_Cm1     5
#define PITCH_Db1     PITCH_Cm1
#define PITCH_D1      6
#define PITCH_Dm1     7
#define PITCH_Eb1     PITCH_Dm1
#define PITCH_E1      8
#define PITCH_F1      9
#define PITCH_Fm1    10
#define PITCH_Gb1    PITCH_Fm1
#define PITCH_G1     11
#define PITCH_Gm1    12
#define PITCH_Ab1    PITCH_Gm1
#define PITCH_A1     13
#define PITCH_Am1    14
#define PITCH_Bb1    PITCH_Am1
#define PITCH_Hb1    PITCH_Am1
#define PITCH_B1     15
#define PITCH_H1     PITCH_B1
#define PITCH_C2     16
#define PITCH_Cm2    17
#define PITCH_Db2    PITCH_Cm2
#define PITCH_D2     18
#define PITCH_Dm2    19
#define PITCH_Eb2    PITCH_Dm2
#define PITCH_E2     20
#define PITCH_F2     21
#define PITCH_Fm2    22
#define PITCH_Gb2    PITCH_Fm2
#define PITCH_G2     23
#define PITCH_Gm2    24
#define PITCH_Ab2    PITCH_Gm2
#define PITCH_A2     25
#define PITCH_Am2    26
#define PITCH_Bb2    PITCH_Am2
#define PITCH_Hb2    PITCH_Am2
#define PITCH_B2     27
#define PITCH_H2     PITCH_B2
#define PITCH_C3     28
#define PITCH_Cm3    29
#define PITCH_Dd3    PITCH_Cm3
#define PITCH_D3     30
#define PITCH_Dm3    31
#define PITCH_Eb3    PITCH_Dm3
#define PITCH_E3     32
#define PITCH_F3     33
#define PITCH_Fm3    34
#define PITCH_Gb3    PITCH_Fm3
#define PITCH_G3     35
#define PITCH_Gm3    36
#define PITCH_Ab3    PITCH_Gm3
#define PITCH_A3     37
#define PITCH_Am3    38
#define PITCH_Bb3    PITCH_Am3
#define PITCH_Hb3    PITCH_Am3
#define PITCH_B3     39
#define PITCH_H3     PITCH_B3
#define PITCH_C4     40
#define PITCH_Cm4    41
#define PITCH_Db4    PITCH_Cm4
#define PITCH_D4     42
#define PITCH_Dm4    43
#define PITCH_Eb4    PITCH_Dm4
#define PITCH_E4     44
#define PITCH_F4     45
#define PITCH_Fm4    46
#define PITCH_Gb4    PITCH_Fm4
#define PITCH_G4     47
#define PITCH_Gm4    48
#define PITCH_Ab4    PITCH_Gm4
#define PITCH_A4     49
#define PITCH_Am4    50
#define PITCH_Bb4    PITCH_Am4
#define PITCH_Hb4    PITCH_Am4
#define PITCH_B4     51
#define PITCH_H4     PITCH_B4
#define PITCH_C5     52
#define PITCH_Cm5    53
#define PITCH_Db5    PITCH_Cm5
#define PITCH_D5     54
#define PITCH_Dm5    55
#define PITCH_Eb5    PITCH_Dm5
#define PITCH_E5     56
#define PITCH_F5     57
#define PITCH_Fm5    58
#define PITCH_Gb5    PITCH_Fm5
#define PITCH_G5     59
#define PITCH_Gm5    60
#define PITCH_Ab5    PITCH_Gm5
#define PITCH_A5     61
#define PITCH_Am5    62
#define PITCH_Bb5    PITCH_Am5
#define PITCH_Hb5    PITCH_Am5
#define PITCH_B5     63
#define PITCH_H5     PITCH_B5
#define PITCH_C6     64
#define PITCH_Cm6    65
#define PITCH_Db6    PITCH_Cm6
#define PITCH_D6     66
#define PITCH_Dm6    67
#define PITCH_Eb6    PITCH_Dm6
#define PITCH_E6     68
#define PITCH_F6     69
#define PITCH_Fm6    70
#define PITCH_Gb6    PITCH_Fm6
#define PITCH_G6     71
#define PITCH_Gm6    72
#define PITCH_Ab6    PITCH_Gm6
#define PITCH_A6     73
#define PITCH_Am6    74
#define PITCH_Bb6    PITCH_Am6
#define PITCH_Hb6    PITCH_Am6
#define PITCH_B6     75
#define PITCH_H6     PITCH_B6
#define PITCH_C7     76
#define PITCH_Cm7    77
#define PITCH_Db7    PITCH_Cm7
#define PITCH_D7     78
#define PITCH_Dm7    79
#define PITCH_Eb7    PITCH_Dm7
#define PITCH_E7     80
#define PITCH_F7     81
#define PITCH_Fm7    82
#define PITCH_Gb7    PITCH_Fm7
#define PITCH_G7     83
#define PITCH_Gm7    84
#define PITCH_Ab7    PITCH_Gm7
#define PITCH_A7     85
#define PITCH_Am7    86
#define PITCH_Bb7    PITCH_Am7
#define PITCH_Hb7    PITCH_Am7
#define PITCH_B7     87
#define PITCH_H7     PITCH_B7
#define PITCH_C8     88
#define PITCH_Cm8    89
#define PITCH_Db8    PITCH_Cm8
#define PITCH_D8     90
#define PITCH_Dm8    91
#define PITCH_Eb8    PITCH_Dm8
#define PITCH_E8     92
#define PITCH_F8     93
#define PITCH_Fm8    94
#define PITCH_Gb8    PITCH_Fm8
#define PITCH_G8     95
#define PITCH_Gm8    96
#define PITCH_Ab8    PITCH_Gm8
#define PITCH_A8     97

#else

/// Note Pitch Frequency Value 
#define PITCH_A0     FREQ_A0 
#define PITCH_Am0    FREQ_Am0
#define PITCH_Bb0    FREQ_Bb0
#define PITCH_Hb0    FREQ_Hb0
#define PITCH_B0     FREQ_B0 
#define PITCH_H0     FREQ_H0 
#define PITCH_C1     FREQ_C1 
#define PITCH_Cm1    FREQ_Cm1
#define PITCH_Db1    FREQ_Db1
#define PITCH_D1     FREQ_D1 
#define PITCH_Dm1    FREQ_Dm1
#define PITCH_Eb1    FREQ_Eb1
#define PITCH_E1     FREQ_E1 
#define PITCH_F1     FREQ_F1 
#define PITCH_Fm1    FREQ_Fm1
#define PITCH_Gb1    FREQ_Gb1
#define PITCH_G1     FREQ_G1 
#define PITCH_Gm1    FREQ_Gm1
#define PITCH_Ab1    FREQ_Ab1
#define PITCH_A1     FREQ_A1 
#define PITCH_Am1    FREQ_Am1
#define PITCH_Bb1    FREQ_Bb1
#define PITCH_Hb1    FREQ_Hb1
#define PITCH_B1     FREQ_B1 
#define PITCH_H1     FREQ_H1 
#define PITCH_C2     FREQ_C2 
#define PITCH_Cm2    FREQ_Cm2
#define PITCH_Db2    FREQ_Db2
#define PITCH_D2     FREQ_D2 
#define PITCH_Dm2    FREQ_Dm2
#define PITCH_Eb2    FREQ_Eb2
#define PITCH_E2     FREQ_E2 
#define PITCH_F2     FREQ_F2 
#define PITCH_Fm2    FREQ_Fm2
#define PITCH_Gb2    FREQ_Gb2
#define PITCH_G2     FREQ_G2 
#define PITCH_Gm2    FREQ_Gm2
#define PITCH_Ab2    FREQ_Ab2
#define PITCH_A2     FREQ_A2 
#define PITCH_Am2    FREQ_Am2
#define PITCH_Bb2    FREQ_Bb2
#define PITCH_Hb2    FREQ_Hb2
#define PITCH_B2     FREQ_B2 
#define PITCH_H2     FREQ_H2 
#define PITCH_C3     FREQ_C3 
#define PITCH_Cm3    FREQ_Cm3
#define PITCH_Dd3    FREQ_Dd3
#define PITCH_D3     FREQ_D3 
#define PITCH_Dm3    FREQ_Dm3
#define PITCH_Eb3    FREQ_Eb3
#define PITCH_E3     FREQ_E3 
#define PITCH_F3     FREQ_F3 
#define PITCH_Fm3    FREQ_Fm3
#define PITCH_Gb3    FREQ_Gb3
#define PITCH_G3     FREQ_G3 
#define PITCH_Gm3    FREQ_Gm3
#define PITCH_Ab3    FREQ_Ab3
#define PITCH_A3     FREQ_A3 
#define PITCH_Am3    FREQ_Am3
#define PITCH_Bb3    FREQ_Bb3
#define PITCH_Hb3    FREQ_Hb3
#define PITCH_B3     FREQ_B3 
#define PITCH_H3     FREQ_H3 
#define PITCH_C4     FREQ_C4 
#define PITCH_Cm4    FREQ_Cm4
#define PITCH_Db4    FREQ_Db4
#define PITCH_D4     FREQ_D4 
#define PITCH_Dm4    FREQ_Dm4
#define PITCH_Eb4    FREQ_Eb4
#define PITCH_E4     FREQ_E4 
#define PITCH_F4     FREQ_F4 
#define PITCH_Fm4    FREQ_Fm4
#define PITCH_Gb4    FREQ_Gb4
#define PITCH_G4     FREQ_G4 
#define PITCH_Gm4    FREQ_Gm4
#define PITCH_Ab4    FREQ_Ab4
#define PITCH_A4     FREQ_A4 
#define PITCH_Am4    FREQ_Am4
#define PITCH_Bb4    FREQ_Bb4
#define PITCH_Hb4    FREQ_Hb4
#define PITCH_B4     FREQ_B4 
#define PITCH_H4     FREQ_H4 
#define PITCH_C5     FREQ_C5 
#define PITCH_Cm5    FREQ_Cm5
#define PITCH_Db5    FREQ_Db5
#define PITCH_D5     FREQ_D5 
#define PITCH_Dm5    FREQ_Dm5
#define PITCH_Eb5    FREQ_Eb5
#define PITCH_E5     FREQ_E5 
#define PITCH_F5     FREQ_F5 
#define PITCH_Fm5    FREQ_Fm5
#define PITCH_Gb5    FREQ_Gb5
#define PITCH_G5     FREQ_G5 
#define PITCH_Gm5    FREQ_Gm5
#define PITCH_Ab5    FREQ_Ab5
#define PITCH_A5     FREQ_A5 
#define PITCH_Am5    FREQ_Am5
#define PITCH_Bb5    FREQ_Bb5
#define PITCH_Hb5    FREQ_Hb5
#define PITCH_B5     FREQ_B5 
#define PITCH_H5     FREQ_H5 
#define PITCH_C6     FREQ_C6 
#define PITCH_Cm6    FREQ_Cm6
#define PITCH_Db6    FREQ_Db6
#define PITCH_D6     FREQ_D6 
#define PITCH_Dm6    FREQ_Dm6
#define PITCH_Eb6    FREQ_Eb6
#define PITCH_E6     FREQ_E6 
#define PITCH_F6     FREQ_F6 
#define PITCH_Fm6    FREQ_Fm6
#define PITCH_Gb6    FREQ_Gb6
#define PITCH_G6     FREQ_G6 
#define PITCH_Gm6    FREQ_Gm6
#define PITCH_Ab6    FREQ_Ab6
#define PITCH_A6     FREQ_A6 
#define PITCH_Am6    FREQ_Am6
#define PITCH_Bb6    FREQ_Bb6
#define PITCH_Hb6    FREQ_Hb6
#define PITCH_B6     FREQ_B6 
#define PITCH_H6     FREQ_H6 
#define PITCH_C7     FREQ_C7 
#define PITCH_Cm7    FREQ_Cm7
#define PITCH_Db7    FREQ_Db7
#define PITCH_D7     FREQ_D7 
#define PITCH_Dm7    FREQ_Dm7
#define PITCH_Eb7    FREQ_Eb7
#define PITCH_E7     FREQ_E7 
#define PITCH_F7     FREQ_F7 
#define PITCH_Fm7    FREQ_Fm7
#define PITCH_Gb7    FREQ_Gb7
#define PITCH_G7     FREQ_G7 
#define PITCH_Gm7    FREQ_Gm7
#define PITCH_Ab7    FREQ_Ab7
#define PITCH_A7     FREQ_A7 
#define PITCH_Am7    FREQ_Am7
#define PITCH_Bb7    FREQ_Bb7
#define PITCH_Hb7    FREQ_Hb7
#define PITCH_B7     FREQ_B7 
#define PITCH_H7     FREQ_H7 
#define PITCH_C8     FREQ_C8 
#define PITCH_Cm8    FREQ_Cm8
#define PITCH_Db8    FREQ_Db8
#define PITCH_D8     FREQ_D8 
#define PITCH_Dm8    FREQ_Dm8
#define PITCH_Eb8    FREQ_Eb8
#define PITCH_E8     FREQ_E8 
#define PITCH_F8     FREQ_F8 
#define PITCH_Fm8    FREQ_Fm8
#define PITCH_Gb8    FREQ_Gb8
#define PITCH_G8     FREQ_G8 
#define PITCH_Gm8    FREQ_Gm8
#define PITCH_Ab8    FREQ_Ab8
#define PITCH_A8     FREQ_A8 

#endif

//@}

//! maximum pitch value
#define PITCH_MAX    97

//! repeat last {note->length} times {number} notes
//! PITCH_REPEAT is a two-element sequence:
//!   The length value of  the first element is the number of times to repeat
//!   The second element is the number of array elements to repeat, expressed as
//!     two unsigned byte values that will be read as a single, unsigned two-byte value
//! NOTE: Repeats cannot be nested
#define PITCH_REPEAT     252

//! set duration of internote spacing
#define PITCH_INTERNOTE  253

//! set duration of 1/64th note
#define PITCH_TEMPO      254

//! mark the end of a list of note_t entries
#define PITCH_END        255

//! calculate duration for PITCH_TEMPO based on beats per minute (bpm)
#define TEMPO_FROM_BPM(note_type,beats_per_minute)  (60000 / (note_type * beats_per_minute))

//! \defgroup notePitches standard note frequency values
//@{

/// Note Pitch Frequency Value 
#define FREQ_A0     0x8d03
#define FREQ_Am0    0x8603
#define FREQ_Bb0    FREQ_Am0
#define FREQ_Hb0    FREQ_Am0
#define FREQ_B0     0x7d03
#define FREQ_H0     FREQ_B0
#define FREQ_C1     0x7703
#define FREQ_Cm1    0x7003
#define FREQ_Db1    FREQ_Cm1
#define FREQ_D1     0x6a03
#define FREQ_Dm1    0x6303
#define FREQ_Eb1    FREQ_Dm1
#define FREQ_E1     0x5e03
#define FREQ_F1     0x5903
#define FREQ_Fm1    0x5403
#define FREQ_Gb1    FREQ_Fm1
#define FREQ_G1     0x4f03
#define FREQ_Gm1    0x4a03
#define FREQ_Ab1    FREQ_Gm1
#define FREQ_A1     0x4603
#define FREQ_Am1    0x4203
#define FREQ_Bb1    FREQ_Am1
#define FREQ_Hb1    FREQ_Am1
#define FREQ_B1     0xfd83
#define FREQ_H1     FREQ_B1
#define FREQ_C2     0xee83
#define FREQ_Cm2    0xe083
#define FREQ_Db2    FREQ_Cm2
#define FREQ_D2     0xd483
#define FREQ_Dm2    0xc783
#define FREQ_Eb2    FREQ_Dm2
#define FREQ_E2     0xbc83
#define FREQ_F2     0xb283
#define FREQ_Fm2    0xa883
#define FREQ_Gb2    FREQ_Fm2
#define FREQ_G2     0x9e83
#define FREQ_Gm2    0x9583
#define FREQ_Ab2    FREQ_Gm2
#define FREQ_A2     0x8d83
#define FREQ_Am2    0x8583
#define FREQ_Bb2    FREQ_Am2
#define FREQ_Hb2    FREQ_Am2
#define FREQ_B2     0x7e83
#define FREQ_H2     FREQ_B2
#define FREQ_C3     0x7683
#define FREQ_Cm3    0x7083
#define FREQ_Dd3    FREQ_Cm3
#define FREQ_D3     0x6983
#define FREQ_Dm3    0x6383
#define FREQ_Eb3    FREQ_Dm3
#define FREQ_E3     0x5e83
#define FREQ_F3     0x5983
#define FREQ_Fm3    0x5383
#define FREQ_Gb3    FREQ_Fm3
#define FREQ_G3     0x4f83
#define FREQ_Gm3    0x4a83
#define FREQ_Ab3    FREQ_Gm3
#define FREQ_A3     0x4683
#define FREQ_Am3    0x4283
#define FREQ_Bb3    FREQ_Am3
#define FREQ_Hb3    FREQ_Am3
#define FREQ_B3     0xfc02
#define FREQ_H3     FREQ_B3
#define FREQ_C4     0xee02
#define FREQ_Cm4    0xe102
#define FREQ_Db4    FREQ_Cm4
#define FREQ_D4     0xd402
#define FREQ_Dm4    0xc802
#define FREQ_Eb4    FREQ_Dm4
#define FREQ_E4     0xbd02
#define FREQ_F4     0xb202
#define FREQ_Fm4    0xa802
#define FREQ_Gb4    FREQ_Fm4
#define FREQ_G4     0x9e02
#define FREQ_Gm4    0x9502
#define FREQ_Ab4    FREQ_Gm4
#define FREQ_A4     0x8d02
#define FREQ_Am4    0x8502
#define FREQ_Bb4    FREQ_Am4
#define FREQ_Hb4    FREQ_Am4
#define FREQ_B4     0xfc82
#define FREQ_H4     FREQ_B4
#define FREQ_C5     0xee82
#define FREQ_Cm5    0xe082
#define FREQ_Db5    FREQ_Cm5
#define FREQ_D5     0xd482
#define FREQ_Dm5    0xc882
#define FREQ_Eb5    FREQ_Dm5
#define FREQ_E5     0xbd82
#define FREQ_F5     0xb282
#define FREQ_Fm5    0xa882
#define FREQ_Gb5    FREQ_Fm5
#define FREQ_G5     0x9e82
#define FREQ_Gm5    0x9682
#define FREQ_Ab5    FREQ_Gm5
#define FREQ_A5     0x8d82
#define FREQ_Am5    0x8582
#define FREQ_Bb5    FREQ_Am5
#define FREQ_Hb5    FREQ_Am5
#define FREQ_B5     0x7e82
#define FREQ_H5     FREQ_B5
#define FREQ_C6     0x7682
#define FREQ_Cm6    0x7082
#define FREQ_Db6    FREQ_Cm6
#define FREQ_D6     0x6982
#define FREQ_Dm6    0x6382
#define FREQ_Eb6    FREQ_Dm6
#define FREQ_E6     0x5e82
#define FREQ_F6     0x5882
#define FREQ_Fm6    0x5382
#define FREQ_Gb6    FREQ_Fm6
#define FREQ_G6     0x4f82
#define FREQ_Gm6    0x4a82
#define FREQ_Ab6    FREQ_Gm6
#define FREQ_A6     0x4682
#define FREQ_Am6    0x4282
#define FREQ_Bb6    FREQ_Am6
#define FREQ_Hb6    FREQ_Am6
#define FREQ_B6     0xfc01
#define FREQ_H6     FREQ_B6
#define FREQ_C7     0xee01
#define FREQ_Cm7    0xe001
#define FREQ_Db7    FREQ_Cm7
#define FREQ_D7     0xd401
#define FREQ_Dm7    0xc801
#define FREQ_Eb7    FREQ_Dm7
#define FREQ_E7     0xbd01
#define FREQ_F7     0xb201
#define FREQ_Fm7    0xa801
#define FREQ_Gb7    FREQ_Fm7
#define FREQ_G7     0x9e01
#define FREQ_Gm7    0x9501
#define FREQ_Ab7    FREQ_Gm7
#define FREQ_A7     0x8d01
#define FREQ_Am7    0x8501
#define FREQ_Bb7    FREQ_Am7
#define FREQ_Hb7    FREQ_Am7
#define FREQ_B7     0x7e01
#define FREQ_H7     FREQ_B7
#define FREQ_C8     0x7601
#define FREQ_Cm8    0x7001
#define FREQ_Db8    FREQ_Cm8
#define FREQ_D8     0x6901
#define FREQ_Dm8    0x6301
#define FREQ_Eb8    FREQ_Dm8
#define FREQ_E8     0x5e01
#define FREQ_F8     0x5801
#define FREQ_Fm8    0x5301
#define FREQ_Gb8    FREQ_Fm8
#define FREQ_G8     0x4f01
#define FREQ_Gm8    0x4a01
#define FREQ_Ab8    FREQ_Gm8
#define FREQ_A8     0x4601

//@}


#ifdef  __cplusplus
}
#endif

#endif  /* __dmusic_h__ */
