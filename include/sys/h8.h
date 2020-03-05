/*! \file   include/sys/h8.h
    \brief  Internal Interface: H8/3297 processor registers
    \author Markus L. Noga <markus@noga.de>
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

#ifndef __sys_h8_h__
#define __sys_h8_h__

#ifdef  __cplusplus
extern "C" {
#endif

// 16 bit timer registers
//

//! 16-bit timer interrupt enable register
extern          unsigned char T_IER;

//! 16-bit timer control / status register
extern volatile unsigned char T_CSR;

//! 16-bit timer count register
extern volatile unsigned      T_CNT;

//! 16-bit timer output compare register A
extern          unsigned      T_OCRA;

//! 16-bit timer output compare register B
/* address shared with T_OCRA */
extern          unsigned      T_OCRB;

//! 16-bit timer control register
extern          unsigned char T_CR;

//! 16-bit timer output control register
extern          unsigned char T_OCR;


//! 16-bit timer input capture A register
extern volatile unsigned      T_ICRA;

//! 16-bit timer input capture B register
extern volatile unsigned      T_ICRB;

//! 16-bit timer input capture C register
extern volatile unsigned      T_ICRC;

//! 16-bit timer input capture D register
extern volatile unsigned      T_ICRD;

//
// TIER bitmasks
//
#define TIER_ENABLE_ICA		0x80	   // input capture IRQ enables
#define TIER_ENABLE_ICB		0x40
#define TIER_ENABLE_ICC		0x20
#define TIER_ENABLE_ICD		0x10
#define TIER_ENABLE_OCA		0x08	   // output compare IRQ enables
#define TIER_ENABLE_OCB		0x04
#define TIER_ENABLE_OF		0x02	   // overflow IRQ enable
#define TIER_RESERVED		0x01	   // always set.

//
// TCSR bitmasks
//
#define TCSR_ICA		0x80	   // input capture events
#define TCSR_ICB		0x40
#define TCSR_ICC		0x20
#define TCSR_ICD		0x10
#define TCSR_OCA		0x08	   // output compare events
#define TCSR_OCB		0x04
#define TCSR_OF			0x02	   // overflow event
#define TCSR_RESET_ON_A		0x01	   // reset counter on match A

//
// TCR bitmasks
//
#define TCR_A_RISING		0x80	   // input capture on rising
#define TCR_B_RISING		0x40	   // edge. if not set -> lower
#define TCR_C_RISING		0x20
#define TCR_D_RISING		0x10
#define TCR_BUFFER_A		0x08	   // buffer A in C
#define TCR_BUFFER_B		0x04	   // buffer B in D
#define TCR_CLOCK_2		0x00	   // clock = pclock / 2
#define TCR_CLOCK_8		0x01	   // clock = pclock / 8
#define TCR_CLOCK_32		0x02	   // clock = pclock / 32
#define TCR_CLOCK_EXT		0x03	   // external clock, rising edge

//
// TOCR bitmasks
//
#define TOCR_OCRA               0x00       // select register to write
#define TOCR_OCRB               0x10
#define TOCR_ENABLE_A           0x08       // enable output signals
#define TOCR_ENABLE_B           0x04
#define TOCR_HIGH_LEVEL_A       0x02       // set output to high for match
#define TOCR_HIGH_LEVEL_B       0x01


//
// 8 bit timer registers
//

//! serial / timer control register
extern          unsigned char STCR;

//! timer 0 control register
extern          unsigned char T0_CR;

//! timer 0 control / status register
extern volatile unsigned char T0_CSR;

//! timer 0 constant A register
extern          unsigned char T0_CORA;

//! timer 0 constant B register
extern          unsigned char T0_CORB;

//! timer 0 counter register
extern volatile unsigned char T0_CNT;


//! timer 1 control register
extern          unsigned char T1_CR;

//! timer 1 control / status register
extern volatile unsigned char T1_CSR;

//! timer 1 constant A register
extern          unsigned char T1_CORA;

//! timer 1 constant B register
extern          unsigned char T1_CORB;

//! timer 1 counter register
extern volatile unsigned char T1_CNT;


//
// CR flag bitmasks
//
#define CR_ENABLE_IRQA		0x40
#define CR_ENABLE_IRQB		0x80
#define CR_ENABLE_IRQO		0x20

#define CR_CLEAR_NEVER		0x00
#define CR_CLEAR_ON_A		0x08
#define CR_CLEAR_ON_B		0x10
#define CR_CLEAR_ON_EXTERN	0x18

// I don't define speeds here because the STCR bits are involved, too.
// Also, speeds differ for T0 and T1.


//
// CSR bitmasks
//
#define CSR_MATCH_A	0x40
#define CSR_MATCH_B	0x80
#define CSR_OVERFLOW	0x20

#define CSR_IGNORE_B  	0x00
#define CSR_0_ON_B     	0x04
#define CSR_1_ON_B     	0x08
#define CSR_TOGGLE_ON_B	0x0c

#define CSR_IGNORE_A  	0x00
#define CSR_0_ON_A     	0x01
#define CSR_1_ON_A     	0x02
#define CSR_TOGGLE_ON_A	0x03

//
// Serial port
//

//! serial receive data register
extern volatile unsigned char S_RDR;

//! serial transmit data register
extern          unsigned char S_TDR;

//! serial mode register
extern          unsigned char S_MR;

//! serial control register
extern          unsigned char S_CR;

//! serial status register
extern volatile unsigned char S_SR;

//! serial baud rate register
extern          unsigned char S_BRR;

//! serial / timer control register
extern          unsigned char S_TCR;

//
// register bitmasks
//
#define SMR_SYNC	0x80	   // in sync mode, the other settings
#define SMR_ASYNC	0x00	   // have no effect.
#define SMR_7BIT	0x40
#define SMR_8BIT	0x00
#define SMR_P_NONE	0x00
#define SMR_P_EVEN	0x20
#define SMR_P_ODD	0x30
#define SMR_1STOP	0x00
#define SMR_2STOP	0x08
#define SMR_MP		0x04	   // multiprocessing -> no parity
				   // for mp, mpe in STRC must be set
#define SMR_CLOCK	0x00	   // clock rate for baud rate generator
#define SMR_CLOCK_4	0x01	   // pclock / 4
#define SMR_CLOCK_16	0x02	   // pclock / 16
#define SMR_CLOCK_64	0x03	   // pclock / 64


#define SCR_TX_IRQ	0x80	   // TIE transmit irq enable
#define SCR_RX_IRQ	0x40	   // RIE receive / recv err irq enable
#define SCR_TRANSMIT	0x20	   // enable transmission
#define SCR_RECEIVE	0x10	   // enable receiving
#define SCR_MP_IRQ	0x08	   // multiprocessing receive irq
#define SCR_TE_IRQ	0x04	   // TEI transmit end irq enable
#define SCR_INT_CLOCK	0x00	   // internal clock source
#define SCR_EXT_CLOCK	0x02	   // external clock source
#define SCR_CLOCK_OUT	0x01	   // output internal clock to SCK pin


#define SSR_TRANS_EMPTY	0x80	   // transmit buffer empty
#define SSR_RECV_FULL	0x40	   // receive buffer full
#define SSR_OVERRUN_ERR	0x20	   // overrun error
#define SSR_FRAMING_ERR	0x10	   // framing error
#define SSR_PARITY_ERR	0x08	   // parity error
#define SSR_ERRORS      0x38       // all errors
#define SSR_TRANS_END	0x04	   // transmission end because buffer empty
#define SSR_MP		0x02	   // multiprocessor
#define SSR_MP_TRANSFER	0x01	   // multiprocessor bit transfer


//
// values for the bit rate register BRR
// assuming CMR_CLOCK selected on 16 MHz processor
// error <= 0.16%
//

#define B2400		207
#define B4800		103
#define B9600		51
#define B19200		25
#define B38400		12

typedef enum
{
    b2400  = B2400,
    b4800  = B4800,
    b9600  = B9600,
    b19200 = B19200,
    b38400 = B38400
} baud_rate_t;

// A/D converter
//

//! A/D converter data register A high
extern volatile unsigned char AD_A_H;

//! A/D converter data register A low
/*! bits 0..5 reserved, probably zero */
extern volatile unsigned char AD_A_L;

//! A/D converter data register B high
extern volatile unsigned char AD_B_H;

//! A/D converter data register B low
/*! bits 0..5 reserved, probably zero */
extern volatile unsigned char AD_B_L;

//! A/D converter data register C high
extern volatile unsigned char AD_C_H;

//! A/D converter data register C low
/*! bits 0..5 reserved, probably zero */
extern volatile unsigned char AD_C_L;

//! A/D converter data register D high
extern volatile unsigned char AD_D_H;

//! A/D converter data register D low
/*! bits 0..5 reserved, probably zero */
extern volatile unsigned char AD_D_L;


//! A/D converter data register A
/*! bits 0..5 reserved, probably zero */
extern volatile unsigned      AD_A;

//! A/D converter data register B
/*! bits 0..5 reserved, probably zero */
extern volatile unsigned      AD_B;

//! A/D converter data register C
/*! bits 0..5 reserved, probably zero */
extern volatile unsigned      AD_C;

//! A/D converter data register D
/*! bits 0..5 reserved, probably zero */
extern volatile unsigned      AD_D;


//! A/D converter control / status register
extern volatile unsigned char AD_CSR;

//! A/D converter control register
extern          unsigned char AD_CR;


//
// A/D CSR bitmasks
//

#define	ADCSR_END		0x80
#define ADCSR_ENABLE_IRQ	0x40
#define ADCSR_START		0x20
#define ADCSR_SCAN		0x10	  // enable scan mode
#define	ADCSR_TIME_266		0x00	  // select A/D conversion time
#define ADCSR_TIME_134		0x08

#define ADCSR_GROUP_0		0x00	  // select scan group
#define ADCSR_GROUP_1		0x04

#define ADCSR_AN_0		0x00	  // first group of inputs to convert
#define ADCSR_AN_1		0x01
#define ADCSR_AN_2		0x02
#define ADCSR_AN_3		0x03	  // in scan mode: AN0-AN3

//
// A/D CR bitmasks
//

#define ADCR_EXTERN		0x80	  // allow external triggering


//
// system control register
// FIXME: incomplete

//! system control register
extern          unsigned char SYSCR;

#define SYSCR_SOFTWARE_STANDBY	0x80	  // software standby flag

//
// I/O ports
//

//! port 1 input pull-up control register
extern          unsigned char PORT1_PCR;

//! port 2 input pull-up control register
extern          unsigned char PORT2_PCR;

//! port 3 input pull-up control register
extern          unsigned char PORT3_PCR;



//! port 1 data direction register
extern          unsigned char PORT1_DDR;

//! port 1 I/O register
extern volatile unsigned char PORT1;


//! port 2 data direction register
extern          unsigned char PORT2_DDR;

//! port 2 I/O register
extern volatile unsigned char PORT2;


//! port 3 data direction register
extern          unsigned char PORT3_DDR;

//! port 3 I/O register
extern volatile unsigned char PORT3;


//! port 4 data direction register
extern          unsigned char PORT4_DDR;

//! port 4 I/O register
extern volatile unsigned char PORT4;


//! port 5 data direction register
extern          unsigned char PORT5_DDR;

//! port 5 I/O register
extern volatile unsigned char PORT5;


//! port 6 data direction register
extern          unsigned char PORT6_DDR;

//! port 6 I/O register
extern volatile unsigned char PORT6;


//! port 7 input register
extern volatile unsigned char PORT7;



//! watch dog timer control register
extern volatile unsigned int WDT_CSR;

#define WDT_CSR_PASSWORD        (0xA500)
#define WDT_CSR_ENABLE          (0x0020)
#define WDT_CSR_MODE_WATCHDOG   (0x0040)
#define WDT_CSR_MODE_OVERFLOW   (0x0000)
#define WDT_CSR_WATCHDOG_NMI    (0x0000)
#define WDT_CSR_WATCHDOG_RES    (0x0008)
#define WDT_CSR_CLOCK_2         (0x0000)
#define WDT_CSR_CLOCK_32        (0x0001)
#define WDT_CSR_CLOCK_64        (0x0002)
#define WDT_CSR_CLOCK_128       (0x0003)
#define WDT_CSR_CLOCK_256       (0x0004)
#define WDT_CSR_CLOCK_512       (0x0005)
#define WDT_CSR_CLOCK_2048      (0x0006)
#define WDT_CSR_CLOCK_4096      (0x0007)

//! watch dog timer counter register
extern volatile unsigned char WDT_CNT;

#define WDT_CNT_PASSWORD        (0x5A00)
#define WDT_CNT_CLEAR           (0x0000)
#define WDT_CNT_MSEC_64         (0x0006)

#ifdef  __cplusplus
}
#endif

#endif	// __sys_h8_h__
