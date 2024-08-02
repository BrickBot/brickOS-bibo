/******************************************************************************

  The contents of this file are subject to the Mozilla Public License
  Version 1.1 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
  the License for the specific language governing rights and limitations
  under the License. 

  The Original Code is a program to show primitive type sizes on the RCX.

  The Initial Developer of the Original Code is Matthew Sheets. All Rights
  Reserved.
  
     File : primatives.c
  Created : 8/1/2024
  Purpose : Check the size of primitve types used by the compiler
            Press the "View" button to cycle through the primitive types
  
******************************************************************************/

#include <config.h>
#include <unistd.h>
#include <conio.h>
#include <dlcd.h>
#include <dkey.h>

void show_size(unsigned char type_char_1, unsigned char type_char_2,
               unsigned char type_char_3, unsigned char type_size) {
  cputc_native_4(type_char_1);
  cputc_native_3(type_char_2);
  cputc_native_2(type_char_3);
  cputc_hex_1(type_size);
  
  dkey_wait(KEY_PRESSED,  KEY_VIEW);
  dlcd_show(LCD_CIRCLE_0);
  dlcd_show(LCD_CIRCLE_1);
  dlcd_show(LCD_CIRCLE_2);
  dlcd_show(LCD_CIRCLE_3);
  dkey_wait(KEY_RELEASED, KEY_VIEW);
  dlcd_hide(LCD_CIRCLE_0);
  dlcd_hide(LCD_CIRCLE_1);
  dlcd_hide(LCD_CIRCLE_2);
  dlcd_hide(LCD_CIRCLE_3);
  cls();
}

int main(int argc, char **argv) {
  
  // Pointer
  show_size(0, CHAR_P, 0, sizeof(void*));
  
  // Unsigned
  show_size(CHAR_U, 0, 0, sizeof(unsigned));
  
  // Char
  show_size(0, CHAR_C, 0, sizeof(char));
  
  // Unsigned Char
  show_size(CHAR_U, CHAR_C, 0, sizeof(unsigned char));
  
  // Short
  show_size(0, CHAR_S, 0, sizeof(short));
  
  // Unsigned Short
  show_size(CHAR_U, CHAR_S, 0, sizeof(unsigned short));
  
  // Int
  show_size(0, CHAR_I, 0, sizeof(int));
  
  // Unsigned Int
  show_size(CHAR_U, CHAR_I, 0, sizeof(unsigned int));
  
  // Long
  show_size(0, CHAR_L, 0, sizeof(long));
  
  // Unsigned Long
  show_size(CHAR_U, CHAR_L, 0, sizeof(unsigned long));
  
  // Long Long
  show_size(0, CHAR_L, CHAR_L, sizeof(long long));
  
  // Unsigned Long Long
  show_size(CHAR_U, CHAR_L, CHAR_L, sizeof(unsigned long long));
  
  // Float
  show_size(0, CHAR_F, 0, sizeof(float));
  
  // Double
  show_size(0, CHAR_D, 0, sizeof(double));
  
  // Long Double
  show_size(CHAR_L, CHAR_D, 0, sizeof(long double));
  
  // Display a brief "done" message
  cputc_native_user(CHAR_d, CHAR_O, CHAR_N, CHAR_E); // DONE
  sleep(1);
  
  return 0;
}
