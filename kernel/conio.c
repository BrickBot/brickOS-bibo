/*! \file   conio.c
    \brief  Implementation: console input / output
    \author Markus L. Noga <markus@noga.de>

    \warning If CONF_LCD_REFRESH is set in config.h, the kernel will refresh
    the display automatically every 100ms.
    Otherwise, display updates are realized exclusively by
    lcd_refresh().

    \par Display positions
    Digit display positions are denumerated from right to left, starting
    with 0 for the digit right to the running man. Digit 5 is only partially
    present on the RCXs display.

    \par Native segment masks
    In these bitmasks, bit 0 toggles the middle segment. Bit 1 toggles the 
    top right segment, and the remaining segments are denumerated
    counterclockwise. The dot isn't encoded because it is desirable
    to seperate its positioning from the number display code.
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
 */

#include <sys/program.h>

#include <conio.h>
#include <mem.h>

#ifdef CONF_CONIO

///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////////////

//! hex display codes
//
const char hex_display_codes[] =
{
   CHAR_0,			// 0
   CHAR_1,			// 1
   CHAR_2,			// 2
   CHAR_3,			// 3
   CHAR_4,			// 4
   CHAR_5,			// 5
   CHAR_6,			// 6
   CHAR_7,			// 7
   CHAR_8,			// 8
   CHAR_9,			// 9
   CHAR_A,			// A 
   CHAR_b,			// b 
   CHAR_c,			// c
   CHAR_d,			// d 
   CHAR_E,			// E
   CHAR_F,			// F
};

#ifdef CONF_ASCII

//! ASCII display codes
/*! This is a 7-bit ASCII table only.
 */
const char ascii_display_codes[] =
{
  0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,	// non-printables
   0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,	// -> underscore
   0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,	// except 0x00.
   0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,

   CHAR_SPACE,			// 32 ' '
   CHAR_EXCLAMATION,	// 33 ! 1
   CHAR_DOUBLE_QUOTE,	// 34 "
   CHAR_POUND,			// 35 #
   CHAR_DOLLAR,			// 36 $ 5 S Z
   CHAR_PERCENT,		// 37 % /
   CHAR_AMPERSAND,		// 38 & 6
   CHAR_SINGLE_QUOTE,	// 39 '  alt: ` 
   CHAR_L_PAREN,		// 40 ( C [ {
   CHAR_R_PAREN,		// 41 ) ] }
   CHAR_ASTERISK,		// 42 * H K X
   CHAR_PLUS,			// 43 +  alt: 0x19 (worse)
   CHAR_COMMA,			// 44 , .  alt: 0x40
   CHAR_MINUS,			// 45 -
   CHAR_PERIOD,			// 46 . alt: 0x40
   CHAR_FORWARD_SLASH,	// 47 /

   CHAR_0,				// 48 0 0
   CHAR_1,				// 49 1
   CHAR_2,				// 50 2
   CHAR_3,				// 51 3
   CHAR_4,				// 52 4
   CHAR_5,				// 53 5
   CHAR_6,				// 54 6
   CHAR_7,				// 55 7
   CHAR_8,				// 56 8
   CHAR_9,				// 57 9

   CHAR_COLON,			// 58 : ; = alt:0x5 (worse)
   CHAR_SEMICOLON,		// 59 ;
   CHAR_LESS_THAN,		// 60 < c alt:0xd (worse)       
   CHAR_EQUAL,			// 61 =
   CHAR_GREATER_THAN,	// 62 >   alt: 0x7 (worse)
   CHAR_QUESTION,		// 63 ?
   CHAR_AT,				// 64 @ alt: e

   CHAR_A,				// 65 A
   CHAR_B,				// 66 b 
   CHAR_C,				// 67 C
   CHAR_D,				// 68 d
   CHAR_E,				// 69 E
   CHAR_F,				// 70 F
   CHAR_G,				// 71 G
   CHAR_H,				// 72 H
   CHAR_I,				// 73 I 1
   CHAR_J,				// 74 J
   CHAR_K,				// 75 K
   CHAR_L,				// 76 L
   CHAR_M,				// 77 M N
   CHAR_N,				// 78 N
   CHAR_O,				// 79 O alt: o
   CHAR_P,				// 80 P
   CHAR_Q,				// 81 Q 
   CHAR_R,				// 82 r
   CHAR_S,				// 83 S
   CHAR_T,				// 84 T alt: t
   CHAR_U,				// 85 U V W
   CHAR_V,				// 86 V
   CHAR_W,				// 87 W
   CHAR_X,				// 88 X
   CHAR_Y,				// 89 Y
   CHAR_Z,				// 90 Z

   CHAR_L_BRACKET,		// 91 [
   CHAR_BACK_SLASH,		// 92 '\\'
   CHAR_R_BRACKET,		// 93 ]
   CHAR_CARET,			// 94 ^ ~
   CHAR_UNDERSCORE,		// 95 _
   CHAR_BACKQUOTE,		// 96 ` alt: '

   CHAR_a,				// 97 A R
   CHAR_b,				// 98 b 
   CHAR_c,				// 99 c
   CHAR_d,				// 100 d 
   CHAR_e,				// 101 E
   CHAR_f,				// 102 F
   CHAR_g,				// 103 G
   CHAR_h,				// 104 h
   CHAR_i,				// 105 I 1
   CHAR_j,				// 106 J alt 0x60 (worse)
   CHAR_k,				// 107 K alt h (worse?)
   CHAR_l,				// 108 L
   CHAR_m,				// 109 m n
   CHAR_n,				// 110 n
   CHAR_o,				// 111 o
   CHAR_p,				// 112 P
   CHAR_q,				// 113 q        
   CHAR_r,				// 114 r
   CHAR_s,				// 115 S
   CHAR_t,				// 116 t
   CHAR_u,				// 117 u v w
   CHAR_v,				// 118 v
   CHAR_w,				// 119 w
   CHAR_x,				// 120 X
   CHAR_y,				// 121 Y
   CHAR_z,				// 122 Z

   CHAR_L_BRACE,		// 123 {
   CHAR_VERTICAL_BAR,	// 124 | (left) alt: 1 (worse)
   CHAR_R_BRACE,		// 125 }
   CHAR_TILDE,			// 126 ~
   0x00					// 127 "" 127 empty
};

#endif // CONF_ASCII

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

#endif // CONF_CONIO

//! uncalibrated delay loop
/*! \param ms approximate time in ms
 */
extern void delay(unsigned ms);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
__asm__("\n\
.text\n\
.align 1\n\
.global _delay\n\
\n\
_delay:\n\
1:	mov.w #614, r1\n\
2:	subs  #1, r1\n\
	mov.w r1, r1\n\
	bne   2b\n\
	subs  #1, r0\n\
	mov.w r0, r0\n\
	bne   1b\n\
	rts\n");
#endif

#ifdef CONF_CONIO

//! clear the user portion of screen
void cls() {
#ifdef CONF_ASCII
  cputs("");
#else
  unsigned char i = 0;
  for (i = 5; i >= 1; i--) {
    cputc_native(0, i);
  }
#endif

  // reset the program number display
  show_program_num();
}

//! display native mode segment mask
/*! \param mask the segment mask.
   \param pos  the desired display position.

   this is a dispatcher for the fixed position routines.
 */
void cputc_native(char mask, int pos)
{
  switch (pos) {
  case 0:
    cputc_native_0(mask);
    break;
  case 1:
    cputc_native_1(mask);
    break;
  case 2:
    cputc_native_2(mask);
    break;
  case 3:
    cputc_native_3(mask);
    break;
  case 4:
    cputc_native_4(mask);
    break;
  case 5:
    cputc_native_5(mask);
  }
}

//! display native mode segment mask at display position 0
/*! \param mask the mask to display
 */
void cputc_native_0(char mask)
{
  // gcc is stupid
  // doesn't re-use constant values in registers.
  // re-ordered stores to help him.

  bit_load(mask, 0x2);
  dlcd_store(LCD_0_TOP);
  bit_load(mask, 0x0);
  dlcd_store(LCD_0_MID);
  bit_load(mask, 0x5);
  dlcd_store(LCD_0_BOT);
  bit_load(mask, 0x1);
  dlcd_store(LCD_0_TOPR);
  bit_load(mask, 0x6);
  dlcd_store(LCD_0_BOTR);
  bit_load(mask, 0x3);
  dlcd_store(LCD_0_TOPL);
  bit_load(mask, 0x4);
  dlcd_store(LCD_0_BOTL);
}

//! display native mode segment mask at display position 1
/*! \param mask the mask to display
 */
void cputc_native_1(char mask)
{
  bit_load(mask, 0x2);
  dlcd_store(LCD_1_TOP);
  bit_load(mask, 0x0);
  dlcd_store(LCD_1_MID);
  bit_load(mask, 0x5);
  dlcd_store(LCD_1_BOT);
  bit_load(mask, 0x1);
  dlcd_store(LCD_1_TOPR);
  bit_load(mask, 0x6);
  dlcd_store(LCD_1_BOTR);
  bit_load(mask, 0x3);
  dlcd_store(LCD_1_TOPL);
  bit_load(mask, 0x4);
  dlcd_store(LCD_1_BOTL);
}

//! display native mode segment mask at display position 2
/*! \param mask the mask to display
 */
void cputc_native_2(char mask)
{
  bit_load(mask, 0x2);
  dlcd_store(LCD_2_TOP);
  bit_load(mask, 0x0);
  dlcd_store(LCD_2_MID);
  bit_load(mask, 0x5);
  dlcd_store(LCD_2_BOT);
  dlcd_hide(LCD_2_DOT);
  bit_load(mask, 0x1);
  dlcd_store(LCD_2_TOPR);
  bit_load(mask, 0x6);
  dlcd_store(LCD_2_BOTR);
  bit_load(mask, 0x3);
  dlcd_store(LCD_2_TOPL);
  bit_load(mask, 0x4);
  dlcd_store(LCD_2_BOTL);
}

//! display native mode segment mask at display position 3
/*! \param mask the mask to display
 */
void cputc_native_3(char mask)
{
  dlcd_hide(LCD_3_DOT);
  bit_load(mask, 0x2);
  dlcd_store(LCD_3_TOP);
  bit_load(mask, 0x0);
  dlcd_store(LCD_3_MID);
  bit_load(mask, 0x5);
  dlcd_store(LCD_3_BOT);
  bit_load(mask, 0x1);
  dlcd_store(LCD_3_TOPR);
  bit_load(mask, 0x6);
  dlcd_store(LCD_3_BOTR);
  bit_load(mask, 0x3);
  dlcd_store(LCD_3_TOPL);
  bit_load(mask, 0x4);
  dlcd_store(LCD_3_BOTL);
}

//! display native mode segment mask at display position 4
/*! \param mask the mask to display
 */
void cputc_native_4(char mask)
{
  dlcd_hide(LCD_4_DOT);
  bit_load(mask, 0x2);
  dlcd_store(LCD_4_TOP);
  bit_load(mask, 0x0);
  dlcd_store(LCD_4_MID);
  bit_load(mask, 0x5);
  dlcd_store(LCD_4_BOT);
  bit_load(mask, 0x1);
  dlcd_store(LCD_4_TOPR);
  bit_load(mask, 0x6);
  dlcd_store(LCD_4_BOTR);
  bit_load(mask, 0x3);
  dlcd_store(LCD_4_TOPL);
  bit_load(mask, 0x4);
  dlcd_store(LCD_4_BOTL);
}

//! display native mode segment mask at display position 5
/*! \param mask the mask to display. only the middle segment ('-')
   is present on the display.
 */
void cputc_native_5(char mask)
{
  bit_load(mask, 0x0);
  dlcd_store(LCD_5_MID);
}

//! display native mode segment masks in the 4 main user display positions
/*! \param mask_left mask for the left-most position
 * \param mask_lcenter mask for the left-center position
 * \param mask_rcenter mask for the right-center position
 * \param mask_right mask for the right-most position
 */
void cputc_native_user(char mask_left, char mask_lcenter, char mask_rcenter, char mask_right)
{
  cputc_native_4(mask_left);
  cputc_native_3(mask_lcenter);
  cputc_native_2(mask_rcenter);
  cputc_native_1(mask_right);
}

//! display a hexword in the four leftmost positions.
/*! \param word the hexword

   position 0 is unaffected by this call.
 */
void cputw(unsigned word)
{
  int i;

  cputc_native(0, 5);
  for (i = 1; i <= 4; i++) {
    cputc_hex(word & 0x0f, i);
    word >>= 4;
  }

#if !defined(CONF_LCD_REFRESH)
  lcd_refresh();
#endif
}

#ifdef CONF_ASCII

//! display an ASCIIZ string
/*! \param s the string

   Only the first 5 characters will be displayed, unless the first character
   is '-', in which case an additional character will be displayed.
   If there are fewer characters than display positions, the remaining
   "user" display positions will be cleared.
 */
void cputs(const char *s)
{
  int i;

  // Determine if the first character is a dash
  if ('-' == (*s)) {
    cputc(*(s++), 5);
  } else {
    cputc_native(0, 5);
  }
  for (i = 4; (*s) && (i >= 0);)
    cputc(*(s++), i--);
  while (i >= 1)
    cputc_native(0, i--);

#if !defined(CONF_LCD_REFRESH)
  lcd_refresh();
#endif
}

#endif // CONF_ASCII

#endif // CONF_CONIO
