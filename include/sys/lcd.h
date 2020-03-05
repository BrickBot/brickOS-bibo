/*! \file   include/sys/lcd.h
    \brief  Internal Interface: LCD control and constants
    \author Eddie C. Dost <ecd@skynet.be>
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

#ifndef __sys_lcd_h__
#define __sys_lcd_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>
#include <rom/lcd.h>

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

// Offsets and size of the LCD shadow buffer used to speed up
// lcd_refresh()
//
#define LCD_SHORT_CMD   0
#define LCD_LONG_CMD    3
#define LCD_DATA_OFFSET 5
#define LCD_DATA_SIZE   9

// PCF8566 LCD driver commands
//
#define LCD_MODE_SET    0x40
#define LCD_ENABLE      0x08
#define LCD_DISABLE     0x00

// LCD i2c bus constants
//
#define LCD_DEV_ID      0x7c

#define I2C_WRITE       0
#define I2C_READ        1

#define SCL             5
#define SDA             6


///////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////

extern unsigned char display_memory[]; //!< LCD display data buffer

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

//! initialize LCD driver
extern void lcd_init(void);

//! power on LCD display, also called from lcd_init()
extern void lcd_power_on(void);

//! power off LCD display
extern void lcd_power_off(void);

//! show LCD display contents to the world
/*! display updates are realized exclusively by calling this function.
 */
extern void lcd_refresh(void);

#ifdef  __cplusplus
}
#endif

#endif	// __sys_lcd_h__
