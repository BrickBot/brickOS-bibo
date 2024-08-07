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

     File : motor.c
  Created : 8/2/2024
  Purpose : Demonstrate motor functionality
  
******************************************************************************/

// NOTE: This program demonstrates usage of <dmotor.h>.

#include <config.h>

#ifdef CONF_DMOTOR

#include <conio.h>
#include <unistd.h>
#include <dkey.h>
#include <tm.h>
#include <sys/program.h>

#include <dmotor.h>
#include <dlcd.h>

const unsigned char step = 17;

void update_lcd(Motor motor) {
  unsigned char speed = motor_speed_get(motor);
  unsigned char speed_step = speed / step;

  // Display the motor number in the usual program spot
  cputc_hex_0(0x0a + (unsigned char)motor);
  
  // Indicate the currently-selected motor with the motor selection arrow
  unsigned char i;
  for (i = (unsigned char)motor_min; i <= (unsigned char)motor_max; i++) {
    if (i == (unsigned char)motor) {
      motor_select_show((Motor)i);
    } else {
      motor_select_hide((Motor)i);
    }
  }
  
  // Clear the dash (if set)
  cputc_native_5(0);
  
  switch (motor_dir_get(motor)) {
    case off:
      cputc_native_user(CHAR_O, CHAR_F, CHAR_F, 0);  // Off
      dlcd_show(LCD_2_DOT);
      cputc_hex_1(speed_step);  // Though off, show the current speed step
      break;
    case brake:
      cputc_native_user(CHAR_H, CHAR_l, CHAR_d, 0);  // Hold
      dlcd_show(LCD_2_DOT);
      cputc_hex_1(speed_step);  // Though off, show the current speed step
      break;
    case rev:
      lcd_unsigned(motor_speed_get(motor));
      // The value is not really negative, but more visually differentiate from forward
      cputc_native_5(CHAR_DASH);
      break;
    case fwd:
      lcd_unsigned(motor_speed_get(motor));
      break;
    default:
      cputw(motor_dir_get(motor));
      break;
  }
}

Motor select_next_motor(Motor motor) {
  motor = (motor_max == motor ? motor_min : (Motor)(((unsigned int)motor) + 1));
  update_lcd(motor);
  return motor;
}

void update_motor_speed(Motor motor) {
  unsigned char speed = motor_speed_get(motor);
  speed = (speed >= MAX_SPEED ? 0 : speed + step);
  motor_speed_set(motor, speed);
  
  // test case to verify get/set works as expected
  if (motor_speed_get(motor) == speed) {
    update_lcd(motor);
  } else {
    cputc_native_user(CHAR_E, CHAR_r, CHAR_r, 0);
    cputc_native_5(CHAR_DASH);
  }
}

void update_motor_dir(Motor motor) {
  MotorDirection dir = motor_dir_get(motor);
  // cycle through directional states in numerical order (off, rev, fwd, brake),
  // but test reverse direction functionality when possible
  switch (dir) {
    case off:
    case fwd:
      // advance from off to rev or from fwd to brake
      dir = (MotorDirection)(((unsigned int)dir) + 1);
      motor_dir_set(motor, dir);
      break;
    case rev:
    case brake:
      // advance from rev to fwd or from brake to off
      dir = motor_dir_reverse(motor);
      break;
  }
  //dir = (dir >= brake ? off : (MotorDirection)(((unsigned int)dir) + 1));
  //motor_dir_set(motor, dir);
  
  // test case to verify get/set works as expected
  if (motor_dir_get(motor) == dir) {
    update_lcd(motor);
  } else {
    cputc_native_user(CHAR_E, CHAR_r, CHAR_r, 0);
    cputc_native_5(CHAR_DASH);
  }
}

/*! Press the "Prgm" button to change the current selected motor
 *  Press the "On-Off" button to change the selected motor's state (float, forward, reverse, brake)
 *  Press the "View" button to change the selected motor's speed
 */
int main(int argc,char *argv[]) {

  Motor current_motor = motor_min;
  unsigned char dkey_current = KEY_NONE;

  // SETUP:  Enable use of both the "On-Off" and "Run" system keys
  allow_system_keys_access(KEY_ONOFF);
  
  // Initialize the display
  update_lcd(current_motor);

  while (!shutdown_requested()) {
    dkey_current = getchar();
    switch (dkey_current) {
      case KEY_VIEW:
        update_motor_speed(current_motor);
        break;
      case KEY_PRGM:
        current_motor = select_next_motor(current_motor);
        break;
      case KEY_ONOFF:
        update_motor_dir(current_motor);
        break;
    }
  }

  // CLEANUP:  Return control of both the "On-Off" and "Run" system keys to the system
  allow_system_keys_access(KEY_NONE);
  cls();

  return 0;
}

#else // CONF_DMOTOR
#warning motor.c requires CONF_DMOTOR
#warning motor demo will do nothing
int main(int argc, char *argv[]) {
  return 0;
}
#endif // CONF_DMOTOR
