/*
 *  cmpsf2.s
 *
 *  Floating point compare: r0r1 = (r0r1 > r2r3) - (r0r1 < r2r3)
 *  GCC apparently does not call this because it is not normal IEEE
 *
 *  For GCC, we also implement: == != < > <= >=
 *  All of these return -1 0 1 as with the ordinary floating point compare
 *  However, all return false given NaN input, except != which returns true
 *
 *  The non-GCC non-IEEE floating point compare returns zero given NaN input
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
;; functions: cmpsf2, eqsf2, nesf2, ltsf2, gtsf2, lesf2, gesf2
;; input: float in r0r1 and float at sp+2
;; output: int in r0
;;

    ; In all cases, assume r2 and r3 saved by caller

    .global ___eqsf2
    .global ___nesf2
    .global ___ltsf2
    .global ___lesf2

___eqsf2:
___nesf2:
___ltsf2:
___lesf2:

    ; Load 1 as NaN return value

    mov.b   #1,r2l
    bra     compareentry

    .global ___gtsf2
    .global ___gesf2

___gtsf2:
___gesf2:

    ; Load -1 as NaN return value

    mov.b   #-1,r2l
    bra     compareentry

    .global ___cmpsf2

___cmpsf2:

    ; Load 0 as NaN return value

    sub.b   r2l,r2l

compareentry:

    ; Save registers

    push    r4
    push    r5

    ; Clear flag byte

    sub.b   r2h,r2h

    ; Set flags for first operand

    mov.w   r0,r3               ; copy first operand to r3r4
    mov.w   r1,r4
    bsr     setflags            ; updates r2h, does not change r3r4

    ; Set flags for second operand

    mov.w   @(6,r7),r3          ; load second operand to r3r4
    mov.w   @(8,r7),r4
    bsr     setflags            ; updates r2h, does not change r3r4

    ; Set a negative flag if both operands negative

    bld     #7,r0h              ; load sign bit of first operand to carry
    band    #7,r3h              ; and with sign bit of second operand
    bst     #2,r2h              ; store carry as flag bit

    ; Note on flag bits: 0=either non-zero, 1=either NaN, 2=both negative

    ; Is the NaN flag set?

    btst    #1,r2h              ; is NaN flag (bit 1) set?
    beq     endif_0             ; zero indicates false

        ; One of the operands is a NaN
        ; Move NaN return value to r0l and return

        mov.b   r2l,r0l
        bra     return

    endif_0:

    ; Is the non-zero flag clear?

    btst    #0,r2h              ; is non-zero flag (bit 0) clear?
    bne     endif_1             ; non-zero indicates false (flag set)

        ; Neither operand is non-zero, so both operands are zero
        ; Operands are equal, so move zero return value to r0l and return

        sub.b   r0l,r0l
        bra     return

    endif_1:

    ; Subtract second operand (r3r4) from first operand (r0r1)

    sub.w   r4,r1
    subx.b  r3l,r0l
    subx.b  r3h,r0h

    ; Is the result equal to zero?
    ; If yes, return value of zero already in r0l, return

    beq     return              ; zero indicates true

    ; Is the result greater than zero?

    blt     else_2

        ; Set the return value to 2 (for now, 2 indicates greater than)

        mov.b   #2,r0l
        bra     endif_2

    else_2:

        ; Set return value to 0 (for now, 0 indicates less than)

        sub.b   r0l,r0l

    endif_2:

    ; Is the negative flag set?

    btst    #2,r2h              ; is negative flag (bit 2) set?
    beq     endif_3             ; zero indicates false

        ; Both operands are negative, so fix incorrect sense of compare

        xor.b   #2,r0l          ; change 2 to 0 and 0 to 2

    endif_3:

    ; Decrement return value

    dec.b   r0l                 ; change 2 to 1 and 0 to -1

return:

    ; Sign extend return value

    bld     #7,r0l
    subx.b  r0h,r0h

    ; Restore registers

    pop     r5
    pop     r4

    ; Return

    rts



;;
;; function: setflags
;; input: float in r3r4, previous flags in r2h
;; output: updated flags in r2h
;; registers: assumes r5 free, preserves r3r4
;;

setflags:

    ; Note on relevant flag bits: 0=either non-zero, 1=either NaN

    ; Move upper input word to r5 and mask off msb

    mov.w   r3,r5
    and.b   #0x7f,r5h

    ; Is the input non-zero?

    mov.w   r4,r4               ; is the lower word non-zero?
    bne     if_4                ; non-zero indicates true
    mov.w   r5,r5               ; is the upper word non-zero?
    beq     endif_4             ; zero indicates false

        if_4:

        ; Input is non-zero, set the non-zero flag

        bset    #0,r2h          ; set the non-zero flag to 1

    endif_4:

    ; Is input a NAN ((input & 7fffffff) > 7f800000) ?

    add.b   #0,r4l              ; subtract 7f800000 from r5r4
    addx.b  #0,r4h              ; do subtract by adding -7f800000=80800000
    addx.b  #0x80,r5l
    addx.b  #0x80,r5h
    ble     endif_5             ; less than or equal indicates false

        ; Input is NaN, set the NaN flag

        bset    #1,r2h

    endif_5:

    rts
