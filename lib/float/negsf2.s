/*
 *  negsf2.s
 *
 *  Floating point negate, single precision: -r0r1
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

; possible optimizations
;  - remove NaN check

;;
;; function: negsf2
;; input: float in r0r1
;; output: float in r0r1
;;

    .section .text

    .global ___negsf2

___negsf2:

    ; Negate operand

    xor.b   #0x80,r0h           ; flip sign bit of operand

return:

    rts
