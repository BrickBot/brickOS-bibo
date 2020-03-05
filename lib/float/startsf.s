/*
 *  startsf.s
 *
 *  Common preamble for floating point routines that take two operands
 *  Common epilogue for floating point routines that take two operands
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
;; function: startsf
;; input: float in r0r1 and float at caller's sp+2 before call
;; output: first float in r0r1 and second float at caller's sp+16 after call
;;         expanded first float, r3 flags/sign, r4 exponent, r5r6 mantissa
;;         expanded second float, sp+0 exponent, sp+2 flags/sign, sp+4 mantissa
;;         expanded second float flags/sign also in r2
;;         saved r4, r5, r6 at caller's sp+12, sp+10, sp+8
;;

    .global ___startsf

___startsf:

    ; Assume r2 and r3 saved by original caller

    pop     r2                  ; load return address to r2

    ; Save registers on stack

    push    r4
    push    r5
    push    r6

    ; Expand the second operand

    mov.w   @(8,r7),r5          ; copy second operand (sp+8) to r5r6
    mov.w   @(10,r7),r6
    jsr  ___expandsf            ; expand to r3h r3l r4 r5r6

    ; Save expanded second operand to stack by pushing in reverse order

    push    r6                  ; mantissa lower word
    push    r5                  ; mantissa upper word
    push    r3                  ; flags, sign
    push    r4                  ; exponent

    ; At this point, stack looks like caller will see it:
    ;    sp+0  - second operand exponent
    ;    sp+2  - second operand flags
    ;    sp+3  - second operand sign
    ;    sp+4  - second operand mantissa
    ;    sp+8  - saved r6
    ;    sp+10 - saved r5
    ;    sp+12 - saved r4
    ;    sp+14 - caller's return address
    ;    sp+16 - second operand

    ; Put our return address back on stack

    push    r2

    ; Copy second operand flags/sign to r2

    mov.w   r3,r2

    ; Expand the first operand

    mov.w   r0,r5               ; copy first operand to r5r6
    mov.w   r1,r6
    jsr  ___expandsf            ; expand to r3h r3l r4 r5r6

    ; Return

    rts

;;
;; entry point: finishsf
;; input: return value in r0r1
;; output: cleans up stack, restoring r4 r5 r6 and leaving r0r1 intact
;;

    .global ___finishsf

___finishsf:

    ; Deallocate stack space

    add.b   #8,r7l
    addx.b  #0,r7h

    ; Restore registers

    pop     r6
    pop     r5
    pop     r4

    ; Return for caller

    rts
