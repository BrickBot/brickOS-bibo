/*! \file   include/dsensor.h
    \brief  Interface: direct reading of sensors
    \author Markus L. Noga <markus@noga.de>
 */

/*
 *  The contents of this file are subject to the Mozilla Public License
 *  Version 1.0 (the "License"); you may not use this file except in
 *  compliance with the License. You may obtain a copy of the License
 *  at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 *  the License for the specific language governing rights and
 *  limitations under the License.
 *
 *  The Original Code is legOS code, released October 17, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 */

/*
 *   2000.04.30 - Paolo Masetti <paolo.masetti@itlug.org>
 *
 *	- Some typecast & ()s in macros to avoid strange effects
 *        using them...
 *
 *  2000.09.06 - Jochen Hoenicke <jochen@gnu.org>
 *
 *	- Added velocity calculation for rotation sensor.
 */


#ifndef	__dsensor_h__
#define __dsensor_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_DSENSOR

#include <sys/h8.h>
#include <sys/bitops.h>

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

//
//!< the raw sensors
//
#define SENSOR_1	AD_C	//!< Sensor on input pad 1
#define SENSOR_2	AD_B	//!< Sensor on input pad 2
#define SENSOR_3	AD_A	//!< Sensor on input pad 3
#define BATTERY		AD_D	//!< Battery sensor

//
//!< active light sensor: estimated raw values
//
#define LIGHT_RAW_BLACK 0xffc0	//!< active light sensor raw black value
#define LIGHT_RAW_WHITE 0x5080	//!< active light sensor raw white value


//
// convert raw values to 0 (dark) .. LIGHT_MAX (bright)
// roughly 0-100.
//
#define LIGHT(a)    (147 - ds_scale(a)/7)	//!< map light sensor to 0..LIGHT_MAX
#define LIGHT_MAX   LIGHT(LIGHT_RAW_WHITE)	//!< maximum decoded value

//
// processed active light sensor
//
#define LIGHT_1     LIGHT(SENSOR_1)		//!< light sensor on input 1
#define LIGHT_2     LIGHT(SENSOR_2)		//!< light sensor on input 2
#define LIGHT_3     LIGHT(SENSOR_3)		//!< light sensor on input 3

#ifdef CONF_DSENSOR_ROTATION
//
// processed rotation sensor
//
#define ROTATION_1  (ds_rotations[2])	//!< rotation sensor on input 1	
#define ROTATION_2  (ds_rotations[1])	//!< rotation sensor on input 2	
#define ROTATION_3  (ds_rotations[0])	//!< rotation sensor on input 3	
#endif

#ifdef CONF_DSENSOR_VELOCITY
//
// processed velocity sensor
//
#define VELOCITY_1  (ds_velocities[2])
#define VELOCITY_2  (ds_velocities[1])
#define VELOCITY_3  (ds_velocities[0])
#endif

#ifdef CONF_DSENSOR_MUX
#define SENSOR_1A (ds_muxs[2][0])
#define SENSOR_1B (ds_muxs[2][1])
#define SENSOR_1C (ds_muxs[2][2])
#define SENSOR_2A (ds_muxs[1][0])
#define SENSOR_2B (ds_muxs[1][1])
#define SENSOR_2C (ds_muxs[1][2])
#define SENSOR_3A (ds_muxs[0][0])
#define SENSOR_3B (ds_muxs[0][1])
#define SENSOR_3C (ds_muxs[0][2])
#endif //CONF_DSENSOR_MUX

//! Convert raw data to touch sensor (0: off, else pressed)
#define TOUCH(a)    ((unsigned int)(a) < 0x8000)

//  Processed touch sensors
//
#define TOUCH_1     TOUCH(SENSOR_1)		//!< touch sensor on input 1
#define TOUCH_2     TOUCH(SENSOR_2)		//!< touch sensor on input 2
#define TOUCH_3     TOUCH(SENSOR_3)		//!< touch sensor on input 3


#define ds_scale(x)   ((unsigned int)(x)>>6)	//!< mask off bottom 6 bits
#define ds_unscale(x) ((unsigned int)(x)<<6)	//!< leave room for bottom 6 bits

///////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////

//
// don't manipulate directly unless you know what you're doing!
//

extern unsigned char ds_sensorbits;	//!< sensor bitmask

#ifdef CONF_DSENSOR_ROTATION
extern unsigned char ds_rotation;	//!< rotation   bitmask

extern volatile int ds_rotations[3];	//!< rotational position

#endif
#ifdef CONF_DSENSOR_VELOCITY
extern volatile int ds_velocities[3];	//!< rotational velocity
#endif

#ifdef CONF_DSENSOR_MUX
extern unsigned char ds_mux;	//!< mux   bitmask

extern volatile int ds_muxs[3][3];	//!< mux ch values
#endif //CONF_DSENSOR_MUX

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

//! set sensor mode to active (light sensor emits light, rotation works)
/*! \param  sensor: &SENSOR_1,&SENSOR_2,&SENSOR_3
*/
extern inline void ds_active(volatile unsigned *sensor)
{
  if (sensor == &SENSOR_3)
    bit_set(&ds_sensorbits, 0);
  else if (sensor == &SENSOR_2)
    bit_set(&ds_sensorbits, 1);
  else if (sensor == &SENSOR_1)
    bit_set(&ds_sensorbits, 2);
}

//! set sensor mode to passive (light sensor detects ambient light)
/*! \param  sensor: &SENSOR_1,&SENSOR_2,&SENSOR_3
*/
extern inline void ds_passive(volatile unsigned *sensor)
{
  if (sensor == &SENSOR_3) {
    bit_clear(&ds_sensorbits, 0);
    bit_clear(&ds_sensorbits, 4);
    bit_clear(&PORT6, 0);
  } else if (sensor == &SENSOR_2) {
    bit_clear(&ds_sensorbits, 1);
    bit_clear(&ds_sensorbits, 5);
    bit_clear(&PORT6, 1);
  }  else if (sensor == &SENSOR_1) {
    bit_clear(&ds_sensorbits, 2);
    bit_clear(&ds_sensorbits, 6);
    bit_clear(&PORT6, 2);
  }
}

#ifdef CONF_DSENSOR_ROTATION
//! set rotation to an absolute value
/*! \param sensor one of &SENSOR_1, &SENSOR_2, or &SENSOR_3
    \param  pos the current rotational postion (typically use 0 here)
	\return Nothing

    The axis should be inert during the function call.
*/
extern void ds_rotation_set(volatile unsigned *sensor, int pos);

//! start tracking rotation sensor
/*! \param  sensor: &SENSOR_1,&SENSOR_2,&SENSOR_3
*/
extern inline void ds_rotation_on(volatile unsigned *sensor)
{
    if (sensor == &SENSOR_3) {
	bit_set(&ds_rotation, 0);
	bit_set(&ds_sensorbits,4);
    } else if (sensor == &SENSOR_2) {
	bit_set(&ds_rotation, 1);
	bit_set(&ds_sensorbits,5);
    } else if (sensor == &SENSOR_1) {
	bit_set(&ds_rotation, 2);
	bit_set(&ds_sensorbits, 6);
    }
}

//! stop tracking rotation sensor
/*! \param  sensor: &SENSOR_1,&SENSOR_2,&SENSOR_3
*/
extern inline void ds_rotation_off(volatile unsigned *sensor)
{
    if (sensor == &SENSOR_3) {
	bit_clear(&ds_rotation, 0);
	bit_clear(&ds_sensorbits, 4);
    }
    else if (sensor == &SENSOR_2) {
	bit_clear(&ds_rotation, 1);
	bit_clear(&ds_sensorbits, 5);
    }
    else if (sensor == &SENSOR_1) {
	bit_clear(&ds_rotation, 2);
	bit_clear(&ds_sensorbits, 6);
    }
}
#endif // CONF_DSENSOR_ROTATION


#ifdef CONF_DSENSOR_MUX

#define DS_MUX_POST_SWITCH 150
//! start multiplexing
/*! \param  sensor: &SENSOR_1,&SENSOR_2,&SENSOR_3
    \param  ch1: indicates if ch1 is to be scanned
    \param  ch2: indicates if ch2 is to be scanned
    \param  ch3: indicates if ch3 is to be scanned
    ch1-ch3 also indicates how long to wait after switching
    to a channel before reading from it.
    DS_MUX_POST_SWITCH defines a good default, but some sensors
    require more time, others may work with less.
    specifying 0 will keep the port from being switched to
    or read
*/
extern void ds_mux_on(volatile unsigned *sensor,
		      unsigned int ch1,
		      unsigned int ch2,
		      unsigned int ch3);


//! stop multiplexing
/*! \param  sensor: &SENSOR_1,&SENSOR_2,&SENSOR_3
*/
extern inline void ds_mux_off(volatile unsigned *sensor)
{
  if (sensor == &SENSOR_3)
    bit_clear(&ds_mux, 0);
  else if (sensor == &SENSOR_2)
    bit_clear(&ds_mux, 1);
  else if (sensor == &SENSOR_1)
    bit_clear(&ds_mux, 2);
}//endof ds_mux_off
#endif // CONF_DSENSOR_MUX





#endif // CONF_DSENSOR

#ifdef  __cplusplus
}
#endif

#endif // __dsensor_h__
