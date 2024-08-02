/******************************************************************************

  The contents of this file are subject to the Mozilla Public License
  Version 1.1 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
  the License for the specific language governing rights and limitations
  under the License. 

  The Original Code is a "Hello, World!" program in C for the RCX.

  Implemented by Matthew Sheets. All Rights Reserved.
  
     File : hello-C.c
  Purpose : Basic "Hello, World!" program for the RCX in C
  

******************************************************************************/

#include <config.h>
#include <conio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  // Display a friendly message on the RCX screen
  cputc_native_user(CHAR_H, CHAR_E, CHAR_PARALLEL, CHAR_O); // HELLO
  sleep(1);
  cputc_native_user(CHAR_L, CHAR_E, CHAR_G, CHAR_O);        // LEGO
  sleep(1);
  cls();
  sleep(1);
  
  // Use different methods and display the same message upside down
  cputc_native_user(CHAR_F, CHAR_L, CHAR_I, CHAR_P);
  cputc_native_5(CHAR_DASH);
  sleep(1);
  cls();
  sleep(1);
  
  cputc_hex_4(0x00);
  cputc_native_3(CHAR_PARALLEL);
  cputc_hex(0x03, 2);
  cputc_hex(0x04, 1);
  sleep(1);
  cputw(0x0637);    // LEGO
  sleep(1);
  cls();
  sleep(1);
  
  return 0;
}
