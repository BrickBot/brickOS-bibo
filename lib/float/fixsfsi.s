/*
 *  fixsfsi.s
 *
 *  Floating point to signed long conversion, single precision:
 *     r0r1 = (long) float r0r1
 *  Floating point to unsigned long conversion, single precision:
 *     r0r1 = (unsigned long) float r0r1
 *
 *  As far as I can tell, conversions for out-of-range values not well-defined
 *  I chose to implement the following:
 *
 *  For conversion to signed long, values for out-of-range conversions are:
 *
 *     converted value < -MAXINT  ->  -MAXINT
 *     converted value > +MAXINT  ->  +MAXINT
 *     input value == NaN         ->  +MAXINT
 *
 *  For conversion to unsigned long, values for out-of-range conversions are:
 *
 *     converted value < 0         ->  +MAXUINT
 *     converted value > +MAXUINT  ->  +MAXUINT
 *     input value == NaN          ->  +MAXUINT
 *
 *  -MAXINT is -2^31, +MAXINT is 2^31-1, +MAXUINT is 2^32-1
 *
 *  Another possibility would have been to return numbers modulo 2^31 or 2^32
 *  Or, could have returned either +MAXINT or +MAXUINT given out-of-range input
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
;; function: fixsfsi
;; input: float in r0r1
;; output: signed long in r0r1
;;

    .global ___fixsfsi

___fixsfsi:

    ; Assume r2 and r3 saved by caller

    mov.b   #0,r2h              ; load sign mask
    mov.b   #157,r2l            ; load maximum exponent (bias + 30)
    mov.b   #0x80,r3h           ; load upper byte of lower return value
    mov.b   #0x7f,r3l           ; load upper byte of higher return value

    ; Jump to fix routine

    bra     fix



;;
;; function: fixunssfsi
;; input: float in r0r1
;; output: unsigned long in r0r1
;;

    .global ___fixunssfsi

___fixunssfsi:

    ; Assume r2 and r3 saved by caller

    mov.b   #0x80,r2h           ; load sign mask
    mov.b   #158,r2l            ; load maximum exponent (bias + 31)
    mov.b   #0xff,r3h           ; load upper byte of lower return value
    mov.b   #0xff,r3l           ; load upper byte of higher return value

    ; Fall through to fix routine



;;
;; function: fix
;; input: float in r0r1, sign mask in r2h, max exp in r2l, return values in r3
;; output: signed or unsigned long in r0r1
;;

fix:

    ; Save registers

    push    r4
    push    r5
    push    r6

    ; Rearrange registers

    mov.w   r0,r5               ; copy input operand to r5r6
    mov.w   r1,r6
    mov.w   r3,r0               ; copy return values to r0

    ; Expand the input operand

    jsr  ___expandsf            ; expand to r3h r3l r4 r5r6

    ; At this point, registers contain the following:
    ;    r0h  - upper byte of lower return value
    ;    r0l  - upper byte of higher return value (also used if input is NaN)
    ;    r1   - [empty]
    ;    r2h  - sign mask
    ;    r2l  - maximum exponent
    ;    r3h  - input operand flags
    ;    r3l  - input operand sign
    ;    r4   - input operand exponent
    ;    r5r6 - input operand mantissa

    ; Note on flag bits: 0=zero, 1=inf, 2=nan

    ; Is the exponent small enough to make the number effectively zero?

    mov.w   #127,r1             ; load bias to r1
    cmp.w   r1,r4               ; is exponent < bias?
    bge     endif_0             ; greater than or equal indicates false

        ; Return zero result

        sub.w   r0,r0           ; load zero to result (r0r1)
        sub.w   r1,r1

        bra     return

    endif_0:

    ; Are we doing an unsigned conversion and is the number negative?
    ; Or does number's exponent exceed the maximum allowed value?
    ; Use sign mask to implement first test
    ; Also, note that if first test false, r2h cleared to zero for second test

    and.b   r3l,r2h             ; and sign into sign mask
    bne     if_1                ; not equal indicates true
    cmp.w   r2,r4               ; is exponent > maximum value for conversion?
    ble     endif_1             ; less than or equal indicates false

        if_1:

        ; Input is out of range.
        ; Is the number positive or NaN?

        bild    #7,r3l          ; load ~sign bit to carry
        bor     #2,r3h          ; or with nan flag bit
        bcc     endif_2         ; carry clear indicates false

            ; Move upper byte of higher return value to r0h

            mov.b  r0l,r0h

        endif_2:

        ; Correct upper byte of return value now in r0h
        ; Push lsb down into lower bytes

        bld     #0,r0h          ; load lsb to carry
        subx.b  r0l,r0l         ; bit extend right
        subx.b  r1h,r1h
        subx.b  r1l,r1l

        bra     return

    endif_1:

    ; At this point, we no longer need r0 r1 r2

    ; Left shift mantissa by 8 places
    ; This makes later shift take more time, but it also makes code smaller

    mov.b   r5l,r5h             ; shift by rearranging bytes
    mov.b   r6h,r5l
    mov.b   r6l,r6h
    sub.b   r6l,r6l

    ; Subtract bias + 31 from exponent

    add.b   #256-158,r4l        ; subtract bias + 31 from exponent
    addx.b  #-1,r4h

    ; Is exponent < 0 ?

    ; Note: do nothing if exponent >= 0

    ; Also note exponent should not be greater than zero if maximum
    ; exponent checks done correctly

    bge     endif_3             ; greater than or equal indicates false

        ; Shift mantissa right, increasing exponent until it reaches zero
        ; Note exponent only has 1 significant byte

        dowhile_4:

            shlr.b  r5h         ; shift mantissa right one place
            rotxr.b r5l
            rotxr.b r6h
            rotxr.b r6l

            inc     r4l         ; increment exponent
            blt     dowhile_4   ; continue while less than zero

    endif_3:

    ; Copy result to output

    mov.w   r5,r0               ; copy result (r5r6) to output (r0r1)
    mov.w   r6,r1

    ; Is sign < 0 ?

    mov.b   r3l,r3l             ; is sign < 0 ?
    bge     endif_5             ; greater than or equal indicates false

        ; Negate result

        sub.w   r0,r0           ; zero destination
        sub.w   r1,r1

        sub.w   r6,r1           ; subtract result (result in r5r6)
        subx.b  r5l,r0l
        subx.b  r5h,r0h

    endif_5:

return:

    ; Restore registers

    pop     r6
    pop     r5
    pop     r4

    rts
