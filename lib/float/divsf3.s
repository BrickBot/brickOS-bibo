/*
 *  divsf3.s
 *
 *  Floating point divide, single precision: r0r1 /= r2r3
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
 *
 *	Includes bug fixes by Evgeni Krimer <krimer at tx.technion.ac.il>
 *	and Roman Barsky <barsky at cs.technion.ac.il>
 *	Project site - http://www.cs.technion.ac.il/~barsky/brickos.html
 *	Last updated 26 Oct 2003 
 *
 */

    .section .text

;;
;; function: divsf3
;; input: float in r0r1 and float at sp+2
;; output: float in r0r1
;;

    .global ___divsf3

___divsf3:

    ; Invoke the preamble to expand input operands

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
        bra     return_jmp

    endif_0:

    ; Are both operands zero or both operands infinity?

    bld     #0,r3h              ; load zero flag of first operand to carry
    band    #0,r2h              ; and carry with zero flag of second operand
    bcs     if_1                ; carry set indicates true

    bld     #1,r3h              ; load inf flag of first operand to carry
    band    #1,r2h              ; and carry with inf flag of second operand
    bcc     endif_1             ; carry clear indicates false

        if_1:

        ; Zero divided by zero or infinity divided by infinity, so return NaN

        mov.w   #0x7fff,r0      ; set return value to NaN (7fffffff)
        mov.w   #0xffff,r1

        ; Hack!
        return_jmp:

        bra     return

    endif_1:

    ; Compute new sign byte

    xor.b   r2l,r3l             ; xor second sign (r2l) into first sign (r3l)

    ; At this point, we no longer need the following:
    ;    r0r1 - first operand
    ;    r2h  - second operand flags
    ;    r2l  - second operand sign
    ;    r3h  - first operand flags

    ; Is second operand zero or first operand infinity?

    bld    #0,r2h               ; load zero flag of second operand to carry
    bor    #1,r3h               ; or carry with inf flag of first operand
    bcc    endif_2

        ; Return +/- infinity according to sign

        mov.w   #0x7f80,r0      ; load infinity (7f800000) to return value
        sub.w   r1,r1
        or.b    r3l,r0h         ; or sign bit into upper byte
        bra     return

    endif_2:

    ; Compute new exponent

    mov.w   @r7,r0              ; load second operand exponend (sp+0) to temp
    sub.w   r0,r4               ; add temp to exponent of first operand (r4)
    add.b   #126,r4l            ; add bias minus one (compensate for 2.29)
    addx.b  #0,r4h              ; finish addition

    ; At this point r0 r1 r2 r3h are free and we want to perform a divide
    ; We need 6 words plus 1 byte of registers to do this
    ; Keep r3l where it is, but make r4 free

    ; Save result exponent to stack

    mov.w   r4,@r7              ; sp+0 is result exponent

    ; Numerator (first operand mantissa) already in r5r6
    ; Load denominator (second operand mantissa) to r0r1

    mov.w   @(4,r7),r0          ; sp+4 is second operand mantissa
    mov.w   @(6,r7),r1

    ; Initialize quotient (r2r4)

    sub.w   r2,r2               ; load zero to r2r4
    sub.w   r4,r4

    ; Initialize counter (r3h)

    mov.b   #26,r3h             ; set counter to 26

    ; Divide

    dowhile_3:

        ; If numerator >= denominator

        cmp.w   r0,r5           ; compare upper words (r5 ? r0)
        ;bgt     if_4           ; greater than indicates true
                                ; NOTE that previous line compares
                                ; signed which causes a bug 
                                ; fixed by Krimer and Barsky
        bhi     if_4            ; greater than indicates true
        ;blt     endif_4        ; less than indicates false 
                                ; NOTE that previous line compares
                                ; signed which causes a bug  
                                ; fixed by Krimer and Barsky
        blo     endif_4         ; less than indicates false 
        cmp.w   r1,r6           ; compare lower words (r6 ? r1)
        ;blt     endif_4        ; less than indicates false
                                ; NOTE that previous line compares
                                ; signed which causes a bug
                                ; fixed by Krimer and Barsky
        blo     endif_4         ; less than indicates false

            if_4:

            ; Subtract denominator from numerator

            sub.w   r1,r6       ; subtract r0r1 from r5r6
            subx.b  r0l,r5l
            subx.b  r0h,r5h

            ; Set quotient bit

            bset    #4,r4l      ; set 1 << 4 bit of quotient

        endif_4:

        ; Shift numerator and quotient left one place

        add.w   r6,r6           ; shift numerator left one place using add
        addx.b  r5l,r5l
        addx.b  r5h,r5h

        add.w   r4,r4           ; shift quotient left one place using add
        addx.b  r2l,r2l
        addx.b  r2h,r2h

        ; Decrement counter

        dec.b   r3h             ; decrement counter
        bne     dowhile_3       ; repeat if counter not yet zero

    ; Set sticky bit of quotient if remainder (numerator) is non-zero

    or.b    r5h,r5l             ; or remainder bytes together
    or.b    r5l,r6h
    or.b    r6h,r6l
    add.b   #0xff,r6l           ; set carry if remainder non-zero
    bst     #0,r4l              ; store sticky bit (carry) in lsb of quotient

    ; Move result to r5r6

    mov.w   r2,r5               ; copy quotient to r5r6
    mov.w   r4,r6

    ; Restore result exponent from stack

    mov.w   @r7,r4              ; sp+0 is result exponent

    ; Join

    jsr  ___joinsf

return:

    ; Invoke the epilogue to cleanup and return

    jmp  ___finishsf
