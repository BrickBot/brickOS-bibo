/*! \file boot/config.h
  \brief  kernel configuration file
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
 *
 * Taiichi #define'ed CONF_VIEW_BUTTON
 */

#ifndef __config_h__
#define __config_h__

// compilation environment
//
// #define CONF_RCX_COMPILER              //!< a special RCX compiler is used.

// core system services
//
#define CONF_TIME                       //!< system time
#define CONF_MM                         //!< memory management
#define CONF_TM                         //!< task management
#define CONF_SETJMP			//!< non local goto
#define CONF_SEMAPHORES                 //!< POSIX semaphores
#define CONF_PROGRAM                    //!< dynamic program loading support
#define CONF_VIS                        //!< generic visualization.
//#define CONF_ROM_MEMCPY                 //!< Use the ROM memcpy routine

// networking services
//
#define CONF_LNP                        //!< link networking protocol
// Can override with compile-time option
#if !defined(CONF_LNP_HOSTADDR)
#define CONF_LNP_HOSTADDR 0             //!< LNP host address
#endif

// Can override with compile-time option
#if !defined(CONF_LNP_STARTUP_BAUD_RATE)
#define CONF_LNP_STARTUP_BAUD_RATE B2400 //!< LNP baud rate
#endif
#define CONF_LNP_IR_CARRIER_ENABLED 1   //!< IR carrier state
#define CONF_LNP_TX_ECHOES 1        //!< TX echoes

// 16 nodes x 16 ports (affects size of lnp_addressing_handler[] table)
#define CONF_LNP_HOSTMASK 0xf0          //!< LNP host mask

// remote control services
//
#define CONF_RCX_PROTOCOL               //!< RCX protocol handler
#define CONF_LR_HANDLER                 //!< remote control keys handler service
#define CONF_RCX_MESSAGE                //!< standard firmware message service

// drivers
//
#define CONF_DKEY                       //!< debounced key driver
#define CONF_BATTERY_INDICATOR          //!< automatic update of lcd battery indicator
#define CONF_LCD_REFRESH                //!< automatic display updates
#define CONF_CONIO                      //!< console
//#define CONF_ASCII                      //!< ascii console
#define CONF_VIEW_BUTTON                //!< view button information display
#define CONF_DSOUND                     //!< direct sound
#define CONF_ON_OFF_SOUND               //!< sound on switch on/off
#define CONF_DMOTOR                     //!< direct motor
// #define CONF_DMOTOR_HOLD               //!< experimental: use hold mode PWM instead of coast mode.
#define CONF_DCC                        //!< Digital Command Control
#define CONF_DSENSOR                    //!< direct sensor
#define CONF_DSENSOR_FAST               //!< poll all sensors in 1 ms
#define CONF_DSENSOR_ROTATION           //!< rotation sensor
//#define CONF_DSENSOR_VELOCITY           //!< rotation sensor velocity
//#define CONF_DSENSOR_MUX                //!< sensor multiplexor
//#define CONF_DSENSOR_SWMUX              //!< techno-stuff swmux sensor
#define CONF_DSENSOR_EDGECOUNT          //!< edge counting for touch/light sensors
//#define CONF_POWERFUNCTIONS               //!< act as power functions remote control


// dependencies
//
#if defined(CONF_ASCII) && !defined(CONF_CONIO)
#error "ASCII needs console IO."
#endif

#if defined(CONF_DKEY) && !defined(CONF_TIME)
#error "Key debouncing needs system time."
#endif

#if defined(CONF_TM) && !defined(CONF_TIME)
#error "Task management needs system time."
#endif

#if defined(CONF_TM) && !defined(CONF_MM)
#error "Task management needs memory management."
#endif

#if defined(CONF_LNP) && defined(CONF_TM) && !defined(CONF_SEMAPHORES)
#error "Tasksafe networking needs semaphores."
#endif

#if defined(CONF_RCX_PROTOCOL) && !defined(CONF_LNP)
#error "RCX protocol needs networking."
#endif

#if defined(CONF_LR_HANDLER) && !defined(CONF_RCX_PROTOCOL)
#error "Remote control handler needs remote control protocol."
#endif

#if defined(CONF_RCX_MESSAGE) && !defined(CONF_LNP)
#error "Standard firmware message needs networking."
#endif

#if defined(CONF_LR_HANDLER) && !defined(CONF_TM)
#error "Remote support needs task managment."
#endif

#if defined(CONF_PROGRAM) && (!defined(CONF_TM) || !defined(CONF_LNP) || !defined(CONF_DKEY) || !defined(CONF_CONIO))
#error "Program support needs task management, networking, key debouncing, and console IO."
#endif

#if defined(CONF_DCC) && !defined(CONF_DMOTOR)
#error "DCC needs motor."
#endif

#if defined(CONF_DSENSOR_ROTATION) && !defined(CONF_DSENSOR)
#error "Rotation sensor needs general sensor code."
#endif

#if defined(CONF_DSENSOR_VELOCITY) && !defined(CONF_DSENSOR_ROTATION)
#error "Velocity sensor needs rotation sensor code."
#endif

//! macro used to put some legOS function in high memory area.
#define __TEXT_INIT__  __attribute__ ((__section__ (".text.in")))
#define __TEXT_HI__  __attribute__ ((__section__ (".text.hi")))

#endif // __config_h__
