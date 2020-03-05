/*! \file   include/conio.h
    \brief  Interface: console input / output
    \author Markus L. Noga <markus@noga.de>

    \warning If CONF_LCD_REFRESH is set in config.h, the kernel will
    refresh the display automatically every 100ms.
    Otherwise, display updates are realized exclusively by
    lcd_refresh().

    \par Display positions
    Digit display positions are denumerated from right to left,
    starting with 0 for the digit to the right of the running man. 
	
	\par
    LCD Postions:  5 4 3 2 1 {man} 0

	\par
    NOTE: Position 5 is only partially present on the LCD display.

    \par Native segment masks
    In these bitmasks, bit 0 toggles the middle segment. Bit 1 toggles
    the top right segment, and the remaining segments are denumerated
    counterclockwise. The dot isn't encoded because it is desirable
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

#ifndef __conio_h_
#define __conio_h_

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_CONIO

#include <sys/lcd.h>
#include <dlcd.h>
#include <dkey.h>

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

// Mask defines for ASCII characters
#define CHAR_SPACE          0x00			// 32 ' '
#define CHAR_EXCLAMATION    0x42			// 33 ! 1
#define CHAR_DOUBLE_QUOTE   0x0a			// 34 "
#define CHAR_POUND          0x7b			// 35 #
#define CHAR_DOLLAR         0x6d			// 36 $ 5 S Z
#define CHAR_PERCENT        0x13			// 37 % /
#define CHAR_AMPERSAND      0x7d			// 38 & 6
#define CHAR_SINGLE_QUOTE   0x08			// 39 '  alt: ` 
#define CHAR_L_PAREN        0x3c			// 40 ( C [ {
#define CHAR_R_PAREN        0x66			// 41 ) ] }
#define CHAR_ASTERISK       0x5b			// 42 * H K X
#define CHAR_PLUS           0x43			// 43 +  alt: 0x19 (worse)
#define CHAR_COMMA          0x10			// 44 , .  alt: 0x40
#define CHAR_MINUS          0x01			// 45 -
#define CHAR_DASH           0x01			// 45 -
#define CHAR_PERIOD         0x10			// 46 . alt: 0x40
#define CHAR_FORWARD_SLASH  0x13			// 47 /

#define CHAR_0              0x7e			// 48 0 0
#define CHAR_1              0x42			// 49 1
#define CHAR_2              0x37			// 50 2
#define CHAR_3              0x67			// 51 3
#define CHAR_4              0x4b			// 52 4
#define CHAR_5              0x6d			// 53 5
#define CHAR_6              0x7d			// 54 6
#define CHAR_7              0x46			// 55 7
#define CHAR_8              0x7f			// 56 8
#define CHAR_9              0x6f			// 57 9

#define CHAR_COLON          0x21			// 58 : ; = alt:0x5 (worse)
#define CHAR_SEMICOLON      0x21			// 59 ;
#define CHAR_LESS_THAN      0x31			// 60 < c alt:0xd (worse)       
#define CHAR_EQUAL          0x21			// 61 =
#define CHAR_GREATER_THAN   0x61			// 62 >   alt: 0x7 (worse)
#define CHAR_QUESTION       0x17			// 63 ?
#define CHAR_AT             0x3f			// 64 @ alt: e

#define CHAR_A              0x5f			// 65 A
#define CHAR_B              0x79			// 66 b 
#define CHAR_C              0x3c			// 67 C
#define CHAR_D              0x73			// 68 d
#define CHAR_E              0x3d			// 69 E
#define CHAR_F              0x1d			// 70 F
#define CHAR_G              0x7c			// 71 G
#define CHAR_H              0x5b			// 72 H
#define CHAR_I              0x42			// 73 I 1
#define CHAR_J              0x62			// 74 J
#define CHAR_K              0x5b			// 75 K
#define CHAR_L              0x38			// 76 L
#define CHAR_M              0x5e			// 77 M N
#define CHAR_N              0x5e			// 78 N
#define CHAR_O              0x7e			// 79 O alt: o
#define CHAR_P              0x1f			// 80 P
#define CHAR_Q              0x4f			// 81 Q 
#define CHAR_R              0x11			// 82 r
#define CHAR_S              0x6d			// 83 S
#define CHAR_T              0x46			// 84 T alt: t
#define CHAR_U              0x7a			// 85 U V W
#define CHAR_V              0x7a			// 86 V
#define CHAR_W              0x7a			// 87 W
#define CHAR_X              0x5b			// 88 X
#define CHAR_Y              0x6b			// 89 Y
#define CHAR_Z              0x37			// 90 Z

#define CHAR_L_BRACKET      0x3c			// 91 [
#define CHAR_BACK_SLASH     0x49			// 92 '\\'
#define CHAR_R_BRACKET      0x66			// 93 ]
#define CHAR_CARET          0x0e			// 94 ^ ~
#define CHAR_UNDERSCORE     0x20			// 95 _
#define CHAR_BACKQUOTE      0x02			// 96 ` alt: '

#define CHAR_a              0x5f			// 97 A R
#define CHAR_b              0x79			// 98 b 
#define CHAR_c              0x31			// 99 c
#define CHAR_d              0x73			// 100 d 
#define CHAR_e              0x3d			// 101 E
#define CHAR_f              0x1d			// 102 F
#define CHAR_g              0x7c			// 103 G
#define CHAR_h              0x59			// 104 h
#define CHAR_i              0x42			// 105 I 1
#define CHAR_j              0x62			// 106 J alt 0x60 (worse)
#define CHAR_k              0x5b			// 107 K alt h (worse?)
#define CHAR_l              0x38			// 108 L
#define CHAR_m              0x51			// 109 m n
#define CHAR_n              0x51			// 110 n
#define CHAR_o              0x71			// 111 o
#define CHAR_p              0x1f			// 112 P
#define CHAR_q              0x4f			// 113 q        
#define CHAR_r              0x11			// 114 r
#define CHAR_s              0x6d			// 115 S
#define CHAR_t              0x39			// 116 t
#define CHAR_u              0x70			// 117 u v w
#define CHAR_v              0x70			// 118 v
#define CHAR_w              0x70			// 119 w
#define CHAR_x              0x5b			// 120 X
#define CHAR_y              0x6b			// 121 Y
#define CHAR_z              0x37			// 122 Z

#define CHAR_L_BRACE        0x3c			// 123 {
#define CHAR_VERTICAL_BAR   0x18			// 124 | (left) alt: 1 (worse)
#define CHAR_R_BRACE        0x66			// 125 }
#define CHAR_TILDE          0x0e			// 126 ~

// Miscellaneous non-ASCII characters
#define CHAR_PARALLEL       0x5a			// Double vertical bars
#define CHAR_EQUIVALENT     0x26			// Triple horizontal bars

#ifndef DOXYGEN_SHOULD_SKIP_INTERNALS

//! Table: list of native patterns, one for each HEX character
/*! \index HEX char value (0-9, a-f)
*/
extern const char hex_display_codes[];

#ifdef CONF_ASCII

//! Table: list of native patterns, one for each ASCII character
/*! \index ASCII char value (least significant 7 bits ONLY)
*/
extern const char ascii_display_codes[];

#endif // CONF_ASCII

#endif // DOXYGEN_SHOULD_SKIP_INTERNALS

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

#endif // CONF_CONIO

//! delay approximately ms mSec
/*! \todo why is delay() in this file?
*/
extern void delay(unsigned ms);

#ifdef CONF_CONIO

//
// display native mode segment mask at fixed display position
//
// encoding: middle=1, topr=2, top=4, ... (counterclockwise)
// dot not included because not reliably present.
//
//! write bit-pattern for segments at position 0 of LCD
extern void cputc_native_0(char mask);
//! write bit-pattern for segments at position 1 of LCD
extern void cputc_native_1(char mask);
//! write bit-pattern for segments at position 2 of LCD
extern void cputc_native_2(char mask);
//! write bit-pattern for segments at position 3 of LCD
extern void cputc_native_3(char mask);
//! write bit-pattern for segments at position 4 of LCD
extern void cputc_native_4(char mask);
//! write bit-pattern for segments at position 5 of LCD
extern void cputc_native_5(char mask);
//! write bit-patter for segments in positions 4 through 1 of the LCD (the main user portion)
extern void cputc_native_user(char mask_left, char mask_lcenter, char mask_rcenter, char mask_right);

//! Set/Clear individual segments at specified position of LCD
/*! (this is essentially a dispatcher for cputc_native_[0-5] functions)
 *  \param mask the segment pattern to be displayed
 *  \param pos the location at which to display the segment pattern
 *  \return Nothing
 */
extern void cputc_native(char mask, int pos);

//! write HEX digit to position 0 of LCD
extern inline void cputc_hex_0(unsigned nibble)
{
  cputc_native_0(hex_display_codes[(nibble) & 0x0f]);
}
//! write HEX digit to position 1 of LCD
extern inline void cputc_hex_1(unsigned nibble)
{
  cputc_native_1(hex_display_codes[(nibble) & 0x0f]);
}
//! write HEX digit to position 2 of LCD
extern inline void cputc_hex_2(unsigned nibble)
{
  cputc_native_2(hex_display_codes[(nibble) & 0x0f]);
}
//! write HEX digit to position 3 of LCD
extern inline void cputc_hex_3(unsigned nibble)
{
  cputc_native_3(hex_display_codes[(nibble) & 0x0f]);
}
//! write HEX digit to position 4 of LCD
extern inline void cputc_hex_4(unsigned nibble)
{
  cputc_native_4(hex_display_codes[(nibble) & 0x0f]);
}
//! write HEX digit to position 5 of LCD
extern inline void cputc_hex_5(unsigned nibble)
{
  cputc_native_5(hex_display_codes[(nibble) & 0x0f]);
}

//! Write HEX digit to specified position of LCD
/*! (this is essentially a dispatcher for cputc_hex_[0-5] functions)
 *  \param c the HEX digit to be displayed
 *  \param pos the location at which to display the HEX digit
 *  \return Nothing
 */
extern inline void cputc_hex(char c, int pos)
{
  cputc_native(hex_display_codes[(c) & 0x7f], pos);
}

//! Write a HEX word to LCD
extern void cputw(unsigned word);

//! clear user portion of LCD
extern void cls();

#ifdef CONF_ASCII
//! write ASCII char to position 0 of LCD
extern inline void cputc_0(unsigned c)
{
  cputc_native_0(ascii_display_codes[(c) & 0x7f]);
}
//! write ASCII char to position 1 of LCD
extern inline void cputc_1(unsigned c)
{
  cputc_native_1(ascii_display_codes[(c) & 0x7f]);
}
//! write ASCII char to position 2 of LCD
extern inline void cputc_2(unsigned c)
{
  cputc_native_2(ascii_display_codes[(c) & 0x7f]);
}
//! write ASCII char to position 3 of LCD
extern inline void cputc_3(unsigned c)
{
  cputc_native_3(ascii_display_codes[(c) & 0x7f]);
}
//! write ASCII char to position 4 of LCD
extern inline void cputc_4(unsigned c)
{
  cputc_native_4(ascii_display_codes[(c) & 0x7f]);
}
//! write ASCII char to position 5 of LCD
extern inline void cputc_5(unsigned c)
{
  cputc_native_5(ascii_display_codes[(c) & 0x7f]);
}

//! Write ASCII character to specified position of LCD
/*! (this is essentially a dispatcher for cputc_[0-5] functions)
 *  \param c the ASCII char to be displayed
 *  \param pos the location at which to display the ASCII char
 *  \return Nothing
 */
extern inline void cputc(char c, int pos)
{
  cputc_native(ascii_display_codes[(c) & 0x7f], pos);
}

//
//! Write string s to LCD (Only first 5 chars)
//
extern void cputs(const char *s);

#endif // CONF_ASCII

#else

#define cls() lcd_clear()

#endif // CONF_CONIO

#ifdef  __cplusplus
}
#endif

#endif // __conio_h__
