/*
 *  addsf3.s
 *
 *  Floating point add, single precision: r0r1 += r2r3
 *  Also implements floating point subtract, single precision: r0r1 -= r2r3
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

; bug fixes:
;  - 12/16/2000 fixed sp+16 sign bug (bug symptom found by Kieran Elby)

; possible optimizations:
;  - combine multiple returns of second/larger operand
;  - possibly simplify shift left 6 by factoring out common left shift op
;  - possibly simplify stickyshift by factoring out common stickyshift op
;  - possibly remove shift left 6 by computing a 1.32 result (use carry bit)

    .section .text

;;
;; function: subsf3
;; input: float in r0r1 and float at sp+2
;; output: float in r0r1
;;

    .global ___subsf3

___subsf3:

    ; Negate the second input

    mov.b @(2,r7),r2l
    xor.b #0x80,r2l
    mov.b r2l,@(2,r7)

    ; Fall through to add routine



;;
;; function: __addsf3
;; input: float in r0r1 and float at sp+2
;; output: float in r0r1
;;

    .global ___addsf3

___addsf3:

    ; Invoke the preamble to expand input operands

    jsr  ___startsf

subentry:

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

    btst    #2,r3h              ; if nan flag of first operand set

    ; Hack!
    bne     return_jmp          ; non-zero indicates true

    ; Is the second operand a NaN?

    btst    #2,r2h              ; if nan flag of second operand set
    beq     endif_0             ; zero indicates false

        ; Return the second operand (which we need to load off stack)

        mov.w   @(16,r7),r0     ; set return value to second operand (sp+16)
        mov.w   @(18,r7),r1

        ; Hack!
        bra     return_jmp

    endif_0:

    ; Are the operands both infinite with different signs?

    bld     #7,r3l              ; get sign of first operand
    bxor    #7,r2l              ; xor with sign bit of second operand
    band    #1,r3h              ; and with inf bit of first operand
    band    #1,r2h              ; and with inf bit of second operand
    bcc     endif_1             ; carry clear indicates false

        ; Both operands are infinite with different signs, so return NaN

        mov.w   #0x7fff,r0      ; set return value to NaN (7fffffff)
        mov.w   #0xffff,r1

        ; Hack!
        bra     return_jmp

    endif_1:

    ; At this point, registers contain the following:
    ;    r0r1  - first operand
    ;    r2h   - second operand flags
    ;    r2l   - second operand sign
    ;    r3h   - first operand flags
    ;    r3l   - first operand sign
    ;    r4    - first operand exponent
    ;    r5r6  - first operand mantissa
    ; We no longer need r0 r1 r2h r3h

    mov.b   r2l,r3h  ; transfer second operand sign to r3h

    ; We now have r0 r1 r2 free

    ; Our goal now is to make the registers/stack contain the following:
    ;    r0r1  - smaller operand mantissa
    ;    r2    - exponent difference (larger exponent minus smaller)
    ;    r3h   - subtract flag (xor of signs)
    ;    r3l   - larger operand sign
    ;    r4    - [empty]
    ;    r5r6  - larger operand mantissa
    ;    sp+2  - larger operand exponent
    ;    sp+16 - larger operand

    ; Set subtract flag

    xor.b   r3l,r3h

    ; Does the second operand have a larger exponent?

    mov.w   @r7,r2              ; load second operand exponent (sp+0) to r2
    sub.w   r4,r2               ; subtract first exponent (r4) from second (r2)

    ble     else_2              ; branch if second exponent <= first exponent

        ; Second operand is larger, rearrange accordingly

        mov.w   r5,r0           ; move first (smaller) mantissa to r0r1
        mov.w   r6,r1
        mov.b   @(3,r7),r3l     ; load second (larger) sign to r3l
        mov.w   @(4,r7),r5      ; load second (larger) mantissa to r5r6
        mov.w   @(6,r7),r6

        bra     endif_2

    else_2:

        ; First operand is larger, rearrange accordingly
        ; Also, negate exponent difference

        mov.w   r0,@(16,r7)     ; store first (larger) operand to sp+16
        mov.w   r1,@(18,r7)
        mov.w   r4,@r7          ; store first (larger) exponent to sp+0
        mov.w   @(4,r7),r0      ; load second (smaller) mantissa to r0r1
        mov.w   @(6,r7),r1
        not.b   r2l             ; negate exponent difference
        not.b   r2h
        adds    #1,r2

    endif_2:

    ; We have now successfully rearranged things
    ; Our registers and stack contain what we set out to have them contain
    ; See above

    ; If exponent difference exceeds 25, then return the larger operand

    mov.w   #25,r4              ; load 25 to free register
    cmp.w   r4,r2               ; if exponent difference (r2) > 25
    ble     endif_3

        ; Return larger operand

        mov.w   @(16,r7),r0     ; load larger operand to r0r1
        mov.w   @(18,r7),r1

        ; Hack!
        return_jmp:

        bra     return

    endif_3:

    ; Shift left both mantissas by 6 places

    mov.b   #6,r4l              ; use r4l as counter, set to 6

    dowhile_4:

        add.w   r1,r1           ; use add to shift smaller mantissa left
        addx.b  r0l,r0l
        addx.b  r0h,r0h

        add.w   r6,r6           ; use add to shift larger mantissa left
        addx.b  r5l,r5l
        addx.b  r5h,r5h

        dec.b   r4l             ; decrement counter
        bne     dowhile_4       ; repeat if counter not yet zero

    ; Shift the smaller operand right by the exponent difference
    ; Since exponent difference is less than 25, use only r2l as counter
    ; Maintain a sticky bit in lsb

    while_5:

        dec.b   r2l             ; if there are more places to shift
        blt     endwhile_5      ; negative counter indicates false

        shlr.b  r0h             ; shift mantissa right 1 place
        rotxr.b r0l
        rotxr.b r1h
        rotxr.b r1l             ; places old sticky bit in carry

        bor     #0,r1l          ; or lsb with old sticky bit to get new bit
        bst     #0,r1l          ; store new sticky bit

        bra     while_5

    endwhile_5:

    ; Load saved exponent from stack

    mov.w   @r7,r4              ; load larger exponent from sp+0

    ; Add or subtract?

    mov.b   r3h,r3h             ; if subtract flag not set
    bne     else_6

        ; Add

        add.w   r1,r6           ; compute r5r6 + r0r1
        addx.b  r0l,r5l
        addx.b  r0h,r5h

        ; If result is zero, we do not change sign, since signs were same

        bra     endif_6

    else_6:

        ; Subtract

        sub.w   r1,r6           ; compute r5r6 - r0r1
        subx.b  r0l,r5l
        subx.b  r0h,r5h

        ; Is result zero?

        bne     else_7          ; non-zero indicates false

            ; Make sign zero so we return +0, not -0

            sub.b   r3l,r3l     ; clear sign byte

            bra     endif_7

        else_7:

            ; Result is non-zero.  Is it negative?

            bge     endif_8     ; greater than or equal indicates false

                ; Flip sign

                xor.b   #0x80,r3l ; invert sign

                ; Negate mantissa

                mov.w   r5,r0   ; transfer mantissa to r0r1
                mov.w   r6,r1

                sub.w   r5,r5   ; zero destination
                sub.w   r6,r6

                sub.w   r1,r6   ; subtract original mantissa
                subx.b  r0l,r5l
                subx.b  r0h,r5h

            endif_8:

            ; Normalize

            jsr  ___normalizesf

        endif_7:

    endif_6:

    jsr  ___joinsf

return:

    ; Invoke the epilogue to cleanup and return

    jmp  ___finishsf
