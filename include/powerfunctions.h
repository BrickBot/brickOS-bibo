/*! \file   powerfunctions.h
    \brief  Interface: power functions IR control
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
 *  2012-05-15 - Carl Troein <carl@thep.lu.se>
 *   - Updated to match PF RC 1.20 and adapted for Bibo
 */

#include <conio.h>
#include <unistd.h>

#include <lnp-logical.h>
#include <sys/h8.h>
#include <sys/time.h>

//Channels
#define PF_CH1 0x0
#define PF_CH2 0x1
#define PF_CH3 0x2
#define PF_CH4 0x3

//Combo direct mode speeds
#define PF_RED_FLT 0x0
#define PF_RED_FWD 0x1
#define PF_RED_REV 0x2
#define PF_RED_BRK 0x3
#define PF_BLUE_FLT 0x0
#define PF_BLUE_FWD 0x4
#define PF_BLUE_REV 0x8
#define PF_BLUE_BRK 0xC

//PWM speed steps
#define PWM_FLT 0x0
#define PWM_FWD1 0x1
#define PWM_FWD2 0x2
#define PWM_FWD3 0x3
#define PWM_FWD4 0x4
#define PWM_FWD5 0x5
#define PWM_FWD6 0x6
#define PWM_FWD7 0x7
#define PWM_BRK 0x8
#define PWM_REV7 0x9
#define PWM_REV6 0xA
#define PWM_REV5 0xB
#define PWM_REV4 0xC
#define PWM_REV3 0xD
#define PWM_REV2 0xE
#define PWM_REV1 0xf

//Single output mode commands
#define PF_TOGGLE_FWD 0x0
#define PF_TOGGLE_DIR 0x1
#define PF_INC_NUM_PWM 0x2
#define PF_DEC_NUM_PWM 0x3
#define PF_INC_PWM 0x4
#define PF_DEC_PWM 0x5
#define PF_FULL_FWD 0x6
#define PF_FULL_REV 0x7
#define PF_TOGGLE_FWD_REV 0x8
#define PF_CLEAR_C1 0x9
#define PF_SET_C1 0xa
#define PF_TOGGLE_C1 0xb
#define PF_CLEAR_C2 0xc
#define PF_SET_C2 0xd
#define PF_TOGGLE_C2 0xe
#define PF_TOGGLE_REV 0xf

//Single output mode outputs
#define PF_RED 0x0
#define PF_BLUE 0x1

//Modes
#define PF_COMBO_DIRECT 0x1
#define PF_SINGLE_OUTPUT_PWM 0x4
#define PF_SINGLE_OUTPUT 0x6
//Escaped mode
#define PF_COMBO_PWM 0x4

//Useful extended mode operations
#define PF_TOGGLE_ADDRESS 0x6
#define PF_ALIGN_TOGGLE 0x7

//For convenience in pf_ functions: add extra address bit to mode if needed
#define PF_EXADDR(mode) (extra_address ? (mode) | 8 : (mode))

/**
	Notes/clarifications about power functions:

	The receiver is not able to process a message that is sent too quickly
	after the stop bit of the previous message. Extending the stop bit to 54
	cycles appears to solve the problem.

	The effect of "PWM numeric increase/decrease" PF_INC(DEC)_NUM_PWM is to
	increase or decrease the absolute value of the PWM speed. Decreasing to
	speed zero gives FLT, as does inc/dec from FLT or BRK.

	When the receiver is powered on, it expects the next message to have the
	toggle bit cleared.

	After a Combo PWM message, the receiver expects the toggle bit to be set.

	As documented, the toggle bit is not validated for single output mode
	messages wth clear/set/full_* commands. However, the next message is
	expected to have the opposite value (assuming it is validated).

	The toggle bit is not affected by messages in the other address space,
	nor by messages to other channels.

	'Toggle full forward' will stop from forward PWM.

	'Toggle direction' changes timeout modes into non-timeout. PWM is reversed
	correctly. Stop remains stop (haven't tested BRK vs FLT).
	
	If C1/C2 are altered individually, the receiver's notion of the current
	direction/speed remain unchanged. It may be restored by 'toggle direction'.

*/

#ifdef CONF_POWERFUNCTIONS

//! Initialize IR transmitter for power functions
void pf_init(void);

//! Send a 16-bit PF message.
//! If pf_repeat is false, the message is sent once. Otherwise it is sent
//! five times, with delays as specified in the PF RC protocol in order to
//! negate the effects of IR collisions. Note that this protocol only works
//! if all IR traffic adheres to it, so there may be interference with LNP.
//! The function blocks interrupts during the whole time the message is
//! sent, roughly 10-12 ms. Finally it waits for 0.4 ms us to allow the
//! receiver to be ready for another message.
void pf_send(unsigned c1, unsigned c2, unsigned c3);

//! ir message toggle bit, one per channel and address space (1-4, extra 1-4)
extern unsigned char pf_toggle[8];

//! Whether to repeat PF messages 5 times. Doing so will make pf_send take
//! approximately 400-600 ms instead of the 10-13 ms required for a single
//! message.
extern int pf_repeat;

// For all these functions, if extra_address is true then the command is sent
// with the extra address space bit set, as described in PF RC protocol v1.20.

//! send one COMBO_DIRECT message
//! e.g. pf_combo_direct(PF_CH1, PF_RED_BRK | PF_BLUE_FWD, false)
static inline void pf_combo_direct(unsigned channel, unsigned speeds,
  unsigned extra_address)
{
  pf_send(channel, PF_EXADDR(PF_COMBO_DIRECT), speeds);
  pf_toggle[channel + extra_address * 4] = 8;
}

//! send one SINGLE_OUTPUT message in PWM mode
//! e.g. pf_single_output_pwm(PF_CH1, PF_RED, PWM_FWD5, false)
static inline void pf_single_output_pwm(unsigned channel,
  unsigned output, unsigned pwm, unsigned extra_address)
{
  unsigned char ix = extra_address * 4 + channel;
  pf_send(pf_toggle[ix] | channel,
    output | PF_EXADDR(PF_SINGLE_OUTPUT_PWM), pwm);
  pf_toggle[ix] ^= 8;
}

//! send one SINGLE_OUTPUT message in C/S/T/I/D mode
//! e.g. pf_single_output(PF_CH1, PF_RED, PF_DEC_NUM_PWM, false)
static inline void pf_single_output(unsigned channel,
  unsigned output, unsigned cmd, unsigned extra_address)
{
  unsigned char ix = extra_address * 4 + channel;
  pf_send(pf_toggle[ix] | channel, output | PF_EXADDR(PF_SINGLE_OUTPUT), cmd);
  pf_toggle[ix] ^= 8;
}

//! send one COMBO_PWM message
//! e.g. pf_combo_pwm(PF_CH1, PWM_FWD5, PWM_REV3, false)
static inline void pf_combo_pwm(unsigned channel,
  unsigned pwm_blue, unsigned pwm_red, unsigned extra_address)
{
  pf_send(PF_EXADDR(PF_COMBO_PWM) | channel, pwm_blue, pwm_red);
  pf_toggle[extra_address * 4 + channel] = 8;
}

//! align toggle bit (e.g. to make sure that the next command has a chance
// of success).
//! e.g. pf_align_toggle(PF_CH1, false)
static inline void pf_align_toggle(int channel, unsigned extra_address)
{
  unsigned char ix = extra_address * 4 + channel;
  pf_send(pf_toggle[ix] | channel, PF_EXADDR(0), PF_ALIGN_TOGGLE);
  pf_toggle[ix] ^= 8;
}

//! toggle the extra/default address space selector
//! e.g. pf_toggle_address(PF_CH1, false)
static inline void pf_toggle_address(int channel, unsigned extra_address)
{
  unsigned char ix = extra_address * 4 + channel;
  pf_send(pf_toggle[ix] | channel, PF_EXADDR(0), PF_TOGGLE_ADDRESS);
  pf_toggle[ix ^ 4] = pf_toggle[ix] ^= 8;
}

#endif // CONF_POWERFUNCTIONS
