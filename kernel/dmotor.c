/*! \file   dmotor.c
    \brief  Implementation: direct motor control
    \author Markus L. Noga <markus@noga.de>
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
 *                  Lou Sortman <lou (at) sunsite (dot) unc (dot) edu>
 *
 *  2003.06.12 - Mark Riley <markril@hotmail.com>
 * - Patch for DCC driver
 *
 * 2009.04.21 - Matthew Sheets <mesheets@users.sourceforge.net>
 * - Fix for dm_mask assembly code in dm_interrupt()
 */

#include <sys/dmotor.h>
#include <dlcd.h>

#ifdef CONF_DMOTOR

#include <sys/h8.h>
#include <sys/irq.h>
#include <rom/lcd.h>

///////////////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////////////

MotorState dm_a = {               //!< motor A state
  { 0 },  0, MOTOR_A_SHIFT,
#ifdef CONF_VIS
  a_select, a_left, a_right,
#endif // CONF_VIS
};
MotorState dm_b = {               //!< motor B state
  { 0 },  0, MOTOR_B_SHIFT,
#ifdef CONF_VIS
  b_select, b_left, b_right,
#endif // CONF_VIS
};
MotorState dm_c = {               //!< motor C state
  { 0 },  0, MOTOR_C_SHIFT,
#ifdef CONF_VIS
  c_select, c_left, c_right,
#endif // CONF_VIS
};

MotorState* dmotor[] = {          //!< motor states
  &dm_a,
  &dm_b,
  &dm_c,
};

unsigned char dm_mask = (MOTOR_A_MASK | MOTOR_B_MASK | MOTOR_C_MASK);

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

//! direct motor output handler
/*! called by system timer in the 16bit timer OCIA irq
*/
extern void dm_interrupt(void);
#ifndef DOXYGEN_SHOULD_SKIP_THIS
__asm__("\n\
.section .text\n\
.align 1\n\
.global _dm_interrupt\n\
_dm_interrupt:\n\
		; r6 saved by ROM\n\
                ; r0 saved by systime_handler\n\
"
#ifdef CONF_DMOTOR_HOLD
"		mov.b #0xcf,r6l			; r6l is output\n"
#else
"     	      	sub.w r6,r6   	      	      	; r6l is output\n"
#endif
"               ; we simultaneously load delta (r0h) and sum (r0l)\n\
                ; this depends on byte order, but the H8 will stay MSB\n\
                ; and the resulting code is efficient and compact.\n\
                \n\
                ; motor A\n\
                \n\
                mov.w   @_dm_a,r0\n\
		add.b	#1,r0h			; maps 255 to 256\n\
		dec.b	r0h\n\
                addx.b	r0h,r0l                 ; add delta to sum\n\
                bcc     dm0                     ; sum overflow?\n\
		  mov.b	@_dm_a+2,r6h            ; -> output drive pattern\n\
		  xor.b	r6h,r6l\n\
            dm0:mov.b   r0l,@_dm_a+1            ; save sum\n\
\n\
                ; motor B\n\
                \n\
                mov.w   @_dm_b,r0\n\
		add.b	#1,r0h			; maps 255 to 256\n\
		dec.b	r0h\n\
                addx.b	r0h,r0l                 ; add delta to sum\n\
                bcc     dm1                     ; sum overflow?\n\
		  mov.b	@_dm_b+2,r6h            ; -> output drive pattern\n\
		  xor.b	r6h,r6l\n\
            dm1:mov.b   r0l,@_dm_b+1            ; save sum\n\
\n\
                ; motor C\n\
                \n\
                mov.w   @_dm_c,r0\n\
		add.b	#1,r0h			; maps 255 to 256\n\
		dec.b	r0h\n\
                addx.b	r0h,r0l                 ; add delta to sum\n\
                bcc     dm2                     ; sum overflow?\n\
		  mov.b	@_dm_c+2,r6h            ; -> output drive pattern\n\
		  xor.b	r6h,r6l\n\
            dm2:mov.b   r0l,@_dm_c+1            ; save sum\n\
\n\
		; driver chip\n\
\n\
		mov.b	@_motor_controller:8,r6h	; current motor bits\n\
		mov.b	@_dm_mask,r0l			; mask of bits that can change\n\
        and.b   r0l,r6l             ; mask off the new dmotor bits we don't want to change\n\
        not.b   r0l                 ; flip the bit mask\n\
        and.b   r0l,r6h             ; mask to preserve the current dmotor bits we want to keep\n\
        or.b    r6h,r6l             ; combine the new bits and preserved bits\n\
                  \n\
		mov.b	r6l,@_motor_controller:8	; output motor waveform\n\
		\n\
		rts		\n\
	");
#endif // DOXYGEN_SHOULD_SKIP_THIS
	

//! initialize motors
//
void dm_init(void) {
  dm_mask = (MOTOR_A_MASK | MOTOR_B_MASK | MOTOR_C_MASK);
  dm_shutdown();				// shutdown hardware
}


//! shutdown motors
//
void dm_shutdown(void) {
  // initialize driver data
  unsigned char i;
  for (i = (unsigned char)motor_min; i <= (unsigned char)motor_max; i++) {
    motor_dir_set((Motor)i, off);
    motor_speed_set((Motor)i, MAX_SPEED);
  }

  motor_controller=0x00;		// shutdown hardware
}

#ifdef CONF_VIS

void motor_select_show(Motor motor)
{
  lcd_show(dmotor[motor]->motor_lcd_segment_select);
}

void motor_select_hide(Motor motor)
{
  lcd_hide(dmotor[motor]->motor_lcd_segment_select);
}

static void motor_dir_show(MotorDirection dir,
               lcd_segment motor_segment_left, lcd_segment motor_segment_right)
{
  if (dir == fwd || dir == brake) {
    lcd_show(motor_segment_right);
  } else {
    lcd_hide(motor_segment_right);
  }
  if (dir == rev || dir == brake) {
    lcd_show(motor_segment_left);
  } else {
    lcd_hide(motor_segment_left);
  }
}

#endif // ifdef CONF_VIS

void motor_dir_set(Motor motor, MotorDirection dir) {
  dmotor[motor]->dir = dir << dmotor[motor]->shift;

#ifdef CONF_VIS
  motor_dir_show(dir, dmotor[motor]->motor_lcd_segment_left, dmotor[motor]->motor_lcd_segment_right);
#endif
}

MotorDirection motor_dir_get(Motor motor) {
  // Mask off the motor-specific value and then shift to standard position
  return (MotorDirection)((dmotor[motor]->dir & (MOTOR_MASK << dmotor[motor]->shift)) >> dmotor[motor]->shift);
}

MotorDirection motor_dir_reverse(Motor motor) {
  MotorDirection dir = motor_dir_get(motor);
  dir = (MotorDirection)(MOTOR_MASK & (~dir));
  motor_dir_set(motor, dir);
  
  return dir;
}

void motor_speed_set(Motor motor, unsigned char speed) {
  dmotor[motor]->access.c.delta = speed;
}

unsigned char motor_speed_get(Motor motor) {
  return dmotor[motor]->access.c.delta;
}

#endif // CONF_DMOTOR
