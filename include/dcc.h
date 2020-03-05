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

     File : dcc.h
  Created : 3/20/2003
  Purpose : C declarations for DCC library

******************************************************************************/

#ifndef _DCC_H
#define _DCC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_DCC

extern unsigned char dcc_mask;
extern unsigned char p_buffer[];

extern void dcc_power_on(unsigned char p_dcc_mask);
extern void dcc_power_off();

extern void dcc_flip();
extern void dcc_packet(unsigned char preamble, unsigned char* packet, unsigned char length);

extern void dcc_ops_packet(unsigned char* data, unsigned char length);
extern void dcc_service_packet(unsigned char* data, unsigned char length, unsigned char count);

extern void dcc_reset();
extern void dcc_idle();

extern void dcc_speed14(int addr, signed char speed, unsigned char fl);
extern void dcc_speed28(int addr, signed char speed);
extern void dcc_speed126(int addr, signed char speed);

#define DCC_FL 0x10
#define DCC_F0 0x10
#define DCC_F1 0x01
#define DCC_F2 0x02
#define DCC_F3 0x04
#define DCC_F4 0x08
extern void dcc_FL_F4(int addr, unsigned char bits);

#define DCC_F5 0x01
#define DCC_F6 0x02
#define DCC_F7 0x04
#define DCC_F8 0x08
extern void dcc_F5_F8(int addr, unsigned char bits);

#define DCC_F9 0x01
#define DCC_F10 0x02
#define DCC_F11 0x04
#define DCC_F12 0x08
extern void dcc_F9_F12(int addr, unsigned char bits);

extern inline void disable_irqs() {
  __asm__ __volatile__("\torc  #0x80,ccr\n":::"cc");
}

extern inline void enable_irqs() {
  __asm__ __volatile__("\tandc #0x7f,ccr\n":::"cc");
}

#endif	// CONF_DCC

#ifdef __cplusplus
}
#endif

#endif

// EOF //
