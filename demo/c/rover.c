/*! \file   rover.c
    \brief  A simple rover demo
    \author Markus L. Noga <markus@noga.de>

    a simple rover that evades obstacles.
    assumes motors on A,C, touch sensors on port 1,3
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

#include <dsensor.h>
#include <dmotor.h>

#include <tm.h>

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////


// the rover driver
//
int main(int argc, char *argv[]) {
    int dir=0;
    
    while (!shutdown_requested()) {
	motor_a_speed(2*MAX_SPEED/3);		// go!
	motor_c_speed(2*MAX_SPEED/3);
	
	motor_a_dir(fwd);
	motor_c_dir(fwd);
	
    cputc_native_user(CHAR_f, CHAR_w, CHAR_w, CHAR_d);  // fwd
	
	while (!shutdown_requested()
	       && (SENSOR_1 >= 0xf000 && SENSOR_3 >= 0xf000))
	    msleep(1);

	if(SENSOR_1<0xf000)    	      // keep in mind.
	    dir=0;
	else
	    dir=1;
	
	// back up
	//
	motor_a_dir(rev);
	motor_c_dir(rev);
	
    cputc_native_user(CHAR_r, CHAR_e, CHAR_v, 0);  // rev
	
	msleep(500);
	
	motor_a_speed(MAX_SPEED);		// go!
	motor_c_speed(MAX_SPEED);
	
	if(dir==1) {
	    motor_c_dir(fwd);
        cputc_native_user(CHAR_l, CHAR_e, CHAR_f, CHAR_t);  // left
	} else {
	    motor_a_dir(fwd);
        cputc_native_user(CHAR_r, CHAR_g, CHAR_h, CHAR_t);  // rght (right)
	}
	
	msleep(500);
    }
    return 0;
}

#else
#warning rover.c requires CONF_DMOTOR and CONF_DSENSOR
#warning rover demo will do nothing
int main(int argc, char *argv[]) {
  return 0;
}
#endif // CONF_DSENSOR, CONF_DMOTOR
