/*! \file   mulhi3.c
    \brief  16 bit multiplication routine
    \author Markus L. Noga <markus@noga.de>
*/
    
/*
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
 *  The Original Code is legOS code, released October 17, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 */

//! 16 bit signed multiplication */
/*! \param  a multiplicand
    \param  b multiplicand
    \return product
*/
int __mulhi3(int a,int b);

__asm__ ("\n\
.section .text\n\
.global ___mulhi3\n\
___mulhi3:\n\
      ; param   r0,r1\n\
      ; return  r0\n\
      ; clobber r2\n\
    \n\
      mov.w    r0,r2\n\
      mulxu.b  r1h,r2\n\
      mov.b    r0h,r2h\n\
      mulxu.b  r1l,r0\n\
      add.b    r2l,r0h\n\
      mulxu.b  r2h,r1\n\
      add.b    r1l,r0h\n\
      rts\n\
");
