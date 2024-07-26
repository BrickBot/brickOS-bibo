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

int main(int argc,char *argv[]) {

  // Play each of the system sounds
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

  return 0;
}
#else
#warning sound.c requires CONF_DSOUND which is not set
#warning sound demo will do nothing
int main(int argc, char *argv[]) {
  return 0;
}
#endif // CONF_DSOUND
