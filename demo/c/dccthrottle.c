/******************************************************************************

  The contents of this file are subject to the Mozilla Public License
  Version 1.1 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
  the License for the specific language governing rights and limitations
  under the License. 

  The Original Code is DCC Library for the RCX.

  The Initial Developer of the Original Code is Mark Riley. Portions
  created by Mark Riley are Copyright (C) 2003 Mark Riley. All Rights
  Reserved.
  
  Updates by Matthew Sheets

     File : dccthrottle.c
  Created : 6/12/2003
  Purpose : DCC demonstration program for DCC library
  
  For more information on LDCC, please visit
  http://home.surewest.net/markril/lego/dcc/

******************************************************************************/

#include <config.h>

#if defined(CONF_DCC) && defined(CONF_DSENSOR_ROTATION)

#include <dcc.h>

#include <unistd.h>
#include <dmotor.h>
#include <dsensor.h>

#define DCC_MASK (MOTOR_A_MASK | MOTOR_B_MASK | MOTOR_C_MASK)

#define MAX_ANGLE 48

#define ADDR 3

// state of decoder functions FL through F4
unsigned char funcs = 0;

int main()
  {
  int angle = 0;
  char state = 0;
  int time = 0;

  ds_active(&SENSOR_3);
  ds_rotation_on(&SENSOR_3);
  ds_rotation_set(&SENSOR_3, 0);

  dcc_power_on(DCC_MASK);

  while (!shutdown_requested())
    {
    switch (state)
      {
      case 1:
        if (SENSOR_3 >= 0x4000)
          {  // button released, toggle headlight;
          funcs ^= DCC_FL;
          state = 0;
          }
        else
          {  // button still held down
          if (time - (int)get_system_up_time() <= 0)
            {  // 1/4 second elapsed, stop loco
            angle = 0;
            ds_rotation_set(&SENSOR_3, 0);
            state = 2;
            }
          }
        break;

      case 2:
        if (SENSOR_3 >= 0x4000)
          // button released, resume
          state = 0;
        break;

      default:
        if (SENSOR_3 >= 0x4000)
          {  // no button pressed, normal throttle operation
          angle = ROTATION_3;

          if (angle < -MAX_ANGLE)
            angle = -MAX_ANGLE;
          else if (angle > MAX_ANGLE)
            angle = MAX_ANGLE;
          }
        else
          {  // button pressed
          time = (int)get_system_up_time() + 250;  // 1/4 second
          state = 1;
          }
        break;
      }

    //dcc_speed14(ADDR, angle * 14 / MAX_ANGLE, funcs & DCC_FL);
    //dcc_speed28(ADDR, angle * 28 / MAX_ANGLE);
    dcc_speed126(ADDR, angle * 126 / MAX_ANGLE);

    dcc_idle();
    dcc_idle();

    dcc_FL_F4(ADDR, funcs);

    dcc_idle();
    dcc_idle();
    }

  dcc_power_off();

  ds_rotation_off(&SENSOR_3);
  ds_passive(&SENSOR_3);

  return 0;
  }
  
#else
#warning dccthrottle.c requires CONF_DCC and CONF_DSENSOR_ROTATION, at least one of which is not set

#include <conio.h>

int main(int argc, char *argv[]) {
	
#ifndef CONF_DCC
  cputc_native_user(CHAR_n, CHAR_o, CHAR_SPACE, CHAR_SPACE); // no
  sleep(1);
  cputc_native_user(CHAR_d, CHAR_c, CHAR_c, CHAR_SPACE);     // dcc
  sleep(1);
  cls();
  sleep(1);
#endif

#ifndef CONF_DSENSOR_ROTATION
  cputc_native_user(CHAR_n, CHAR_o, CHAR_SPACE, CHAR_SPACE); // no
  sleep(1);
  cputc_native_user(CHAR_r, CHAR_o, CHAR_t, CHAR_r);         // rotr
  sleep(1);
  cls();
  sleep(1);
#endif

  return 0;
}
#endif  //  defined(CONF_DCC) && defined(CONF_DSENSOR_ROTATION)

// EOF //
