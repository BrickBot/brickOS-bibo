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
  Purpose : Cycle through kernel properties by pressing the "View" button
  
******************************************************************************/

// NOTE: This program demonstrates usage of <dlcd.h>.
//    Please see <rom/lcd.h> for some alternate LCD segment capabilities.

#include <config.h>

#include <conio.h>
#include <unistd.h>
#include <sys/tm.h>
#include <dkey.h>

#include <dlcd.h>

int main(int argc,char *argv[]) {
  unsigned char dkey_snapshot;

  // Until the "View" button is pressed, show when the "Program" button is pressed
  do {
    dkey_snapshot = dkey;
    
    cputc_native_4(dkey_snapshot & KEY_ONOFF ? CHAR_O : CHAR_SPACE);
    cputc_native_3(dkey_snapshot & KEY_VIEW  ? CHAR_V : CHAR_SPACE);
    cputc_native_2(dkey_snapshot & KEY_PRGM  ? CHAR_P : CHAR_SPACE);
    cputc_native_1(dkey_snapshot & KEY_RUN   ? CHAR_R : CHAR_SPACE);
    
    msleep(100);
  } while (!shutdown_requested() && dkey_snapshot != KEY_VIEW);

//  lcd_clear();
  
  cputc_native_5(CHAR_DASH);
  cputc_native_user(CHAR_P, CHAR_U, CHAR_S, CHAR_H); // KEYS
  sleep(1);
  
//  unsigned char input = getchar();

  // Cycle through various kernel properties by
  //    pressing the "View" button
  
  char cached_kl = kernel_lock;

  cputw(cached_kl);
  dkey_wait(KEY_PRESSED, KEY_VIEW);
//  dlcd_cycle_circle_clockwise();

  cached_kl = kernel_lock;
  cputw(cached_kl);
  cputc_native_5(CHAR_DASH);

  dkey_wait(KEY_RELEASED, KEY_VIEW);
//  dlcd_cycle_circle_clockwise();
//  yield();  yield();  yield();  yield();  yield();

  cached_kl = kernel_lock;
  cputw(cached_kl);
  cputc_native_5(0);

  dkey_wait(KEY_PRESSED, KEY_PRGM);
//  dlcd_cycle_circle_clockwise();

  cached_kl = kernel_lock;
  cputw(cached_kl);
  cputc_native_5(CHAR_DASH);

  dkey_wait(KEY_RELEASED, KEY_PRGM);
  cputc_native_5(0);
//  dlcd_cycle_circle_clockwise();

  cached_kl = kernel_lock;
  cputw(cached_kl);

  sleep(4);  

  return 0;
}
