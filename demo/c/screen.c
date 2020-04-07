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

     File : dccdemo.c
  Created : 4/7/2020
  Purpose : Demonstration program for LCD screen
  
******************************************************************************/

#include <config.h>
#include <dlcd.h>
#include <mem.h>
#include <unistd.h>
#include <conio.h>

#define MSEC_PAUSE 750

typedef struct segment_t {
  unsigned int segment_loc;
  unsigned char segment_bit;
} segment_t;


static const segment_t segments[] = {
  // the man
//  { LCD_ARMS },
//  { LCD_BODY },
//  { LCD_1LEG },
//  { LCD_2LEGS },

  // digits
  { LCD_5_MID },
  { LCD_4_MID },
  { LCD_4_BOTR },
  { LCD_4_BOT },
  { LCD_4_BOTL },
  { LCD_4_TOPL },
  { LCD_4_TOP },
  { LCD_4_TOPR },
  { LCD_4_DOT },
  { LCD_3_MID },
  { LCD_3_BOTR },
  { LCD_3_BOT },
  { LCD_3_BOTL },
  { LCD_3_TOPL },
  { LCD_3_TOP },
  { LCD_3_TOPR },
  { LCD_3_DOT },
  { LCD_2_MID },
  { LCD_2_BOTR },
  { LCD_2_BOT },
  { LCD_2_BOTL },
  { LCD_2_TOPL },
  { LCD_2_TOP },
  { LCD_2_TOPR },
  { LCD_2_DOT },
  { LCD_1_MID },
  { LCD_1_BOTR },
  { LCD_1_BOT },
  { LCD_1_BOTL },
  { LCD_1_TOPL },
  { LCD_1_TOP },
  { LCD_1_TOPR },
  { LCD_0_MID },
  { LCD_0_BOTR },
  { LCD_0_BOT },
  { LCD_0_BOTL },
  { LCD_0_TOPL },
  { LCD_0_TOP },
  { LCD_0_TOPR },
  
  // the quartered circle
  { LCD_CIRCLE_2 },
  { LCD_CIRCLE_1 },
  { LCD_CIRCLE_0 },
  { LCD_CIRCLE_3 },
  
  // the motor displays
  { LCD_C_RIGHT },
  { LCD_C_SELECT },
  { LCD_C_LEFT },
  { LCD_B_RIGHT },
  { LCD_B_SELECT },
  { LCD_B_LEFT },
  { LCD_A_RIGHT },
  { LCD_A_SELECT },
  { LCD_A_LEFT },
  
  // the IR display
  { LCD_IR_LOWER },
  { LCD_IR_UPPER },
  
  // the first sensor displays
  { LCD_S1_SELECT },
  { LCD_S1_ACTIVE },
  
  // the dotted line
  { LCD_DOT_0 },
  { LCD_DOT_1 },
  { LCD_DOT_2 },
  { LCD_DOT_3 },
  { LCD_DOT_4 },
  
  // the second sensor displays
  { LCD_S2_SELECT },
  { LCD_S2_ACTIVE },
  
  // the crossed-out battery
  { LCD_BATTERY_X },

  // the third sensor displays
  { LCD_S3_SELECT },
  { LCD_S3_ACTIVE },
  
  // two empty bits
  //{ LCD_EMPTY_1 },
  //{ LCD_EMPTY_2 },

  // End
  { (unsigned int)NULL, (unsigned char)NULL }
};


void cycle_display(unsigned char do_set, unsigned char do_clear, unsigned int msec_pause) {
  // loop through displaying the segments
  unsigned char i = 0;
  for (i = 0; segments[i].segment_loc != (unsigned int)NULL; i++) {
    unsigned int segment_loc = segments[i].segment_loc;
    unsigned char segment_bit = segments[i].segment_bit;
    
    // The compiler complains of an "impossible constraintin 'asm'" if the
    //     bit argument is a variable and not a literal.
    switch (segment_bit) {
      case 0x0:
        if (do_set) { bit_set(segment_loc, 0x0); }
        if (msec_pause != 0) { msleep(msec_pause); }
        if (do_clear) { bit_clear(segment_loc, 0x0); }
        break;
      case 0x1:
        if (do_set) { bit_set(segment_loc, 0x1); }
        if (msec_pause != 0) { msleep(msec_pause); }
        if (do_clear) { bit_clear(segment_loc, 0x1); }
        break;
       case 0x2:
        if (do_set) { bit_set(segment_loc, 0x2); }
        if (msec_pause != 0) { msleep(msec_pause); }
        if (do_clear) { bit_clear(segment_loc, 0x2); }
        break;
      case 0x3:
        if (do_set) { bit_set(segment_loc, 0x3); }
        if (msec_pause != 0) { msleep(msec_pause); }
        if (do_clear) { bit_clear(segment_loc, 0x3); }
        break;
      case 0x4:
        if (do_set) { bit_set(segment_loc, 0x4); }
        if (msec_pause != 0) { msleep(msec_pause); }
        if (do_clear) { bit_clear(segment_loc, 0x4); }
        break;
      case 0x5:
        if (do_set) { bit_set(segment_loc, 0x5); }
        if (msec_pause != 0) { msleep(msec_pause); }
        if (do_clear) { bit_clear(segment_loc, 0x5); }
        break;
      case 0x6:
        if (do_set) { bit_set(segment_loc, 0x6); }
        if (msec_pause != 0) { msleep(msec_pause); }
        if (do_clear) { bit_clear(segment_loc, 0x6); }
        break;
      case 0x7:
        if (do_set) { bit_set(segment_loc, 0x7); }
        if (msec_pause != 0) { msleep(msec_pause); }
        if (do_clear) { bit_clear(segment_loc, 0x7); }
        break;
      default:
        if (do_set) {
          cputw(segment_loc);
          cputc_hex_0(segment_bit);
        }
        if (msec_pause != 0) { msleep(msec_pause); }
        if (do_clear) {
          cls();
          cputc_native_0(CHAR_SPACE);
        }
    }
  }
}


int main(int argc,char *argv[]) {
  unsigned int msec_pause = MSEC_PAUSE;
  
  cls();
  cputc_native_0(CHAR_SPACE);
  
  // Cycle through the LCD segments individually
  // Note that some segments actively controlled by the OS
  //   (e.g. battery level, sensor activity) might not display
  cycle_display(1, 1, msec_pause);
  
  msleep(msec_pause);
  
  // Show all the LCD segments concurrently
  cycle_display(1, 0, 0);
  msleep(msec_pause * 8);
  cycle_display(0, 1, 0);
  
  cls();

  return 0;
}
