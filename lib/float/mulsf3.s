/*
 *  mulsf3.s
 *
 *  Floating point multiply, single precision: r0r1 *= r2r3
 *
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
 *  The Original Code is Librcx floating point code, released May 27, 1999.
 *
 *  The Initial Developer of the Original Code is Kekoa Proudfoot.
 *  Portions created by Kekoa Proudfoot are Copyright (C) 1999
 *  Kekoa Proudfoot. All Rights Reserved.
 *
 *  Contributor(s): Kekoa Proudfoot <kekoa@graphics.stanford.edu>
 */

; possible optimizations:
;  - use push/pop to save exponent and sign on stack
;  - might it have been faster/simpler to inline a 24 iteration multiply?
;       - this only requires 3b + 3b + 4b + 1b in regs to implement?

    .section .text

;;
;; function: mulsf3
;; input: float in r0r1 and float at sp+2
;; output: float in r0r1
;;

    .global ___mulsf3

___mulsf3:

    ; Invoke the prologue to expand input operands

    jsr  ___startsf

    ; At this point, registers/stack contain the following:
    ;    r0r1  - first operand
    ;    r2h   - second operand flags
    ;    r2l   - second operand sign
    ;    r3h   - first operand flags
    ;    r3l   - first operand sign
    ;    r4    - first operand exponent
    ;    r5r6  - first operand mantissa
    ;    sp+0  - second operand exponent
    ;    sp+2  - second operand flags (same as r2h)
    ;    sp+3  - second operand sign (same as r2l)
    ;    sp+4  - second operand mantissa
    ;    sp+16 - second operand

    ; Note on flag bits: 0=zero, 1=inf, 2=nan

    ; Is the first operand a NaN?
    ; If yes, return the first operand (the value already in r0r1)

    bld     #2,r3h              ; if nan flag of first operand set

    ; Hack!
    bcs     return_jmp          ; carry set indicates true

    ; Is the second operand a NaN?

    bld     #2,r2h              ; if nan flag of second operand set
    bcc     endif_0             ; carry clear indicates false

        ; Return the second operand (which we need to load off stack)

        mov.w   @(16,r7),r0     ; set return value to second operand (sp+16)
        mov.w   @(18,r7),r1

        ; Hack!
        return_jmp:

        bra     return

    endif_0:

    ; Is the first operand infinity and the second operand zero?
    ; Or is the second operand infinity and the first operand zero?

    bld     #1,r3h              ; load inf flag of first operand to carry
    band    #0,r2h              ; and carry with zero flag of second operand
    bcs     if_1                ; carry set indicates true

    bld     #1,r2h              ; load inf flag of second operand to carry
    band    #0,r3h              ; and carry with zero flag of first operand
    bcc     endif_1             ; carry clear indicates false

        if_1:

        ; Zero multiplied by infinity, so return NaN

        mov.w   #0x7fff,r0      ; set return value to NaN (7fffffff)
        mov.w   #0xffff,r1
        bra     return

    endif_1:

    ; At this point, we no longer need the following:
    ;    r0r1 - first operand
    ;    r2h  - second operand flags
    ;    r3h  - first operand flags

    ; Compute new exponent

    mov.w   @r7,r0              ; load second operand exponent (sp+0) to temp
    add.w   r0,r4               ; add temp to exponent of first operand (r4)
    add.b   #-127,r4l           ; subtract bias
    addx.b  #-1,r4h             ; finish subtraction

    ; Compute new sign

    xor.b   r2l,r3l             ; xor second sign (r2l) into first sign (r3l)

    ; At this point r2l is also free (and therefore all of r2 is free)
    ; We now want to multiply the two mantissas
    ; We need r3 and r4 free to do this

    ; Save result exponent and sign to stack

    mov.w   r4,@r7              ; sp+0 is result exponent
    mov.b   r3l,@(3,r7)         ; sp+3 is result sign

    ; Given two 1.23 mantissas, compute a 2.30 product with a sticky lsb
    ; Use three 24 by 8 multiplies to multiply two 24-bit mantissas

    ; Save first mantissa to r2r3

    mov.w   r5,r2               ; copy r5r6 to r2r3
    mov.w   r6,r3               ; note multiply done using r5r6

    ; Perform first multiply (first mantissa * lower byte of second mantissa)

    mov.b   @(7,r7),r4l         ; load lower byte of second mantissa to r4l

    bsr     mul_24_8            ; multiply r5l r6h r6l by r4l

    ; Save lower four result bytes in r0r1

    mov.w   r5,r0               ; copy r5r6 to r0r1
    mov.w   r6,r1

    ; Perform second multiply (first mantissa * middle byte of second mantissa)

    mov.w   r2,r5               ; copy first mantissa back to r5r6 from r2r3
    mov.w   r3,r6

    mov.b   @(6,r7),r4l         ; load middle byte of second mantissa to r4l

    bsr     mul_24_8            ; multiply r5l r6h r6l by r4l

    ; Combine new result into old and propagate carry into fifth result byte

    add.b   r6l,r1h             ; add corresponding bytes
    addx.b  r6h,r0l
    addx.b  r5l,r0h
    addx.b  #0,r5h              ; propagate carry

    ; Combine lower two result bytes into a single sticky byte

    or.b    r1l,r1h             ; compute new sticky byte

    ; Save fifth result byte

    mov.b   r5h,r1l             ; store fifth result byte

    ; Perform final multiply (first mantissa * upper byte of second mantissa)

    mov.w   r2,r5               ; copy first mantissa back to r5r6 from r2r3
    mov.w   r3,r6

    mov.b   @(5,r7),r4l         ; load upper byte of second mantissa to r4l

    bsr     mul_24_8            ; multiply r5l r6h r6l by r4l

    ; Combine old result into new and propagate carry into upper result byte

    add.w   r0,r6               ; add corresponding bytes
    addx.b  r1l,r5l
    addx.b  #0,r5h              ; propagate carry

    ; 2.30 result now in r5r6

    ; Set sticky bit if sticky byte or result lsb non-zero

    add.b   #0xff,r1h           ; set carry if sticky byte non-zero
    bor     #0,r6l              ; or carry with lsb to get sticky bit
    bst     #0,r6l              ; store sticky bit in lsb

    ; Sticky shift the result right one place to get a 2.29 value

    shlr.b  r5h                 ; shift mantissa right 1 place
    rotxr.b r5l
    rotxr.b r6h
    rotxr.b r6l                 ; places old sticky bit in carry

    bor     #0,r6l              ; or lsb with old sticky bit to get new bit
    bst     #0,r6l              ; store new sticky bit

    ; Restore result exponent and sign from stack

    mov.w   @r7,r4              ; sp+0 is result exponent
    mov.b   @(3,r7),r3l         ; sp+3 is result sign

    ; Pack the result

    jsr  ___joinsf

return:

    ; Invoke the epilogue to cleanup and return

    jmp  ___finishsf



;;
;; function: mul_24_8:
;; input: three-byte operand in r5l r6h r6l, single-byte operand in r4l
;; output: four-byte product in r5 r6
;; registers: assumes r4h r5h free
;;

mul_24_8:

    ; Rearrange bytes

    mov.b   r6h,r4h             ; move middle byte of three-byte operand to r4h

    ; Perform bytewise multiply

    mulxu.b r4l,r5              ; multiply r5l r4h r6l by single-byte operand
    mulxu.b r4l,r6              ; result split into words in r5 r4 r6
    mulxu.b r4h,r4              ; r5 r4 r6 are resp. upper middle lower words

    ; Combine results

    ; Add lower byte of middle word into upper byte of lower word
    ; Add upper byte of middle word into lower byte of upper word, using carry
    ; Propagate second carry to upper byte of upper word

    add.b   r4l,r6h             ; add corresponding bytes
    addx.b  r4h,r5l             ; add corresponding bytes, with carry
    addx.b  #0,r5h              ; propagate second carry

    rts
