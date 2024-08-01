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
 *                  Matthew Sheets
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

#include <dmusic.h>

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
/// Rests should be specified by placing \ref PITCH_REST in the {pitch} value.
/// The duration of the rest is placed in {length}.
typedef struct {
  unsigned char pitch;      //!< note pitch: 0 = rest, 1 ^= A_0 (~55 Hz)
  unsigned char length;     //!< note length in 1/64ths
} note_t;


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

const  volatile note_t * dsound_next_note;       //!< pointer to current note
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
static inline void dsound_system(unsigned char nr) {
    if(nr < DSOUND_SYS_MAX) {
        dsound_play(dsound_system_sounds[nr]);
    }
}

//! returns nonzero value if a sound is playing
static inline char dsound_playing(void) {
    return dsound_next_note!=0;
}

//! Wait until sound stops playing.
extern char dsound_wait(void);

//! stop playing sound
extern void dsound_stop(void);

#endif // CONF_DSOUND

#ifdef  __cplusplus
}
#endif

#endif  /* __dsound_h__ */
