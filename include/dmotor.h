/*! \file   include/dmotor.h
    \brief  Interface: direct motor control
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

#ifndef	__dmotor_h__
#define __dmotor_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_DMOTOR

#ifdef CONF_VIS
#include <rom/lcd.h>
#endif

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

#ifndef DOXYGEN_SHOULD_SKIP_INTERNALS
//! the motor status type.
typedef struct {
  union {
    unsigned assembler;		//!< assures word alignment for assembler

    struct {
      unsigned char delta;	//!< the speed setting

      volatile unsigned char sum;	//!< running sum

    } c;
  } access;			//!< provides access from C and assembler

  unsigned char dir;		//!< output pattern when sum overflows
  unsigned char shift;		//!< number of bits to shift for the motor

#ifdef CONF_VIS
  lcd_segment motor_lcd_segment_select;
  lcd_segment motor_lcd_segment_left;
  lcd_segment motor_lcd_segment_right;
#endif // CONF_VIS

} MotorState;
#endif  // DOXYGEN_SHOULD_SKIP_INTERNALS

#define  MIN_SPEED	0     	//!< minimum motor speed
#define  MAX_SPEED	255   	//!< maximum motor speed

#define MOTOR_MASK 0x03

#define MOTOR_A_SHIFT 6
#define MOTOR_B_SHIFT 2
#define MOTOR_C_SHIFT 0

#define MOTOR_A_MASK  (MOTOR_MASK << MOTOR_A_SHIFT)
#define MOTOR_B_MASK  (MOTOR_MASK << MOTOR_B_SHIFT)
#define MOTOR_C_MASK  (MOTOR_MASK << MOTOR_C_SHIFT)

#define MOTOR_FLOAT   0x00
#define MOTOR_REV     0x01
#define MOTOR_FWD     0x02
#define MOTOR_BRAKE   0x03

#define MOTOR_A_BRAKE (MOTOR_BRAKE << MOTOR_A_SHIFT)
#define MOTOR_A_FWD   (MOTOR_FWD   << MOTOR_A_SHIFT)
#define MOTOR_A_REV   (MOTOR_REV   << MOTOR_A_SHIFT)
#define MOTOR_A_FLOAT (MOTOR_FLOAT << MOTOR_A_SHIFT)

#define MOTOR_B_BRAKE (MOTOR_BRAKE << MOTOR_B_SHIFT)
#define MOTOR_B_FWD   (MOTOR_FWD   << MOTOR_B_SHIFT)
#define MOTOR_B_REV   (MOTOR_REV   << MOTOR_B_SHIFT)
#define MOTOR_B_FLOAT (MOTOR_FLOAT << MOTOR_B_SHIFT)

#define MOTOR_C_BRAKE (MOTOR_BRAKE << MOTOR_C_SHIFT)
#define MOTOR_C_FWD   (MOTOR_FWD   << MOTOR_C_SHIFT)
#define MOTOR_C_REV   (MOTOR_REV   << MOTOR_C_SHIFT)
#define MOTOR_C_FLOAT (MOTOR_FLOAT << MOTOR_C_SHIFT)

//! the motors
typedef enum {
  motor_min,
  motor_a = motor_min,
  motor_b,
  motor_c,
  motor_max = motor_c
} Motor;

//! the motor directions
typedef enum {
  off = MOTOR_FLOAT,	//!< freewheel
  fwd = MOTOR_FWD,		//!< forward
  rev = MOTOR_REV,	    //!< reverse
  brake = MOTOR_BRAKE	//!< hold current position
} MotorDirection;

///////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////

extern MotorState  dm_a,	//!< motor A state
                   dm_b,	//!< motor B state
                   dm_c;	//!< motor C state

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

#ifdef CONF_VIS

extern void motor_select_show(Motor motor);

extern void motor_select_hide(Motor motor);

#endif // ifdef CONF_VIS

//!< set the directional state of the given motor to dir
/*! \param motor the motor
 *  \param dir the direction
 */
extern void motor_dir_set(Motor motor, MotorDirection dir);

//!< get the directional state of the given motor
/*! \param motor the motor
 *  \return the motor direction
 */
extern MotorDirection motor_dir_get(Motor motor);

//!< reverses the directional state of the given motor (e.g. brake -> off or fwd -> rev)
/*! \param motor the motor
 *  \return the new motor direction
 */
extern MotorDirection motor_dir_reverse(Motor motor);

//!< set the speed of the specified motor
/*! \param motor the motor
 *  \param speed the speed
 */
extern void motor_speed_set(Motor motor, unsigned char speed);

//!< get the speed of the specified motor
/*! \param motor the motor
 *  \return the motor speed
 */
extern unsigned char motor_speed_get(Motor motor);

#endif // CONF_DMOTOR

#ifdef  __cplusplus
}
#endif

#endif // __dmotor_h__
