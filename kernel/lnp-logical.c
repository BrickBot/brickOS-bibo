/*! \file   lnp-logical.c
    \brief  Implementation: link networking protocol logical layer
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
 *                  Chris Dearman <chris@algor.co.uk>
 *                  Martin Cornelius <Martin.Cornelius@t-online.de>
 *
 *                   Matthew Sheets - 2009-05-02
 *                    - Added CONF_HOST capabilities for improved code reuse
 *
 *                   Matthew Sheets - 2009-06-13
 *                    - Enabled changing the baud rate and carrier state at runtime
 *                    - Enabled turning off RCX echo handing (e.g. for bluetooth hacks)
 */

#include <sys/lnp-logical.h>

#ifdef CONF_LNP

#include <sys/lnp.h>

#ifndef CONF_HOST

#include <sys/h8.h>
#include <sys/irq.h>

#include <time.h>
#include <mem.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/tm.h>
#include <sys/time.h>
#include <sys/waitqueue.h>

#include <rom/registers.h>

///////////////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////////////

//! LNP baud rate
//! Default vaule in config.h may be overidden at compile type
volatile baud_rate_t lnp_baud_rate = CONF_LNP_STARTUP_BAUD_RATE;

//! IR Carrier state (enabled/disabled)
//! Default value in config.h
volatile unsigned char lnp_ir_carrier_enabled = CONF_LNP_IR_CARRIER_ENABLED;

//! Is TX echoed?
//! Default value in config.h
volatile unsigned char lnp_tx_echoes = CONF_LNP_TX_ECHOES;

/* We use only low 16 bit of sys_time, which is safe */
extern volatile time_t sys_time;

static const unsigned char *tx_ptr; //!< ptr to next byte to transmit
static const unsigned char *tx_verify;  //!< ptr to next byte to verify
static const unsigned char *tx_end; //!< ptr to byte after last

static unsigned int lnp_lastbyte_time; //!< time of last received byte

volatile signed char tx_state;    //!< flag: transmission state

//! when to allow next transmission
/*! transmit OK -> wait some time to grant others bus access
    collision   -> wait some more, plus "random" amount.
    receive     -> reset timeout to default value.
*/

static void run_allow_tx(void *data);
//! timer to allow new transmission
static timer_t allow_tx_timer = {
    0, run_allow_tx, 0, 0
};
static waitqueue_t *lnp_waitqueue = NULL;   //!< wait queue for transmitter

#ifdef CONF_TM
sem_t tx_sem;                //!< transmitter access semaphore
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

//! disable IR carrier frequency.
extern inline void carrier_shutdown(void) {
  T1_CR  =0;
  T1_CSR =0;
}

//! enable IR carrier frequency.
extern void carrier_init(void) {
  if (lnp_ir_carrier_enabled){
    T1_CR  =0x9;
    T1_CSR =0x13;
    T1_CORA=0x1a;
  } else {
    carrier_shutdown();
  }
}

//! Get the baud rate
inline baud_rate_t get_baud_rate(void) {
  return lnp_baud_rate;
}

//! Set the baud rate
/*! Configure the baud rate
 * \param baud: the baud rate of type baud_rate_t to use
 */
inline void set_baud_rate(baud_rate_t baud) {
  lnp_baud_rate = baud;
  S_BRR = baud;
}

char get_byte_time() {
  switch (get_baud_rate()) {
    case  b2400:    return MSECS_TO_TICKS(5);
    case  b4800:    return MSECS_TO_TICKS(3);
    case  b9600:    return MSECS_TO_TICKS(2);
    case b19200:    return MSECS_TO_TICKS(1);
    case b38400:    return MSECS_TO_TICKS(1);
    default:        return MSECS_TO_TICKS(5);
  }
}

//! Get the IR carrier state
inline unsigned char get_carrier_state(void) {
  return lnp_ir_carrier_enabled;
}

//! Set the IR carrier state
/*! Configure the INFRARED carrier state
 \param carrier_state: TRUE: carrier enabled, FALSE: carrier disabled
 */
inline void set_carrier_state(unsigned char carrier_state) {
  lnp_ir_carrier_enabled = carrier_state;
  carrier_init();
}

//! Get whether TX is flagged as echoing
inline unsigned char get_tx_echoes(void) {
  return lnp_tx_echoes;
}

//! Set whether TX echoes
/*! Flag whether the current TX method echoes transmissions
 * \param tx_echoes: TRUE: tx echoes, FALSE tx does not echo
 */
inline void set_tx_echoes(unsigned char tx_echoes) {
  lnp_tx_echoes = tx_echoes;
}

static void rx_core(void);
static void rxerror_core(void);

handler_t rx_handler __attribute__ ((unused)) = { 
    0, &rx_core 
};
handler_t rxerror_handler __attribute__ ((unused)) = {
    0, &rxerror_core 
};

extern void rx_interrupt(void);
extern void rxerror_interrupt(void);
extern void tx_interrupt(void);
extern void txend_interrupt(void);

//! the byte received interrupt handler
//
#ifndef DOXYGEN_SHOULD_SKIP_THIS
__asm__("\n\
.text\n\
.align 1\n\
.global _rx_interrupt\n\
.global _rxerror_interrupt\n\
.global _tx_interrupt\n\
.global _txend_interrupt\n\
\n\
_rx_interrupt:\n\
	; r6 saved by ROM\n\
	push  r0\n\
\n"
	enqueue_handler_irq(rx_handler)

        "\n\
	pop   r0\n\
	bclr  #6,@_S_SR:8       ; clear RDR Full flag\n\
        rts\n\
\n\
_rxerror_interrupt:\n\
	; r6 saved by ROM\n\
	push  r0\n\
\n"
	enqueue_handler_irq(rxerror_handler)

        "\n\
	pop   r0\n\
\n\
	mov.b @_S_SR:8, r6l\n\
	and.b #0xc7, r6l        ; clear overrun framing and parity error\n\
	mov.b r6l, @_S_SR:8\n\
        rts\n\
\n\
\n\
; the transmit byte interrupt handler\n\
; write next byte if there's one left, otherwise unhook irq.\n\
_tx_interrupt:\n\
	; r6 saved by ROM\n\
	push  r0\n\
	mov.w @_tx_ptr, r0\n\
	mov.w @_tx_end, r6\n\
	cmp.w r6,r0             ; any more bytes to transmit?\n\
	bhs   1f\n\
	mov.b @r0+,r6l          ; get next byte\n\
	mov.b r6l,@_S_TDR:8     ; put it in transmit register\n\
	mov.w r0,@_tx_ptr        ; increase tx_ptr\n\
	bclr  #7,@_S_SR:8       ; clear TDR empty flag\n\
	bra   2f\n\
1:	bclr  #7,@_S_CR:8       ; disable TX interrupt\n\
2:	pop   r0\n\
        rts\n\
\n\
; shutdown transmit and irqs, clear status flags\n\
_txend_interrupt:\n\
	; r6 saved by ROM\n\
	bclr  #5,@_S_CR:8       ; clear transmit flag\n\
	bclr  #2,@_S_CR:8       ; disable txend interrupt\n\
        rts\n"
    );
#endif // DOXYGEN_SHOULD_SKIP_THIS

__TEXT_HI__ void tx_coll(void) {
    S_CR&=~(SCR_TX_IRQ | SCR_TRANSMIT | SCR_TE_IRQ);
    S_SR&=~(SSR_TRANS_EMPTY | SSR_TRANS_END);
    tx_state=TX_COLL;
    wakeup(lnp_waitqueue);
}

__TEXT_HI__ void rx_core(void) {
  if(tx_state<TX_ACTIVE || !lnp_tx_echoes) {
    // foreign bytes
    //
    if ((unsigned) sys_time - lnp_lastbyte_time
	>= (unsigned) LNP_BYTE_TIMEOUT) {
	lnp_integrity_reset();
    }
    lnp_lastbyte_time = (unsigned) sys_time;
    lnp_integrity_byte(S_RDR);
  } else {
    // echos of own bytes
    //
    if(S_RDR!=*tx_verify) {
      // bytes do not match -> collision detection
      tx_coll();
    } else if( tx_end <= ++tx_verify ) {
      // let transmission end handler handle things
      //
      tx_state=TX_IDLE;
      wakeup(lnp_waitqueue);
    }
  }
}

__TEXT_HI__ void run_allow_tx(void *data) {
    wakeup(lnp_waitqueue);
}

//! the receive error interrupt handler
//
__TEXT_HI__ void rxerror_core(void) {
    if(tx_state<TX_ACTIVE || !lnp_tx_echoes) {
	lnp_integrity_reset();
	lnp_lastbyte_time = (unsigned) sys_time;
    } else {
	tx_coll();
    }
}

//! shutdown IR port
__TEXT_HI__ void lnp_logical_shutdown(void) {
  S_CR =0;        // everything off
  carrier_shutdown();
  lnp_logical_range(0);

  tx_state=TX_IDLE;
  remove_timer(&allow_tx_timer);

#ifdef CONF_TM
  sem_destroy(&tx_sem);
#endif
}

//! initialize link networking protocol logical layer (IR port).
/*! initially set to low range.
*/
__TEXT_HI__ void lnp_logical_init(void) {
  // safe initial state.
  //
  lnp_logical_shutdown();

#ifdef CONF_TM
  sem_init(&tx_sem,0,1);
#endif

  // serial setup -  was 8N1 fixed, now parity is also set.
  //
  S_MR =SMR_P_ODD;
  set_baud_rate(lnp_baud_rate);
  S_SR =0;

  // carrier setup
  //
  rom_port4_ddr |= 1;     // port 4 bit 0 output
  PORT4_DDR = rom_port4_ddr;
  carrier_init();
  rom_port5_ddr = 4;      // init p5ddr, for now
  PORT5_DDR = rom_port5_ddr;

  // IRQ handler setup
  //
  eri_vector=&rxerror_interrupt;
  rxi_vector=&rx_interrupt;
  txi_vector=&tx_interrupt;
  tei_vector=&txend_interrupt;

  // enable receiver and IRQs
  //
  S_CR=SCR_RECEIVE | SCR_RX_IRQ;
}

//! write to IR port, blocking.
/*! \param buf data to transmit
    \param len number of bytes to transmit
    \return 0 on success, else collision or shutdown requested
*/
int lnp_logical_write(const void* buf,size_t len) {
    unsigned char tmp;
    int      coll_delay;
    unsigned lastbyte_delay;
    int      result;
    waitqueue_t entry;

    /* First grab the transmit semaphore.  This prevents all
     * other senders from disturbing us.
     */
    if (sem_wait(&tx_sem) < 0)
  	return TX_ACTIVE;

    /* Store address of transmit buffer in global variables
     * for the interrupt routines.
     * tx_ptr: next byte to transmit
     * tx_verify: next byte to receive (IR echo).
     * tx_end: end of transmit buffer.
     */
    tx_verify=tx_ptr=buf;
    tx_end=buf+len;
    
    grab_kernel_lock();
    add_to_waitqueue(&lnp_waitqueue, &entry);

 retry:
    if (shutdown_requested())
	goto clean_up;
    
    /* check whether we should delay transmission */
    coll_delay = get_timer_count(&allow_tx_timer);
    lastbyte_delay = (unsigned) sys_time - lnp_lastbyte_time;
    if (lastbyte_delay < (unsigned) LNP_BYTE_SAFE
	&& coll_delay < (signed) (LNP_BYTE_SAFE - lastbyte_delay)) {
	add_timer(LNP_BYTE_SAFE - lastbyte_delay, &allow_tx_timer);
	coll_delay = LNP_BYTE_SAFE - lastbyte_delay;
    }

    if (coll_delay != -1) {
	wait();
	goto retry;
    }

    tx_state=TX_ACTIVE;
    S_SR&=~(SSR_TRANS_EMPTY | SSR_TRANS_END); // clear flags
    S_CR|=SCR_TRANSMIT | SCR_TX_IRQ | SCR_TE_IRQ; // enable transmit & irqs
    
    while (tx_state == TX_ACTIVE && !shutdown_requested())
	wait();
    
    // determine delay before next transmission
    //
    if(tx_state==TX_COLL)
	tmp=LNP_WAIT_COLL + ( ((unsigned char) 0x0f) &
			      ( ((unsigned char) len)+
				((unsigned char*)buf)[len-1]+
				((unsigned char) sys_time)    ) );
    else
	tmp=LNP_WAIT_TXOK;
    add_timer(tmp, &allow_tx_timer);

 clean_up:
    remove_from_waitqueue(&entry);
    release_kernel_lock();
    result = tx_state;
    sem_post(&tx_sem);
    return result;
}
#else  // CONF_HOST

#include <errno.h>
#include <stdio.h>
#include <unistd.h>


tty_t lnp_tty = { BADFILE, tty_t_undefined };

//! Initialize the logical layer (IR port)
void lnp_logical_init(tty_t *tty) {
    lnp_tty.fd = tty->fd;
    lnp_tty.type = tty->type;
}

//! Shut down the logical layer (IR port)
extern void lnp_logical_shutdown(void) {
    lnp_tty.fd = BADFILE;
    lnp_tty.type = tty_t_undefined;
}

//! write to IR port, blocking.  Provides compatibility with the RCX library using globals for the FILEDESCR and tty_type_t; use lnp_logical_write_host instead if possible!
/*! \param buf data to transmit
    \param len number of bytes to transmit
    \return 0 on success, else collision or shutdown requested
*/
int lnp_logical_write(const void* buf, size_t len) {
    return lnp_logical_write_host(&lnp_tty, buf, len);
}

//! write to IR port, blocking; specific to the "host" implementation; for keepalive compatibility, use higher-level rcx_nbwrite instead
/*! \param file descriptor to write to
    \param tty type
    \param buf data to transmit
    \param len number of bytes to transmit
    \return 0 on success, else collision
*/
int lnp_logical_write_host(tty_t *tty, const void* buf, size_t len) {
    if (tty_t_undefined == tty->type) {
        fprintf(stderr, "ERROR: lnp_logical not initialized or tty_type undefined\n");
        fflush(stderr);
        return -1;
    }
    /* // Debug code added by Taiichi
    int i;
    unsigned char *bbuf = (unsigned char*)buf;
    fprintf(stderr,"\nlnp_logical_write_host (%d, ..., %d)\n", (int)tty->fd, (int)len);
    for (i = 0; i < len; i++) {
        fprintf(stderr,"%02x ", bbuf[i]);
        if (((i+1) % 16) == 0) fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
    // */
    
#if defined(_WIN32)
  // TCP uses more Linux-style communication
  if (!(tty_types_tcp & tty->type)) {
    DWORD nBytesWritten=0;
    if (WriteFile(tty->fd, buf, len, &nBytesWritten, NULL))
      return (int)(len - nBytesWritten);
    else
      return -1;
  }
#endif
   /* For usb tower, the driver, legousbtower, uses interrupt  */
   /* urb which can only carry up to 8 bytes at a time. To     */
   /* transmit more we have to check and send the rest.  It is */
   /* a good thing to check, so I'll make it general.          */

   int actual = 0;
   int rc = 0;
   char * cptr;
   int retry = 1000;

   if (len < 1) return len;
   cptr = (char *) buf;
   while (actual < len) {
     rc = write(tty->fd, cptr+actual, len-actual);
     if (rc == -1) {
       if ((errno == EINTR) || (errno == EAGAIN))  {
         rc = 0;
         usleep(10);
         retry --;
       } else return -1;
     }
     actual += rc;
     if (retry < 1) return len - actual;
   }
   return len - actual;
}

#endif // CONF_HOST

#endif  // CONF_LNP
