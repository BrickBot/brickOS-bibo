/*
 *  expandsf.s
 *
 *  Expands a single float into a flag byte, a sign byte, an exponent,
 *  and a mantissa.
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

    .section .text

;;
;; function: expandsf
;; input: float in r5r6
;; output: flags in r3h, sign in r3l, exp in r4, mant in r5r6
;;         flag bits: 0=zero, 1=inf, 2=nan
;;

    .global ___expandsf

___expandsf:

    ; Clear flag byte

    sub.b   r3h,r3h             ; clear flags (r3h)

    ; Extract sign (to r3l)

    mov.b   r5h,r3l             ; extract byte
    and.b   #0x80,r3l           ; mask to get sign bit

    ; Extract exponent (to r4)

    sub.w   r4,r4               ; clear exponent (r4)
    mov.b   r5h,r4l             ; copy upper 7 bits of exponent from r5h
    bld     #7,r5l              ; load lsb to carry (lsb is msb of r5l)
    rotxl.b r4l                 ; rotate to align

    ; Clear upper mantissa bit (mantissa is in r5l r6h r6l)

    bclr    #7,r5l              ; clear upper bit of second mantissa byte

    ; Upper byte of mantissa (r5h) as a temporary, for now
    ; Set it to non-zero if mantissa (r5l r6h r6l) is non-zero

    mov.b   r5l,r5h             ; set temp (r5h) non-zero if mantissa non-zero
    or.b    r6h,r5h
    or.b    r6l,r5h

    ; Is the exponent non-zero?  Note r4h is known to be zero.

    mov.b   r4l,r4l             ; if exponent (r4l) non-zero
    beq     else_0

        ; Does the exponent indicate infinity or NaN?  Note r4h is zero.

        cmp.b   #0xff,r4l       ; if exponent (r4l) is ff
        bne     endif_1

            ; Set the infinity flag if the mantissa is zero
            ; Set the NaN flag if the mantissa is non-zero
            ; Use temp in r5h to see if mantissa is zero or not

            add.b   #0xff,r5h   ; set carry if temp non-zero (destroying temp)
            bist    #1,r3h      ; set inf flag to ~carry (mantissa zero)
            bst     #2,r3h      ; set nan flag to carry (mantissa non-zero)

            ; Make exponent huge so it dominates finites
            ; Simplifies e.g. adding finite to inf or dividing inf by finite

            mov.b   #0x01,r4h   ; set lsb of upper exponent byte

        endif_1:

        ; Set the hidden one bit in the mantissa

        bset    #7,r5l          ; set hidden one bit
        bra     endif_0

    else_0:

        ; Exponent is zero.
        ; Is the mantissa non-zero, indicating a denorm?
        ; Use temp in r5h to see if mantissa is zero or not

        mov.b   r5h,r5h         ; if mantissa is non-zero
        beq     else_2

            ; Set the exponent to 1, its true value

            mov.b   #0x01,r4l   ; set exponent (r4l) to one

            ; Normalize the mantissa
            ; Do/while appropriate because must shift left at least one place

            dowhile_3:

                ; Shift mantissa one place to left

                add.w    r6,r6  ; shift mantissa left 1 place by adding
                addx.b   r5l,r5l ; note only 3 significant bytes

                ; Decrement exponent

                subs    #1,r4

                ; Is one bit set?  (one bit is in 1 << 23 or 00800000 position)

                btst    #7,r5l  ; check 00800000 bit of mantissa
                beq     dowhile_3 ; zero indicates bit not set yet

            bra     endif_2

        else_2:

            ; Mantissa is zero, indicating a zero value
            ; Set the zero flag

            bset    #0,r3h      ; set zero flag

        endif_2:

    endif_0:

    ; Clear the upper mantissa byte, which we used as a temporary

    sub.b   r5h,r5h             ; clear upper mantissa byte

    rts
