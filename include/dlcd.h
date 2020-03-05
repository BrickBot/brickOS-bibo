/*! \file   include/dlcd.h
    \brief  Interface: direct control of LCD display
    \author Markus L. Noga <markus@noga.de>

    \warning If CONF_LCD_REFRESH is set in config.h, the kernel will
    refresh the display automatically every 100ms.
    Otherwise, display updates are realized exclusively by
    lcd_refresh().
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

#ifndef __dlcd_h__
#define __dlcd_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <sys/bitops.h>

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

// LCD segment control byte and bit locations
// 0xNNNN,0xM => Mth bit (value 1<<M) of byte 0xNNNN
// overall memory range: 0xef43-0xef4b (9 bytes)
//

// the man
//
#define LCD_ARMS	0xef43,0x0
#define LCD_BODY	0xef43,0x1
#define LCD_1LEG	0xef43,0x2
#define LCD_2LEGS	0xef43,0x3

// the digits
// numbered right to left
// dots are associated with the digit to their left
// only digit 2,3,4 have dots
// digit 5 can only be used as a minus sign
//
#define LCD_0_TOP	0xef44,0x4
#define LCD_0_MID	0xef44,0x5
#define LCD_0_BOT	0xef44,0x7
#define LCD_0_TOPR	0xef45,0x5
#define LCD_0_BOTR	0xef45,0x7
#define LCD_0_TOPL	0xef47,0x5
#define LCD_0_BOTL	0xef47,0x7

#define LCD_1_TOP	0xef43,0x4
#define LCD_1_MID	0xef43,0x5
#define LCD_1_BOT	0xef43,0x7
#define LCD_1_TOPR	0xef47,0x1
#define LCD_1_BOTR	0xef47,0x3
#define LCD_1_TOPL	0xef48,0x5
#define LCD_1_BOTL	0xef48,0x7

#define LCD_2_TOP	0xef44,0x0
#define LCD_2_MID	0xef44,0x1
#define LCD_2_BOT	0xef44,0x3
#define LCD_2_TOPR	0xef48,0x1
#define LCD_2_BOTR	0xef48,0x3
#define LCD_2_TOPL	0xef49,0x5
#define LCD_2_BOTL	0xef49,0x7
#define LCD_2_DOT	0xef48,0x6

#define LCD_3_TOP	0xef46,0x0
#define LCD_3_MID	0xef46,0x1
#define LCD_3_BOT	0xef46,0x3
#define LCD_3_TOPR	0xef4b,0x1
#define LCD_3_BOTR	0xef4b,0x3
#define LCD_3_TOPL	0xef4b,0x5
#define LCD_3_BOTL	0xef4b,0x7
#define LCD_3_DOT	0xef49,0x6

#define LCD_4_TOP	0xef46,0x4
#define LCD_4_MID	0xef46,0x5
#define LCD_4_BOT	0xef46,0x7
#define LCD_4_TOPR	0xef4a,0x1
#define LCD_4_BOTR	0xef4a,0x3
#define LCD_4_TOPL	0xef4a,0x5
#define LCD_4_BOTL	0xef4a,0x7
#define LCD_4_DOT	0xef4b,0x6

#define LCD_5_MID	0xef4a,0x6

// the motor displays
//
#define LCD_A_SELECT	0xef4a,0x2
#define LCD_A_LEFT	0xef46,0x6
#define LCD_A_RIGHT	0xef46,0x2

#define LCD_B_SELECT	0xef43,0x6
#define LCD_B_LEFT	0xef44,0x2
#define LCD_B_RIGHT	0xef47,0x2

#define LCD_C_SELECT	0xef44,0x6
#define LCD_C_LEFT	0xef47,0x6
#define LCD_C_RIGHT	0xef45,0x6

// the sensor displays
//
#define LCD_S1_SELECT	0xef49,0x0
#define LCD_S1_ACTIVE	0xef49,0x1

#define LCD_S2_SELECT	0xef48,0x4
#define LCD_S2_ACTIVE	0xef48,0x0

#define LCD_S3_SELECT	0xef47,0x4
#define LCD_S3_ACTIVE	0xef45,0x4

// the quartered circle
// quarter nubering is counterclockwise
// starts at top right
//
#define LCD_CIRCLE_0	0xef45,0x0
#define LCD_CIRCLE_1	0xef45,0x1
#define LCD_CIRCLE_2	0xef45,0x3
#define LCD_CIRCLE_3	0xef45,0x2

// the dotted line
// dots numbered right to left
//
#define LCD_DOT_0	0xef49,0x4
#define LCD_DOT_1	0xef4b,0x0
#define LCD_DOT_2	0xef4b,0x4
#define LCD_DOT_3	0xef4a,0x0
#define LCD_DOT_4	0xef4a,0x4

// the IR display
//
#define LCD_IR_LOWER	0xef49,0x2
#define LCD_IR_UPPER	0xef49,0x3

// the crossed out battery
//
#define LCD_BATTERY_X	0xef47,0x0

// two empty bits
//
#define LCD_EMPTY_1	0xef48,0x2
#define LCD_EMPTY_2	0xef4b,0x2

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

//! set a segment directly in the LCD buffer
/*! \param a the segment to set
 */
#define dlcd_show(a)	bit_set(BYTE_OF(a),BIT_OF(a))

//! clear a segment directly in the LCD buffer
/*! \param a the segment to clear
 */
#define dlcd_hide(a)	bit_clear(BYTE_OF(a),BIT_OF(a))

//! store the carry flag to a segment directly in the LCD buffer
/*! \param a the segment to store to

   this is highly useful in combination with bit_load(mask,bit)
 */
#define dlcd_store(a)	bit_store(BYTE_OF(a),BIT_OF(a))

//! helper macros
//
#define BYTE_OF(a,b)	a
#define BIT_OF(a,b)	b

#ifdef  __cplusplus
}
#endif

#endif // __dlcd_h__
