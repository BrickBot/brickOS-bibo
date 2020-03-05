/*! \file   dsensor.c
    \brief  Implementation: direct sensor access
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
 *                  Eric Habnerfeller <ehaberfe@atitech.ca>
 *                  Lou Sortman <lou@sunsite.unc.edu>
 */

/*
 *  2000.03.11 - Paolo Masetti <paolo.masetti@itlug.org>
 *
 *	- Included a fix for rotation sensor posted by "Ben Jackson"
 *        on lugnet.robotics.rcx.legos
 *
 *  2000.04.30 - Paolo Masetti <paolo.masetti@itlug.org>
 *
 *	- ISR Reading routine fix to make read values stable.
 *	- Fixed rotation sensor status table values to avoid offset problems.
 *
 *  2000.09.06 - Jochen Hoenicke <jochen@gnu.org>
 *
 *	- Added velocity calculation for rotation sensor.
 */

#include <dsensor.h>

#ifdef CONF_DSENSOR

#include <sys/h8.h>
#include <sys/irq.h>
#include <sys/bitops.h>
#include <sys/handlers.h>
#include <rom/registers.h>
#include <unistd.h>
#include <conio.h>

///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////////////

#define DS_ALL_ACTIVE         0x07             //!< all sensors active mode
#define DS_ALL_PASSIVE        (~DS_ALL_ACTIVE) //!< all sensors passive mode

///////////////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////////////

volatile unsigned char ds_channel;        //!< current A/D channel

/**
 * active sensor bitmask.  if bit 0-3 is set the corresponding channel
 * is active, if bit 4-7 is set the corresponding channel needs a bottom
 * handler.
 */
unsigned char ds_sensorbits;

/**
 * bitmask for sensors with pending changes.
 */
unsigned char ds_dirty;

static void ds_callback(void);
handler_t ds_handler = { 0, ds_callback };


#ifdef CONF_DSENSOR_ROTATION
unsigned char ds_rotation;                //!< rotation bitmask. 1-> active
volatile int ds_rotations[3];             //!< sensor revolutions * 16

static signed char rotation_state[3];     //!< rotation state
static signed char rotation_new_state[3]; //!< proposed rotation state
static unsigned int state_duration[3];    //!< proposed rotation state duration

#ifdef CONF_DSENSOR_VELOCITY
volatile int ds_velocities[3];            //!< sensor velocity
static unsigned int last_rotation[3];     //!< last time of rotation signal
static unsigned int next_rotation[3];     //!< rough upper estimatation of next signal time
static signed char rotation_dir[3];       //!< direction of last rotation
#endif



//! convert a/d values to rotation states
/*! Indexed with (value>>12).
    Invalid values yield negative states.
*/
static const signed char ad2state[16]={
  // 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f	// (sensor value>>12)
    -1,-1,-1,-1,-1, 2, 2, 2, 3, 3, 3, 3, 1, 1, 1, 0	// New values to be used
							// with delayed read

//  -1,-1,-1,-1,-1,-1, 2, 2, 2, 3, 3, 3, 1, 1, 1, 0	// Old values: biased for
							// non-delayed read

};

//! convert state difference to revolution count change
/*! Indexed with (newstate-state)+3.
    Invalid differences yield zero change.
    Differences of magnitude two could have been achieved in either
    rotational sense, so their expected value is zero.
*/
static const signed char diff2change[7]={
  //-3 -2 -1  0  1  2  3      	      	      	      // newstate-state
     1, 0,-1, 0, 1, 0,-1
};

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

//! set rotation to an absolute value
/*! \param sensor the sensor address, can be &SENSOR_1, &SENSOR_2 or &SENSOR_3
    \param pos    desired absolute position

    axis should be inert during the function call
*/
void ds_rotation_set(volatile unsigned *sensor,int pos) {
  if(sensor>=&AD_A && sensor<=&AD_C) {    // catch range violations
    unsigned channel=(unsigned) (sensor-&AD_A);
    signed char state=ad2state[(*sensor)>>12];

    if(state<0)
      state=0;

    rotation_state[channel]=state;
    rotation_new_state[channel] = -1;
    state_duration[channel]=0;
    ds_rotations[channel]=pos;            // reset counter

  }
}

/* We use only low 16 bit of sys_time, which is safe */
extern volatile time_t sys_time;

//! process rotation sensor on current A/D channel
/*! \sa ds_channel current channel (global input value)
*/
void ds_rotation_handler(unsigned channel) {
  unsigned    raw     =(*((&AD_A)+channel));
  signed char newstate=ad2state[raw>>12];
  signed char change = diff2change[newstate - rotation_state[channel] + 3];

  if (change && newstate >= 0) {
      ds_rotations[channel] += change;

#ifdef CONF_DSENSOR_VELOCITY
      {
	/* We only take the lowest 16 bits of sys_time.  We have to be
	 * a bit careful with wraparounds, but this is handled here.
	 */
	unsigned int time = (unsigned int) sys_time;
	if (change != rotation_dir[channel]) {
	  rotation_dir[channel] = change;
	  ds_velocities[channel] = 0;
	  last_rotation[channel] = time;
	  next_rotation[channel] = time + 1000;
	} else {
	  if (time == last_rotation[channel])
	    ds_velocities[channel] = 1000 * change;
	  else {
	    unsigned int time_diff = (time - last_rotation[channel]);
	    if (time_diff > 1000) {
	      rotation_dir[channel] = 0;
	      ds_velocities[channel] = 0;
	    } else {
	      int speed = 1000 / time_diff;
	      ds_velocities[channel] = change > 0 ? speed : -speed;
	      last_rotation[channel] = time;
	      next_rotation[channel] = time + time_diff * 3 / 2;
	    }
	  }
	}
      }
#endif
      rotation_state[channel] = newstate;
#ifdef CONF_DSENSOR_VELOCITY
  } else {
    /* No rotation change, check if velocity measure timeouts. */
    unsigned int time = (unsigned int) sys_time;
    if (rotation_dir[channel] &&  
	((signed int) (time - next_rotation[channel])) >= 0) {
      unsigned int time_diff = (time - last_rotation[channel]);
      if (time_diff > 1000) {
	rotation_dir[channel] = 0;
	ds_velocities[channel] = 0;
      } else {  
	int speed = 1000 / time_diff;
	ds_velocities[channel] = rotation_dir[channel] > 0 ? speed : -speed;
	next_rotation[channel] = time + time_diff / 2;
      }
    }
#endif
  }

}
#endif // CONF_DSENSOR_ROTATION

#ifdef CONF_DSENSOR_EDGECOUNT
unsigned char ds_edgecount;			//!< bitmask
volatile unsigned int ds_edgecounts[3];		//!< edge counts
unsigned char ds_edgecount_state[3];		//!< overall state, prev sample state
unsigned char ds_edgecount_level_low[3];	//!< maximum (raw>>8) value for 'off'
unsigned char ds_edgecount_level_high[3];	//!< minimum (raw>>8) value for 'on'


void ds_edgecount_handler(unsigned channel)
{
  unsigned char raw = (*((&AD_A)+channel)) >> 8;
  unsigned char state = ds_edgecount_state[channel];
  if(raw < ds_edgecount_level_low[channel])
  {
    if(state & 1)
      state &= ~1;
    else if(state & 2)
    {
      state &= ~2;
      ++ds_edgecounts[channel];
    }
    else
      return;
  }
  else if(raw > ds_edgecount_level_high[channel])
  {
    if(!(state & 1))
      state |= 1;
    else if(!(state & 2))
    {
      state |= 2;
      ++ds_edgecounts[channel];
    }
    else
      return;
  }
  else
    return;
  ds_edgecount_state[channel] = state;
}

#endif // CONF_DSENSOR_EDGECOUNT

#ifdef CONF_DSENSOR_MUX
unsigned char ds_mux;	//!< mux   bitmask

volatile int ds_muxs[3][3];	//!< mux ch values


//width of each mux pulse
#define DS_MUX_PULSE_TM_MS 10



typedef struct {
  unsigned long nextTm;  //timestamp for next pulse
  char remainingPulses;    //pulses left in pulse train
  char channel; //current mux sub channel (0,1,2)
  unsigned int attached[3];//what channels are sensors attached to
                            //this also defines the number of ms
                            //to wait before reading the value
  
  enum {ds_mux_prepRead,
	ds_mux_read, 
	ds_mux_pulse_low, 
	ds_mux_pulse_high} action; //specify next action
} ds_mux_data_t;

ds_mux_data_t ds_mux_data[3]; //data on mux

#endif //CONF_DSENSOR_MUX

#ifdef CONF_DSENSOR_MUX

static inline void ds_power_on(unsigned channel) {
    PORT6 |= (1 << channel);
}

static inline void ds_power_off(unsigned channel) {
    PORT6 &= ~(1 << channel);
}

//! start multiplexing
void ds_mux_on(volatile unsigned *sensor,
	       unsigned int ch1,
	       unsigned int ch2,
	       unsigned int ch3) {
  unsigned char i,j;
  ds_passive(sensor);//powered, but not active in legOS sense


  if(ch1==0 &&
     ch2==0 &&
     ch3==0) {
    //umm this is useless
    //avoid endless cycling
    ds_mux_off(sensor);
    return;
  }

  i = sensor - &SENSOR_3;
  ds_mux_data[i].attached[0]=ch1;
  ds_mux_data[i].attached[1]=ch2;
  ds_mux_data[i].attached[2]=ch3;
  
  //add extended time based on the channel
  //this is required by the mux
  //the user supplies extra time based on the
  //type of sensor they hook up
  //these defaults give enough time to read
  //a light sensor and should be ok for most
  //sensors
  if(ch1)
    ds_mux_data[i].attached[0]+=160;
  if(ch2)
    ds_mux_data[i].attached[1]+=135;
  if(ch3)
    ds_mux_data[i].attached[2]+=25;

  //check if we're just adjusting the ports
  //if so we can return here
  if (ds_mux & (1 << i))
    return;

  //starting up mux

  //power up
  ds_power_on(i);
  
  //schedule first event
  //find first attached sensor
  for(j=0;j<3 && ds_mux_data[i].attached[j]==0;j++);
    
  ds_mux_data[i].channel=j;
  ds_mux_data[i].remainingPulses = j+1;
  ds_mux_data[i].action=ds_mux_pulse_low;
  ds_mux_data[i].nextTm=sys_time+DS_MUX_PULSE_TM_MS;

  ds_mux |= 1 << i;

}//endof ds_mux_on



void ds_mux_handler(unsigned sen) {
  if(ds_mux_data[sen].nextTm <= sys_time) {
    //we've reached our next scheduled step
    //lcd_int(sys_time-ds_mux_data[sen].nextTm);
    switch(ds_mux_data[sen].action) {
    case ds_mux_prepRead:
      ds_power_off(sen);//power down for read
      ds_mux_data[sen].action=ds_mux_read;
      ds_mux_data[sen].nextTm=sys_time;//do it ASAP, but not now
      break;
    case ds_mux_read:
      //read data
      ds_muxs[sen][(int)ds_mux_data[sen].channel]=*(&AD_A + sen);

 
      //change channel
      do {
	ds_mux_data[sen].channel++;
	if(ds_mux_data[sen].channel>2/*max chan*/) {
	  ds_mux_data[sen].channel=0;
	}
	//make sure selected channel is marked attached
	//don't worry about an endless loop ds_mux_on makes
	//sure at least one channel is attached
      } while(
	      (ds_mux_data[sen].attached
	       [(int)ds_mux_data[sen].channel])==0);
      
    
      //use this low pulse as the first low pulse of next train

      ds_mux_data[sen].remainingPulses = ds_mux_data[sen].channel + 1;

      //schedule next high pulse
      ds_mux_data[sen].action=ds_mux_pulse_high;
      ds_mux_data[sen].nextTm=sys_time+DS_MUX_PULSE_TM_MS;
      break;
    case ds_mux_pulse_low:
      //go low
      ds_power_off(sen);
      //schedule next high pulse
      ds_mux_data[sen].nextTm=sys_time+DS_MUX_PULSE_TM_MS;
      ds_mux_data[sen].action=ds_mux_pulse_high;
      break;
    case ds_mux_pulse_high:
      //go high
      ds_power_on(sen);

      if(--ds_mux_data[sen].remainingPulses==0) {
	//done with train
	//schedule prepRead
	ds_mux_data[sen].action=ds_mux_prepRead;

	//schedule enough time for the mux to make the switch
	//this is scaled because the timeout the mux uses starts
	//when the first pulse comes in, it is around 70ms, so
	//when switching to sensor 1 we must want an additional
	//amount of time before it mux reacts, we wait less for 2
	//and not at all for 3
	//then we wait a little bit before reading the sensor
	//this give the sensor time to power up
	ds_mux_data[sen].nextTm=sys_time+
	  ds_mux_data[sen].attached[(int)ds_mux_data[sen].channel];
	//lcd_int(ds_mux_data[sen].channel+1);

	break;
      } else {
	//schedule next low pulse
	ds_mux_data[sen].action=ds_mux_pulse_low;
	ds_mux_data[sen].nextTm=sys_time+DS_MUX_PULSE_TM_MS;
      }      
      break;
    default:
      //bad
    }

  }
}//endof ds_mux_handler

#endif //CONF_DSENSOR_MUX


void ds_callback(void) {
    int channel;
    int mask = 1<<4;
    for (channel = 0, mask = 1<<4; channel < 3; channel++, mask+=mask) {
	if (!(ds_dirty & mask))
	    continue;
	ds_dirty &= ~mask;
#ifdef CONF_DSENSOR_ROTATION
	if (ds_rotation & (1 << channel))
	    ds_rotation_handler(channel);
#endif
#ifdef CONF_DSENSOR_MUX
	if (ds_mux & (1 << channel))
	    ds_mux_handler(channel);
#endif
#ifdef CONF_DSENSOR_EDGECOUNT
	if (ds_edgecount & (1 << channel))
	    ds_edgecount_handler(channel);
#endif
    }
}


//! sensor A/D conversion IRQ handler
//
extern void ds_interrupt(void);
#ifndef DOXYGEN_SHOULD_SKIP_THIS
__asm__("\n\
.text\n\
.align 1\n\
_ds_interrupt:\n\
       ; r6 saved by ROM\n\
\n\
        mov.b @_ds_channel,r6l	; r6l = current channel\n\
\n\
	mov.b @_ds_sensorbits,r6h	; r6h = activation bitmask\n\
	btst r6l,r6h			; activate output?\n\
	beq ds_nounset\n\
	bset r6l,@_PORT6:8	; activate output of last port scanned\n\
ds_nounset:\n\
	or.b #4, r6l\n\
	btst r6l,r6h			; process handler for sensor?\n\
	beq ds_nohandler\n\
        mov.b @_ds_dirty, r6h\n\
        bset r6l,r6h\n\
        mov.b r6h, @_ds_dirty\n\
ds_nohandler:\n\
	inc r6l			; next channel\n\
	and #0x03,r6l		; limit to 0-3\n\
	mov.b r6l, @_ds_channel	; save channel\n\
	mov.b @_AD_CSR:8,r6h		; r6h = A/D CSR\n\
	and.b #0x7c,r6h		; reset interrupt flag and channel num\n\
	or.b  r6l,r6h		; scan next channel\n\
	mov.b r6h,@_AD_CSR:8		; put r6h back on A/D CSR\n"
#ifdef CONF_DSENSOR_FAST
	"\n\
        inc r6l\n\
        cmp #3, r6l\n\
        bhi ds_finished         ; we polled all 4 channels\n\
        beq 1f                  ; no need to deactivate channel #3 (battery)\n\
        bclr r6l,@_PORT6:8      ; deactivate next channel\n\
1:\n\
        bset #5,@_AD_CSR:8      ; start polling this channel \n\
ds_rts:\n\
	rts\n"
#endif
"ds_finished:\n\
        mov.b @_ds_dirty, r6l\n\
        beq ds_rts\n\
        push r0\n"
        enqueue_handler_irq(ds_handler)
"	jmp   @_irq_return\n"
#ifndef CONF_DSENSOR_FAST
"ds_rts:\n\
	rts\n"
#endif
	);
#endif // DOXYGEN_SHOULD_SKIP_THIS


//! initialize sensor a/d conversion
/*! all sensors set to passive mode
    rotation tracking disabled
*/
void ds_init(void) {
    rom_port6_ddr|=DS_ALL_ACTIVE;         // notify ROM we are using
    PORT6_DDR     =rom_port6_ddr;         // PORT6 bit 0..2 as outputs
    
    ds_sensorbits= 0;                      // all sensors passive
    ds_channel   = 3;			// start on channel 3
    
#ifdef CONF_DSENSOR_ROTATION
    ds_rotation  =0;                      // rotation tracking disabled
#endif
    
#ifdef CONF_DSENSOR_MUX
    ds_mux=0;                             // muxing disabled
#endif

#ifdef CONF_DSENSOR_EDGECOUNT
  ds_edgecount = 0;			// edge counting disabled
#endif
    
    ad_vector=&ds_interrupt;		// setup IRQ handler
    AD_CR &=~ADCR_EXTERN;
    AD_CSR =ADCSR_TIME_266 | ADCSR_GROUP_0 | ds_channel  |
	ADCSR_ENABLE_IRQ | ADCSR_START;
}


//! shutdown sensor a/d conversion
/*! all sensors set to passive mode
*/
void ds_shutdown(void) {

  AD_CSR=0x00;
  PORT6        &=DS_ALL_PASSIVE;
  rom_port6_ddr&=DS_ALL_PASSIVE;
  PORT6_DDR     =rom_port6_ddr;
}

#endif  // CONF_DSENSOR
