/******************************************************************************

  The contents of this file are subject to the Mozilla Public License
  Version 1.1 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
  the License for the specific language governing rights and limitations
  under the License. 

  The Initial Developer of the Original Code is Matthew Sheets.  All Rights
  Reserved. 

     File : music.c
  Created : 7/25/2024
  Purpose : Demonstrate playing music
  
******************************************************************************/


#include <config.h>
#if defined(CONF_DSOUND)
#include <dsound.h>
#include <tm.h>

#ifdef CONF_CONIO
#include <conio.h>
#endif

/* An array of notes that comprise the pentatonic tune of */
/*    the hymn "Amazing Grace" (tune name: "New Britain") */
static const note_t amazing_grace[] = { 
  { PITCH_TEMPO, TEMPO_FROM_BPM(QUARTER, 90) },
  { PITCH_INTERNOTE, DSOUND_DEFAULT_internote_ms },

  { PITCH_D3, QUARTER },
  { PITCH_G3, HALF },
  { PITCH_B3, EIGHTH },
  { PITCH_G3, EIGHTH },
  { PITCH_B3, HALF },
  { PITCH_A3, QUARTER },
  { PITCH_G3, HALF },
  { PITCH_E3, QUARTER },
  { PITCH_D3, HALF },

  { PITCH_D3, QUARTER },
  { PITCH_G3, HALF },
  { PITCH_B3, EIGHTH },
  { PITCH_G3, EIGHTH },
  { PITCH_B3, HALF },
  { PITCH_A3, QUARTER },
  { PITCH_D4, HALF_DOTTED + QUARTER }, // Tied note

  { PITCH_REST, QUARTER },

  { PITCH_B3, QUARTER },
  { PITCH_D4, QUARTER_DOTTED },
  { PITCH_B3, EIGHTH },
  { PITCH_D4, EIGHTH },
  { PITCH_B3, EIGHTH },
  { PITCH_G3, HALF },

  { PITCH_D3, QUARTER },
  { PITCH_E3, QUARTER_DOTTED },
  { PITCH_G3, EIGHTH },
  { PITCH_G3, EIGHTH },
  { PITCH_E3, EIGHTH },
  { PITCH_D3, HALF },

  { PITCH_D3, QUARTER },
  { PITCH_G3, HALF },
  { PITCH_B3, EIGHTH },
  { PITCH_G3, EIGHTH },
  { PITCH_B3, HALF },
  { PITCH_A3, QUARTER },
  { PITCH_G3, HALF_DOTTED },

  // Slowly repeat the last phrase
  { PITCH_TEMPO, TEMPO_FROM_BPM(QUARTER, 60) },
  { PITCH_REPEAT, 1 },  // Number of times to repeat
  { 0, 8 },  // Number of array elements to repeat,
             //   expressed as two unsigned byte values that will
             //   be read as a single, unsigned two-byte value

  { PITCH_END, 0 }
};

int main(int argc,char *argv[]) {
  int i;
  int intro_ms = 0;
  
  for(i = 2; i <=6; i++) {
    intro_ms += amazing_grace[i].length;
  }
  intro_ms = (intro_ms * amazing_grace[0].length);

  // Play the melody in a loop
  for(i = 0; !shutdown_requested(); i++) {
#ifdef CONF_CONIO    
    cputw(i);
#endif // CONF_CONIO

    dsound_play(amazing_grace);

    // Alternate whether to play the full song or just the intro
    if (i & 0x01) {
      dsound_wait();
      sleep(1);
    } else {
#ifdef CONF_CONIO
      cputc_native_5(CHAR_DASH);
#endif
      msleep(intro_ms);
      dsound_stop();
#ifdef CONF_CONIO
      cputc_native_5(0);
#endif
    }

    sleep(1);
  }

  return 0;
}
#else
#warning sound.c requires CONF_DSOUND which is not set
#warning sound demo will do nothing
int main(int argc, char *argv[]) {
  return 0;
}
#endif // CONF_DSOUND
