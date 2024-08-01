/******************************************************************************

  The contents of this file are subject to the Mozilla Public License
  Version 1.1 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
  the License for the specific language governing rights and limitations
  under the License. 

  The initial developer of this implementation is Matthew Sheets.
  All Rights Reserved. 

     File : sound.c
  Created : 7/26/2024
  Purpose : Demonstrate playing sounds
  
******************************************************************************/


#include <config.h>
#if defined(CONF_DSOUND)
#include <dsound.h>
#include <tm.h>

#ifdef CONF_CONIO
#include <dlcd.h>
#include <conio.h>
#endif

static note_t tuning[] = {
  { PITCH_TEMPO, TEMPO_FROM_BPM(QUARTER, 60) },
  { PITCH_INTERNOTE, 0 },

  { PITCH_Bb4, BREVE },

  { PITCH_END, 0 }
};

static note_t chromatic_scale[] = { 
  { PITCH_TEMPO, TEMPO_FROM_BPM(QUARTER, 120) },
  { PITCH_INTERNOTE, DSOUND_DEFAULT_internote_ms },

  { PITCH_A0, QUARTER },

  { PITCH_REPEAT, PITCH_MAX },  // Number of times to repeat
  { 0, 1 },  // Number of array elements to repeat,
             //   expressed as two unsigned byte values that will
             //   be read as a single, unsigned two-byte value

  { PITCH_END, 0 }
};

int main(int argc,char *argv[]) {

  // Play each of the system sounds
  // NOTE: these are manually defined and not the actual sounds in ROM
  //    c.f. https://mralligator.com/rcx/  -  ROM 327c, code 1772
  unsigned char i;
  for (i = 0; i < DSOUND_SYS_MAX; i++) {
#ifdef CONF_CONIO
    cputc_native_4(CHAR_n);
    cputc_native_3(CHAR_o);
    dlcd_show(LCD_3_DOT);
    cputc_native_2(CHAR_SPACE);
    cputc_hex_1(i);
#endif // CONF_CONIO

    dsound_system(i);
    dsound_wait();
    sleep(1);
    
#ifdef CONF_CONIO
    cls();
#endif // CONF_CONIO

    sleep(1);
  }

  //Play a tuning note
  dsound_play(tuning);
#ifdef CONF_CONIO
  cputc_native_user(CHAR_T, CHAR_U, CHAR_N, CHAR_E);
  for (i = 0; i <= 8 && dsound_playing(); i++) {
    cputc_hex_0(i);
    sleep(1);
  }
  cputc_native_0(CHAR_DASH);
#endif // CONF_CONIO
  dsound_wait();
  cls();
  sleep(1);

  // Play the full, available chromatic scale
  i = 1;

#ifdef CONF_CONIO
  // Display a music note on the LCD (quarter note)
  dlcd_show(LCD_4_DOT);
  dlcd_show(LCD_3_BOTL);

  // Update and show the note number
  cputc_hex_1(i);
#endif // CONF_CONIO

  int note_duration_ms = chromatic_scale[2].length * chromatic_scale[0].length;

  dsound_play(chromatic_scale);
  msleep(chromatic_scale[1].length);  // Internote duration

  for (i = 2; i <= PITCH_MAX; i++) {
    // Update the chromatic scale data structure on the fly
    // (NOTE: Really shouldn't attempt this for normal dsound_play() use)
    chromatic_scale[2].pitch = i;
    msleep(note_duration_ms);
#ifdef CONF_CONIO
    cputc_hex_1(i % 10);
    cputc_hex_2(i / 10);
#endif // CONF_CONIO
  }

  // End of loop; wait for playing to stop
#ifdef CONF_CONIO
  cputc_native_5(CHAR_DASH);
#endif // CONF_CONIO

  dsound_wait();

#ifdef CONF_CONIO
  cls();
#endif // CONF_CONIO

  sleep(1);

  return 0;
}
#else
#warning sound.c requires CONF_DSOUND which is not set
#warning sound demo will do nothing
int main(int argc, char *argv[]) {
  return 0;
}
#endif // CONF_DSOUND
