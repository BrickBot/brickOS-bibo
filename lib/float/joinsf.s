/*
 *  joinsf.s
 *
 *  Joins a sign byte, an exponent, and a mantissa into a single float.
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
;  - factor sticky shift (4 shift ops, 2 bit ops) into a bsr

    .section .text

;;
;; function: joinsf
;; input: sign in r3l, exp in r4, mant in r5r6
;; output: float in r0r1
;;

    .global ___joinsf

___joinsf:

    ; Check for and fix overflow, converting 2.29 to 1.29

    bsr     fixoverflow

    ; Is exponent < 1 and is mantissa one bit set?

    mov.w   r4,r4               ; if exponent < 1
    bgt     endif_0             ; greater than implies false
    btst    #5,r5h              ; if mantissa one bit set
    beq     endif_0             ; non-zero implies false

        ; Handle a denorm.

        ; Is exponent < -23?

        mov.w   #-23,r0         ; load -23 to r0, which is currently free
        cmp.w   r0,r4           ; if exponent < -23
        bge     else_1          ; greater than or equal implies false

            ; Set mantissa and exponent to zero

            sub.w   r5,r5       ; clear mantissa
            sub.w   r6,r6
            sub.w   r4,r4       ; clear exponent

            bra     endif_1

        else_1:

            ; Shift mantissa right 1 - exponent places, maintaining sticky bit
            ; Note that since -23 <= exponent < 1, can use byte for counter
            ; Also note that 1 - exponent is at least 1, so use do/while
            ; Use r0l for counter

            mov.b   #1,r0l      ; load 1 to counter (r0l)
            sub.b   r4l,r0l     ; subtract exponent

            dowhile_2:

                ; Shift exponent right one place, maintaining sticky bit

                shlr.b  r5h     ; shift mantissa right 1 place
                rotxr.b r5l
                rotxr.b r6h
                rotxr.b r6l     ; last shift places old sticky bit in carry

                bor     #0,r6l  ; or lsb with old sticky bit
                bst     #0,r6l  ; store new sticky bit

                dec.b   r0l;    ; decrement counter and repeat if non-zero
                bne     dowhile_2

            ; Set exponent to 1

            mov.w   #1,r4       ; load 1 to exponent (r4)

        endif_1:

    endif_0:

    ; Round to nearest even by adding 0x1f if lsb is zero, 0x20 if lsb is one
    ; The lsb in this case is that of the rounded mantissa (the 1 << 6 bit)

    ; We do math by computing mantissa + 0x1f + carry, where lsb is in carry

    bld     #6,r6l              ; load lsb to carry
    addx.b  #0x1f,r6l           ; add 0x1f with carry
    addx.b  #0,r6h              ; finish addition
    addx.b  #0,r5l
    addx.b  #0,r5h

    ; Since that might have overflowed, check for and fix overflow

    bsr     fixoverflow

    ; Check for infinite result (exponent > 254)

    mov.w   #254,r0             ; load 254 to r0
    cmp.w   r0,r4               ; if exponent > 254
    ble     endif_2             ; less than or equal indicates false

        ; Return +inf if positive (sign==0x00), -inf if negative (sign==0x80)

        mov.w   #0x7f80,r0      ; load 0x7f800000 to r0r1
	sub.w   r1,r1
        or.b    r3l,r0h         ; or upper byte with sign byte to set sign

        bra     return

    endif_2:

    ; Is one bit zero (indicating denorm or zero)?

    btst    #5,r5h              ; is one bit set?
    bne     endif_3             ; non-zero indicates false

        ; Set exponent to zero

        sub.w   r4,r4           ; clear exponent to zero

    endif_3:

    ; Shift result right 6 places to remove guard bits

    mov.b   #6,r0l              ; use r0l as counter, set to 6

    dowhile_4:

        shlr.b  r5h             ; shift mantissa right 1 place
        rotxr.b r5l
        rotxr.b r6h
        rotxr.b r6l

        dec.b   r0l             ; decrement counter
        bne     dowhile_4       ; repeat if counter not yet zero

    ; Pack exponent (note 0 <= exp <= 254)

    mov.b   r4l,r5h             ; store in upper mantissa byte
    shlr.b  r5h                 ; shift right one place
    bst     #7,r5l              ; store exponent lsb in msb of next byte

    ; Set sign bit

    or.b    r3l,r5h             ; or in the sign byte

    ; Move result to r0r1

    mov.w   r5,r0
    mov.w   r6,r1

return:

    rts



;;
;; function: fixoverflow
;; input: exponent in r4, 2.29 or 1.29 mantissa in r5r6
;; output: exponent in r4, 1.29 mantissa in r5r6
;;

fixoverflow:

    btst    #6,r5h              ; is two bit set?
    beq     endif_5             ; zero indicates false

        ; Shift mantissa right one place, maintaining sticky bit

        shlr.b  r5h             ; shift mantissa right 1 place
        rotxr.b r5l
        rotxr.b r6h
        rotxr.b r6l             ; last shift places old sticky bit in carry

        bor     #0,r6l          ; or lsb with old sticky bit
        bst     #0,r6l          ; store new sticky bit

        ; Increase exponent

        adds    #1,r4           ; add one to exponent

    endif_5:

    rts
