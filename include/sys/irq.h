/*! \file   include/lnp/sys/irq.h
    \brief  Internal LNP Interface: RCX redirected IRQ vectors
    \author Markus L. Noga <markus@noga.de>

    Lego Mindstorms RCX IRQ redirection vector table
    All redirected handlers can assume r6 to be saved
    All redirected handlers must return with rts, *not* rte.

    \warning Incomplete.
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

#ifndef __sys_irq_h__
#define __sys_irq_h__

#ifdef  __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

#ifdef CONF_RCX_COMPILER
#define HANDLER_WRAPPER(wrapstring,handstring)
#else
#define HANDLER_WRAPPER(wrapstring,handstring) \
__asm__ (".text\n.align 1\n.global _" wrapstring "\n_" wrapstring \
": push r0\npush r1\npush r2\npush r3\njsr @_" handstring \
"\npop r3\npop r2\npop r1\npop r0\nrts\n")
#endif

///////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////

extern void *reset_vector;      //!< reset vector
extern void *nmi_vector;        //!< NMI interrupt vector
extern void *irq0_vector;       //!< IRQ0 interrupt vector
extern void *irq1_vector;       //!< IRQ1 interrupt vector
extern void *irq2_vector;       //!< IRQ2 interrupt vector
extern void *icia_vector;       //!< ICIA interrupt vector
extern void *icib_vector;       //!< ICIB interrupt vector
extern void *icic_vector;       //!< ICIC interrupt vector
extern void *icid_vector;       //!< ICID interrupt vector
extern void *ocia_vector;       //!< OCIA interrupt vector
extern void *ocib_vector;       //!< OCIB interrupt vector
extern void *fovi_vector;       //!< FOVI interrupt vector
extern void *cmi0a_vector;      //!< CMI0A interrupt vector
extern void *cmi0b_vector;      //!< CMI0B interrupt vector
extern void *ovi0_vector;       //!< OVI0 interrupt vector
extern void *cmi1a_vector;      //!< CMI1A interrupt vector
extern void *cmi1b_vector;      //!< CMI1B interrupt vector
extern void *ovi1_vector;       //!< OVI1 interrupt vector
extern void *eri_vector;        //!< ERI interrupt vector
extern void *rxi_vector;        //!< RXI interrupt vector
extern void *txi_vector;        //!< TXI interrupt vector
extern void *tei_vector;        //!< TEI interrupt vector
extern void *ad_vector;         //!< A/D interrupt vector
extern void *wovf_vector;       //!< WOVF interrupt vector

extern void *rom_reset_vector;	 //!< ROM reset vector
extern void rom_ocia_handler();  //!< ROM OCIA interrupt handler
extern void rom_ocia_return();   //!< return address in ROM OCIA handler
extern void rom_dummy_handler(); //!< address of an RTS instruction


///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

//! disable interrupt processing
extern inline void disable_irqs() {
  __asm__ __volatile__("\torc  #0x80,ccr\n":::"cc");
}

//! enable interrupt processing
extern inline void enable_irqs() {
  __asm__ __volatile__("\tandc #0x7f,ccr\n":::"cc");
}

#ifdef  __cplusplus
}
#endif

#endif	// __sys_irq_h__

