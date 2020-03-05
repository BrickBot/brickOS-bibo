/*! \file   include/rom/lcd.h
    \brief  ROM Interface: RCX LCD control
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

#ifndef __rom_lcd_h__
#define __rom_lcd_h__

#ifdef  __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

//! LCD segment codes
/*! these are not to be confused with the codes defined in direct-lcd.h

   circle and dot codes cycle. cycle state is preserved on powerdown.

   each dot code should be invoked six times before using the other.
   mixing them will result in strange behaviour.

 */
typedef enum {
  man_stand = 0x3006,
  man_run   = 0x3007,

  s1_select = 0x3008,
  s1_active = 0x3009,

  s2_select = 0x300A,
  s2_active = 0x300B,

  s3_select = 0x300C,
  s3_active = 0x300D,

  a_select  = 0x300E,
  a_left    = 0x300F,
  a_right   = 0x3010,

  b_select  = 0x3011,
  b_left    = 0x3012,
  b_right   = 0x3013,

  c_select  = 0x3014,
  c_left    = 0x3015,
  c_right   = 0x3016,

  unknown_1 = 0x3017,		//!< seemingly without effect. cycle reset?

  circle    = 0x3018,		//!< 0..3 quarters: add one. 4 quarters: reset

  dot       = 0x3019,		//!< 0..4 dots: add a dot. 5 dots: reset
      	      	      	        // dots are added on the right.
  dot_inv   = 0x301A,		//!< 0 dots: show 5. 1..4 dots: subtract one
      	      	      	        // dots are removed on the left

  battery_x = 0x301B,

  ir_half   = 0x301C, 	    	//! the IR display values are mutually exclusive.
  ir_full   = 0x301D,   	//! the IR display values are mutually exclusive.

  everything= 0x3020

} lcd_segment;

//! LCD number display styles
/*! note: signed and unsigned are taken by the C programming language
 */
typedef enum {
  digit  = 0x3017,		//!< single digit on the right
      	      	      	        // works only with comma_style digit

  sign   = 0x3001,		//!< signed, no leading zeros
  unsign = 0x301F		//!< unsigned, 0 displayed as 0000

} lcd_number_style;

//! LCD comma display styles
/*
 */
typedef enum {
  digit_comma = 0x0000,		//!< single digit on the right
      	      	      	        // works only with number_style digit

  e0  = 0x3002,			//!< whole
  e_1 = 0x3003,	  	      	//!< 10ths
  e_2 = 0x3004,		      	//!< 100ths
  e_3 = 0x3005,		      	//!< 1000ths, problematic with negatives

} lcd_comma_style;		// lcd display comma style

//! display an integer in decimal
#define lcd_int(i)	lcd_number(i,sign,e0)

//! display an unsigned value in decimal
#define lcd_unsigned(u)	lcd_number(u,unsign,e0)

//! display a clock.
/*! passing an argument of 1015 will display 10.15
 */
#define lcd_clock(t)	lcd_number(t,unsign,e_2)

//! display a single digit right of the man symbol
#define lcd_digit(d)	lcd_number(d,digit,digit_comma)

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

/*! g++ warns about __asm__ __volatile__. we define the problem away.
 *   the CXX symbol is predefined in the Makefile
 *   \todo (when move to g++ 3.x) determine if this is still the case 
 *   and fix this if not...
*/
#ifdef CXX
#define ASMVOLATILE
#else
#define ASMVOLATILE __volatile__
#endif

//! show LCD segment
/*! \param segment segment to show
 */
extern inline void lcd_show(lcd_segment segment)
{
  __asm__ ASMVOLATILE(
		       "push r6\n"
		       "mov.w %0,r6\n"
		       "jsr @lcd_show\n"
		       "pop r6\n"
		       :	// outputs
		       :"r"(segment)	// inputs
  );
}

//! hide LCD segment
/*! \param segment segment to hide
 */
extern inline void lcd_hide(lcd_segment segment)
{
  __asm__ ASMVOLATILE(
		       "push r6\n"
		       "mov.w %0,r6\n"
		       "jsr @lcd_hide\n"
		       "pop r6\n"
		       :	// outputs
		       :"r"(segment)	// inputs
  );
}

//! show number on LCD display
/*! \param i the number
   \param n a number style
   \param c a comma style
 */
extern inline void lcd_number(int i, lcd_number_style n, lcd_comma_style c)
{
  __asm__ ASMVOLATILE(
		       "push r6\n"
		       "push %2         ; comma_style\n"
		       "push %0         ; number       -> stack\n"
		       "mov.w %1,r6     ; number_style -> r6\n"
		       "jsr @lcd_number ; call ROM\n"
		       "adds #2,r7      ; clear stack\n"
		       "adds #2,r7\n"
		       "pop r6\n"
		       :	// outputs
		       : "r" (i), "g" (n), "r" (c));
}

//! clear LCD display
extern inline void lcd_clear(void)
{
  __asm__ ASMVOLATILE(
		       "push r6\n"
		       "jsr @lcd_clear\n"
		       "pop r6\n"
  );
}

#ifdef  __cplusplus
}
#endif

#endif // __rom_lcd_h__
