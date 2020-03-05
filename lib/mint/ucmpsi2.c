/*
 *  ucmpsi2.c
 *
 *  Implementation of ucmpsi2, a 32-bit unsigned compare: r0r1 <=> r2r3
 *  Returns -1, 0, or 1, which might not be correct.
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
 *  The Initial Developer of the Original Code is Kekoa Proudfoot
 *  Portions created by Kekoa Proudfoot are Copyright (C) 1999
 *  Kekoa Proudfoot.  All Rights Reserved.
 *
 * Contributor(s): Kekoa Proudfoot <kekoa@graphics.stanford.edu>
 */

__asm__	("\n\
	.section .text\n\
\n\
	.global	___ucmpsi2\n\
\n\
___ucmpsi2:\n\
\n\
	sub.w	r3,r1\n\
	subx.b	r2l,r0l\n\
	subx.b	r2h,r0h\n\
\n\
	blo		2f\n\
\n\
	beq		1f         ; r0 is already zero\n\
\n\
	; First operand greater than second operand\n\
	mov.w	#1,r0\n\
\n\
1:\n\
	rts\n\
\n\
2:\n\
\n\
	; First operand less than second operand\n\
\n\
	mov.w	#-1,r0\n\
	rts\n\
");
