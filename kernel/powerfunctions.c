/*! \file   powerfunctions.c
    \brief  Implementation: power functions IR control
    \author Carl Troein <carl@thep.lu.se>
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
 *  Based on code by Bob Kojima <bob@fial.com> 2008-01-28
 *
 *  2012-07-11 - Carl Troein <carl@thep.lu.se>
 *   - Updated to match PF RC 1.20 and adapted for Bibo
 */

#include <powerfunctions.h>
#include <semaphore.h>
#include <sys/irq.h>

#ifdef CONF_POWERFUNCTIONS

unsigned char pf_toggle[8] = {0,0,0,0,0,0,0,0};
int pf_repeat = 0;

#ifdef CONF_LNP
extern sem_t tx_sem;                //!< transmitter access semaphore
#else
sem_t tx_sem;
#endif

extern void pf_init(void)
{
  // switch on IR carrier (timer 1, 38.4 kHz) and set long range mode
#ifdef CONF_LNP
  lnp_logical_range(1);
  carrier_init();
#else
  *((char*)&PORT4) &=~1;
  T1_CR = 0x9;    // clear timer on A; clock/8
  T1_CSR = 0x13;  // toggle output on A
  T1_CORA = 0x1a; // 26 usek per cycle
  sem_init(&tx_sem, 0, 1);
#endif
  pf_repeat = 0;
  memset(pf_toggle, 0, 8);
}


//! uncalibrated delay loop
//! \param len approximate time in units of 1.5 microseconds (24 cycles),
//! but each call carries an overhead of 3-4 microsecs.
void ir_pause(unsigned len);

#define IR_PAUSE(x) ir_pause((x)*17-2)


#ifndef DOXYGEN_SHOULD_SKIP_THIS
// 24 cycles (1.5 microsec) 
__asm__("\n\
.text\n\
.align 1\n\
.global _ir_pause\n\
\n\
_ir_pause:\n\
1:	subs  #1, r0\n\
	mov.w r0, r0\n\
	bne   1b\n\
	rts\n");
#endif


// send six IR pulses
static inline void ir_mark(void)
{
//  disable_irqs();
  PORT5 = 0;
  IR_PAUSE(6); // IR sending pause, 6 38kHz cycles (158 us)
  PORT5 = 1;
//  enable_irqs();
}

static void pf_send_once(unsigned code)
{
  sem_wait(&tx_sem);
  // Unfortunately sending is not reliable at all unless we enable interrupts
  disable_irqs();

  S_CR = S_CR & 0xCF; // clear TE and RE bits
  PORT5_DDR = 5; // set direction on the P50 (TxD) to out

  ir_mark();
  IR_PAUSE(39); // start bit pause for 39 38kHz cycles (1026 us)

  int b;
  for(b = 16; b > 0; --b)
  {
    ir_mark();
    if(code & 0x8000)
      IR_PAUSE(21); // high bit pause for 21 38kHz cycles (553 us)
    else
      IR_PAUSE(10); // low bit pause for 10 38kHz cycles (263 us)
    code <<= 1;
  }

  ir_mark();
  enable_irqs(); // the last stop bit can be longer, so enable interrupts now
  IR_PAUSE(39); // stop bit

  S_CR = S_CR | 0x30; // set again TE and RE bits
  sem_post(&tx_sem);
}


//! Send a 16-bit PF message.
//! If pf_repeat is false, the message is sent once. Otherwise it is sent
//! five times, with delays as specified in the PF RC protocol in order to
//! negate the effects of IR collisions. Note that this protocol only works
//! if all IR traffic adheres to it, so there may be interference with LNP.
//! The function blocks interrupts during the whole time the message is
//! sent, roughly 10-12 ms. Finally it waits briefly to allow the
//! receiver to be ready for another message.
extern void pf_send(unsigned c1, unsigned c2, unsigned c3)
{
  // Shifting by 8 is quick but by 4/12 less so, so shift c1 and c3 together.
  unsigned code = (c1 ^ c2 ^ c3 ^ 0xf) | ((c1 << 8 | c3) << 4) | (c2 << 8);
  pf_send_once(code);
  if(pf_repeat)
  {
    msleep(16 * 4);
    pf_send_once(code);
    msleep(16 * 4);
    pf_send_once(code);
    int s = 16 * 7 + 16 * 2 * (c1 & 3);
    msleep(s);
    pf_send_once(code);
    msleep(s);
    pf_send_once(code);
  }
  IR_PAUSE(15);
}

#endif
