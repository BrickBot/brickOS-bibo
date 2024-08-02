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

     File : keypad.c
  Created : 4/7/2020
  Purpose : Demonstrate keypad/button (plus some screen) functionality
  
******************************************************************************/

// NOTE: This program demonstrates usage of <dlcd.h>.
//    Please see <rom/lcd.h> for some alternate LCD segment capabilities.

#include <config.h>

#include <conio.h>
#include <unistd.h>
#include <dkey.h>
#include <sys/program.h>

#include <dlcd.h>

int main(int argc,char *argv[]) {
  unsigned char dkey_current;
  unsigned char count_onOff = 0, count_view = 0, count_prgm = 0, count_run = 0;

  // SETUP:  Enable use of both the "On-Off" and "Run" system keys
  allow_system_keys_access(KEY_ONOFF | KEY_RUN);

  // PART I:  Cycle through a pressing of each button //
  cputc_native_user(CHAR_P, CHAR_U, CHAR_S, CHAR_H); // PUSH
  sleep(1);
  lcd_clear();

  // On-Off button
  dlcd_show(LCD_4_BOT);
  dlcd_show(LCD_4_BOTL);
  dkey_wait(KEY_PRESSED,  KEY_ONOFF);
  dlcd_show(LCD_CIRCLE_2);
  dkey_wait(KEY_RELEASED, KEY_ONOFF);
  lcd_clear();

  // View button
  dlcd_show(LCD_3_TOP);
  dlcd_show(LCD_3_TOPL);
  dkey_wait(KEY_PRESSED,  KEY_VIEW);
  dlcd_show(LCD_CIRCLE_1);
  dkey_wait(KEY_RELEASED, KEY_VIEW);
  lcd_clear();

  // Program button
  dlcd_show(LCD_2_TOP);
  dlcd_show(LCD_2_TOPR);
  dkey_wait(KEY_PRESSED,  KEY_PRGM);
  dlcd_show(LCD_CIRCLE_0);
  dkey_wait(KEY_RELEASED, KEY_PRGM);
  lcd_clear();

  // Run button
  dlcd_show(LCD_1_BOT);
  dlcd_show(LCD_1_BOTR);
  dkey_wait(KEY_PRESSED,  KEY_RUN);
  dlcd_show(LCD_CIRCLE_3);
  dkey_wait(KEY_RELEASED, KEY_RUN);
  lcd_clear();

  sleep(1);


  // PART 2: Count button presses until one button is pressed 0xf+1 times

  cls();  // Calling cls will also reset the program number display
  dlcd_show(LCD_4_BOT);
  dlcd_show(LCD_4_BOTL);
  dlcd_show(LCD_3_TOP);
  dlcd_show(LCD_3_TOPL);
  dlcd_show(LCD_2_TOP);
  dlcd_show(LCD_2_TOPR);
  dlcd_show(LCD_1_BOT);
  dlcd_show(LCD_1_BOTR);

  do {
    // dkey_current = dkey;
    dkey_current = getchar();
    cputc_native_5(CHAR_SPACE);
    
    switch (dkey_current) {
      case KEY_ONOFF:
        cputc_hex_4(++count_onOff);
        break;
      case KEY_VIEW:
        cputc_hex_3(++count_view);
        break;
      case KEY_PRGM:
        cputc_hex_2(++count_prgm);
        break;
      case KEY_RUN:
        cputc_hex_1(++count_run);
        break;
      default:
        cputc_native_5(CHAR_DASH);
        break;
    }
  } while (!shutdown_requested()
      && count_onOff <= 0xf && count_view <=0xf && count_prgm <= 0xf && count_run <= 0xf);

  sleep(1);


  // CLEANUP:  Return control of both the "On-Off" and "Run" system keys to the system
  allow_system_keys_access(KEY_NONE);

  cputc_native_user(CHAR_B, CHAR_Y, CHAR_E, CHAR_SPACE); // BYE!
  dlcd_show(LCD_2_DOT);
  dlcd_show(LCD_1_BOTL);
  dlcd_show(LCD_1_TOPL);
  sleep(6);

  lcd_clear();

  return 0;
}
