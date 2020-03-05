/*! \file   fontdesign.c
    \brief  Rather spartan font design tool for a rather spartan display
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
 
#include <stdio.h>


void show_it(unsigned value) {
	printf("\n0x%-2x:  %c\n",value,(value&4?'_':'.'));
	printf("      %c%c%c\n",(value&8?'|':'.'),
			  (value&1?'_':'.'),
			  (value&2?'|':'.'));
	printf("      %c%c%c\n",(value&16?'|':'.'),
			  (value&32?'_':'.'),
			  (value&64?'|':'.'));
}
	  

int main() {
	int value=0;
	int norefresh=0;
	while(1) {
		norefresh=0;
		switch(getchar()) {
		case '2':
		case '3':
			value^=4;
			break;
		case 'q':
			value^=8;
			break;
		case 'e':
			value^=2;
			break;
		case 'w':
		case 's':
			value^=1;
			break;
		case 'a':
			value^=16;
			break;
		case 'y':
		case 'x':
		case 'c':
			value^=32;
			break;
		case 'd':
			value^=64;
			break;
		case ' ':
			if(value)
				value=0;
			else
				value=0x7f;
			break;
		case 27:
			return 0;
		default:
			norefresh=1;
		}
		if(!norefresh)
			show_it(value);
	}
}		
