/*! \file   linetrack.c
    \brief  A simple line tracker demo 
    \author Markus L. Noga <markus@noga.de>
    
    A simple line tracker demo.
    
    Lines are assumed to curve to the right, and initially encountered
    from the right (for example, from the inside of the test pad).
    Straight line tracking needs to define STRAIGHT_LINE.
    
    Assumes motors on A,C, light sensors on port 2
*/
    
/*
 *  The contents of this file are subject to the Mozilla Public License
 *  Version 1.0 (the "License"); you may not use this file except in
 *  compliance with the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *  License for the specific language governing rights and limitations
 *  under the License.
 *
 *  The Original Code is legOS code, released October 17, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 */

#include <config.h>
#if defined(CONF_DSENSOR) && defined(CONF_DMOTOR)

#include <conio.h>
#include <unistd.h>
#include <tm.h>

#include <dsensor.h>
#include <dmotor.h>

#define LIGHTSENS       SENSOR_2
#define DARK_THRESH     0x40
#define BRIGHT_THRESH   0x48

#define NORMAL_SPEED (2*MAX_SPEED/3)
#define TURN_SPEED   (MAX_SPEED)

// #define STRAIGHT_LINE

static wakeup_t dark_found(wakeup_t data) {
  return LIGHT(LIGHTSENS)<(unsigned short)data;
}

static wakeup_t bright_found(wakeup_t data) {
  return LIGHT(LIGHTSENS)>(unsigned short)data;
}

static void locate_line() {
  motor_a_speed(NORMAL_SPEED);
  motor_c_speed(NORMAL_SPEED);
  motor_a_dir(fwd);
  motor_c_dir(fwd);
  
  wait_event(dark_found,DARK_THRESH);
}

static void follow_line() {
  int dir=0;
  
  
  while (!shutdown_requested()) {
    motor_a_speed(NORMAL_SPEED);
    motor_c_speed(NORMAL_SPEED);
    motor_a_dir(fwd);
    motor_c_dir(fwd);
    
    if (wait_event(bright_found,BRIGHT_THRESH) != 0)
    {    
	    if(dir==0)
  	    motor_a_dir(rev);
	    else
  	    motor_c_dir(rev);
#ifdef STRAIGHT_LINE
    		dir=!dir;
#endif
        
	    motor_a_speed(TURN_SPEED);
  	  motor_c_speed(TURN_SPEED);
    
    	wait_event(dark_found,DARK_THRESH);
    }
  }
}

int main(int argc, char *argv[]) {
  ds_active(&LIGHTSENS);
  
  locate_line();
  follow_line();
  
  return 0;
}
#else
#warning linetrack.c requires CONF_DSENSOR and CONF_DMOTOR
#warning linetrack demo will do nothing
int main(int argc, char *argv[]) {
  return 0;
}
#endif // defined(CONF_DSENSOR) && defined(CONF_DMOTOR)
