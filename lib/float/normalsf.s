/*
 *  normalsf.s
 *
 *  Normalizes a denormalized 1.29 mantissa by left shifting it
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
;; function: normalizesf
;; input: exp in r4, mant in r5r6, mant must be non-zero and in 1.29 format
;; output: exp in r4, mant in r5r6
;;

    .global ___normalizesf

___normalizesf:

    ; Left shift while one bit not set (final format is 1.29)

    while_0:

        ; Is one bit set? (one bit is in 1 << 29 or 20000000 position)

        btst    #5,r5h          ; load 20000000 bit of mantissa
        bne     endwhile_0      ; non-zero indicates bit set

        ; Shift mantissa one place to left

        add.w   r6,r6           ; use add to shift mantissa left 1 place
        addx.b  r5l,r5l
        addx.b  r5h,r5h

        ; Decrement exponent

        subs    #1,r4

        ; Repeat

        bra     while_0

    endwhile_0:

    rts
