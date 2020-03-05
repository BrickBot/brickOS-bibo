/*
 *  umodsi3.c
 *
 *  32-bit unsigned modulo: r0r1 %= r2r3
 *
 *  Calls the ROM version of udivsi3, which leaves the remainder in r3r4.
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
 *  The Original Code is Librcx code, released February 9, 1999.
 *
 *  The Initial Developer of the Original Code is Kekoa Proudfoot.
 *  Portions created by Kekoa Proudfoot are Copyright (C) 1999
 *  Kekoa Proudfoot. All Rights Reserved.
 *
 *  Contributor(s): Kekoa Proudfoot <kekoa@graphics.stanford.edu>
 */

__asm__ (
    ".section .text\n\t"
    ".global ___umodsi3\n"
    "___umodsi3:\n\t"
    "push r4\n\t"
    "push r5\n\t"
    "push r6\n\t"
    "mov.w r1,r6\n\t"
    "mov.w r0,r5\n\t"
    "mov.w r3,r4\n\t"
    "mov.w r2,r3\n\t"
    "jsr @@86\n\t"
    "mov.w r4,r1\n\t"
    "mov.w r3,r0\n\t"
    "pop r6\n\t"
    "pop r5\n\t"
    "pop r4\n\t"
    "rts"
);
