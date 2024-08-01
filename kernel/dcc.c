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

     File : dcc.c
  Created : 3/20/2003
  Purpose : DCC support functions for DCC library

******************************************************************************/

#include <dcc.h>

#ifdef CONF_DCC

#include <unistd.h>
#include <sys/dmotor.h>
#include <dsensor.h>

// Motor ports on which to output the DCC signal (MOTOR_A_MASK | MOTOR_B_MASK | MOTOR_C_MASK)
unsigned char dcc_mask = 0x00;

unsigned char dm_mask_prev = (MOTOR_A_MASK | MOTOR_B_MASK | MOTOR_C_MASK);

unsigned char p_buffer[6];

/*****************************************************************************/

void dcc_power_on(unsigned char p_dcc_mask)
  {
  unsigned char i;
  
  // Set the motor output masks
  dm_mask_prev = dm_mask;
  dm_mask &= ~p_dcc_mask;
  dcc_mask = p_dcc_mask;

  disable_irqs();
  MOTOR_CONTROLLER = (MOTOR_CONTROLLER & ~dcc_mask) |
      ((MOTOR_A_FWD | MOTOR_B_FWD | MOTOR_C_FWD) & dcc_mask);
  enable_irqs();

  for (i = 0; i < 20; i++) 
    dcc_reset();

  for (i = 0; i < 14; i++) 
    dcc_idle();
  }

/*****************************************************************************/

void dcc_power_off()
  {
  disable_irqs();
  MOTOR_CONTROLLER = (MOTOR_CONTROLLER & ~dcc_mask) |
      ((MOTOR_A_FLOAT | MOTOR_B_FLOAT | MOTOR_C_FLOAT) & dcc_mask);
  enable_irqs();
  
  // Clear the DCC mask
  dcc_mask = 0x00;

  // Restore the motor output mask
  dm_mask = dm_mask_prev;
  }

/*****************************************************************************/

void dcc_ops_packet(unsigned char* data, unsigned char length)
  {
  dcc_packet(16, data, length);
  dcc_flip();
  }

/*****************************************************************************/

static unsigned char p_reset[] = { 0x00, 0x00 };

void dcc_reset()
  {
  dcc_ops_packet(p_reset, sizeof(p_reset));
  }

/*****************************************************************************/

static unsigned char p_idle[] = { 0xFF, 0x00 };

void dcc_idle()
  {
  dcc_ops_packet(p_idle, sizeof(p_idle));
  }


/*****************************************************************************/

void dcc_FL_F4(unsigned char addr, unsigned char bits)
  {
  p_buffer[0] = addr;
  p_buffer[1] = 0x80 | bits;
  dcc_ops_packet(p_buffer, 2);
  }

/*****************************************************************************/

void dcc_F5_F8(unsigned char addr, unsigned char bits)
  {
  p_buffer[0] = addr;
  p_buffer[1] = 0xB0 | bits;
  dcc_ops_packet(p_buffer, 2);
  }

/*****************************************************************************/

void dcc_F9_F12(unsigned char addr, unsigned char bits)
  {
  p_buffer[0] = addr;
  p_buffer[1] = 0xA0 | bits;
  dcc_ops_packet(p_buffer, 2);
  }

/*****************************************************************************/

void dcc_speed14(unsigned char addr, signed char speed, unsigned char fl)
  {
  p_buffer[0] = addr;
  p_buffer[1] = 0x60;
  if (fl)
    p_buffer[1] |= 0x10;
  if (speed < 0)
    p_buffer[1] ^= 0x20,
    speed = -speed;
  if (speed)
    speed++;
  p_buffer[1] |= speed;
  dcc_ops_packet(p_buffer, 2);
  }

/*****************************************************************************/

void dcc_speed28(unsigned char addr, signed char speed)
  {
  p_buffer[0] = addr;
  p_buffer[1] = 0x60;
  if (speed < 0)
    p_buffer[1] ^= 0x20,
    speed = -speed;
  if (speed)
    speed += 3;
  if (speed & 1)
    p_buffer[1] |= 0x10;
  p_buffer[1] |= speed >> 1;
  dcc_ops_packet(p_buffer, 2);
  }

/*****************************************************************************/

void dcc_speed126(unsigned char addr, signed char speed)
  {
  p_buffer[0] = addr;
  p_buffer[1] = 0x3F;
  p_buffer[2] = 0x80;
  if (speed < 0)
    p_buffer[2] ^= 0x80,
    speed = -speed;
  if (speed)
    speed++;
  p_buffer[2] |= speed;
  dcc_ops_packet(p_buffer, 3);
  }

#endif // CONF_DCC
