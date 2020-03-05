/******************************************************************************

  The contents of this file are subject to the Mozilla Public License
  Version 1.1 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
  the License for the specific language governing rights and limitations
  under the License. 

  The Original Code is DCC Library for the RCX.

  The Initial Developer of the Original Code is Mark Riley. Portions
  created by Mark Riley are Copyright (C) 2003 Mark Riley. All Rights
  Reserved. 

     File : dcc.s
  Created : 3/20/2003
  Purpose : H8 assembly language routines for DCC library

******************************************************************************/

#include <config.h>

#ifdef CONF_DCC

MOTOR = 0xFF80

 .macro ENABLE_IRQS
  andc #0x7F,ccr
 .endm

 .macro DISABLE_IRQS
  orc #0x80,ccr
 .endm

 .macro SET_C
  orc #0x01,ccr
 .endm

 .global _dcc_mask

/******************************************************************************
  void dcc_flip()

  r0l trashed
******************************************************************************/

 .global _dcc_flip
_dcc_flip:

  DISABLE_IRQS

  ; flip polarity
  mov @MOTOR,r0h
  mov @_dcc_mask,r0l
  xor r0l,r0h
  mov r0h,@MOTOR

  ENABLE_IRQS
  rts

/******************************************************************************
  Macro to invert polarity of signal
  r2h <- polarity flip mask
  r3l trashed
******************************************************************************/

FP_STATES = 15+6+15

 .macro FLIP_POLARITY

  mov @MOTOR,r3l     ; 15
  xor r2h,r3l  ; 6
  mov r3l,@MOTOR     ; 15

 .endm

/******************************************************************************
  Macro for timing delay
  
  US     <- microseconds
  STATES <- states used by overhead instructions

  r3h <- adjustment (units of 18 states)
  r3h -> 0
******************************************************************************/

 .macro DELAY US,STATES

  ; subtract 6 for "add #" states???

  add #((\US) * 16 - (FP_STATES + (\STATES)) + 9) / (6 + 12),r3h  ; 6
9:
  dec r3h            ; 6
  bne 9b             ; 12

 .endm

/******************************************************************************
  Adjustment macro for timing delay
  
  STATES <- states used by overhead instructions

  r3h <-> adjustment (units of 18 states)
******************************************************************************/

 .macro ADJUST_DELAY STATES

  add #-((6 + (\STATES) + 9) / (6 + 12)),r3h  ; 6

 .endm

/******************************************************************************
  void dcc_packet(UInt8 preamble, UInt8* packet, UInt8 length)

  r0l <- number of preamble bits
  r1  <- packet data
  r2l <- packet length
  r0-r2 trashed
******************************************************************************/

 .global _dcc_packet
_dcc_packet:

  push r3

  ; clear delay loop adjustment
  mov #0,r3h

  ; polarity flip mask
  mov @_dcc_mask,r2h

  DELAY 58,0

  DISABLE_IRQS

  FLIP_POLARITY

1:; output preamble bits
  bsr dcc_1_bit      ; ~16

  ADJUST_DELAY 6+12

  dec r0l            ; 6
  bne 1b             ; 12

  ; output packet start bit
  bsr dcc_0_bit      ; ~16

  ; reset checksum
  mov #0,r0h         ; 6

  ADJUST_DELAY 6+11+6

1:; loop through packet data bytes
  mov @r1+,r0l       ; ~11

  ; compute checksum
  xor r0l,r0h        ; 6

  ; output data byte
  bsr dcc_byte_out   ; ~16

  ; output end of data byte bit
  bsr dcc_0_bit      ; ~16

  ADJUST_DELAY 6+12+11+6

  dec r2l            ; 6
  bne 1b             ; 12

  ; output checksum
  mov r0h,r0l        ; 6
  bsr dcc_byte_out   ; ~16

  ; output packet end bit
  bsr dcc_1_bit      ; ~16

  ENABLE_IRQS        ; 6

  ; end of packet delay
  DELAY 58,0

  pop r3
  rts

/******************************************************************************
  r0l <- byte
  r0l,r3 trashed
******************************************************************************/

; NON-FRC

dcc_byte_out:
  ADJUST_DELAY 16+6+6+12

  SET_C              ; 6
  rotxl r0l          ; 6
  bcc 0f             ; 12

1:
  bsr dcc_1_bit      ; ~16

  ADJUST_DELAY 6+12+12

  shll r0l           ; 6
  beq 2f             ; 12
  bcs 1b             ; 12

0:
  bsr dcc_0_bit      ; ~16

  ADJUST_DELAY 6+12+12

  shll r0l           ; 6
  beq 2f             ; 12
  bcs 1b             ; 12

  bra 0b             ; 12

2:
  ADJUST_DELAY 18-12

  rts                ; ~18

/******************************************************************************
  r3 trashed
******************************************************************************/

; NON-FRC

dcc_1_bit:
  DELAY 58,18+16

  FLIP_POLARITY

  DELAY 58,0

  FLIP_POLARITY

  rts                ; ~18

/******************************************************************************
  r3 trashed
******************************************************************************/

; NON-FRC

dcc_0_bit:
  ; interrupts OK here
  ENABLE_IRQS        ; 6

  DELAY 100,18+16+6+6

  DISABLE_IRQS       ; 6

  FLIP_POLARITY

  ; interrupts OK here
  ENABLE_IRQS        ; 6

  DELAY 100,6+6

  DISABLE_IRQS       ; 6

  FLIP_POLARITY

  rts                ; ~18

/* EOF */

#endif
