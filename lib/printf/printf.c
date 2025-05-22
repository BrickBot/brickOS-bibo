/*! \file   printf.c
    \brief  printf function
    \author Matthew Ahrens <mahrens@acm.org>
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
 *  The Original Code is from the Brown Computer Science Department,
 *  Weenix support code.
 *
 *  The Initial Developer of the Original Code is David Powell.
 *
 *  Contributor(s): Matthew Ahrens <mahrens@acm.org>
 */

#include <stdio.h>
#include <stdarg.h>

static const char hexdigits[] = "0123456789ABCDEF";
static const char lhexdigits[] = "0123456789abcdef";
static const char empty_string[] = "(null string)";

/* This function takes an unsigned int and puts its ASCII
 * representation at a particular location in a buffer of given
 * length.  The radix used must also be provided, as well as a
 * string containing the characters used in the ASCII representation.
 *
 * This function makes no attempt to handle two-byte characters. 
 */
static int itoa(unsigned int value, char *dst, int pos, int len,
		int radix, const char *digits)
{
	char	sbuf[10];
	int	spos;

	spos = 0;
	while(value) {
		sbuf[spos] = value % radix;
		spos++;
		value /= radix;
	}
	if(!spos)
		*sbuf = 0, spos = 1;

	for(spos--; pos<len && spos>=0; pos++,spos--) {
		dst[pos] = digits[(int)sbuf[spos]];
	}

	return pos-1;
}


/* This function takes a buffer, its length, a format specification,
 * and a va_list, formats the provided data according the the format
 * spec, and places the result in the buffer.  It returns the length of
 * the output.  The only format options supported by this function
 * are:
 *   %d - decimal unsigned int
 *   %i -    "        "     "
 *   %x - hexidecimal unsigned int (lowercase letters)
 *   %X -      "          "     "   (uppercase letters)
 *   %c - character
 *   %s - string
 *   %% - '%' character
 */
int vsnprintf(char *dst, int len, const char *fmt, va_list arg)
{
	int		pos;
	int		cc;
	const char	*temp;
	unsigned int	scratch;

	for(pos=0, cc=0; (pos<len) && (cc=*fmt); pos++, fmt++) {
		if(cc!='%')
			dst[pos] = cc;
		else {
			cc = *(++fmt);
			switch(cc) {
			case 'd':
			case 'i':
				/* integer */
				scratch = va_arg(arg, unsigned);
				pos = itoa(scratch, dst, pos, len, 10,
					   hexdigits);
				break;
			case 'X':
				/* hexadecimal (uppercase) */
				scratch = va_arg(arg, unsigned);
				pos = itoa(scratch, dst, pos, len, 16,
					   hexdigits);
				break;
			case 'x':
				/* hexadecimal (lowercase) */
				scratch = va_arg(arg, unsigned);
				pos = itoa(scratch, dst, pos, len, 16,
					   lhexdigits);
				break;
			case 's':
				/* string */
				temp = va_arg(arg, const char*);
				if(!temp)
					temp = empty_string;
				for(; pos<len && *temp; pos++, temp++) {
					dst[pos] = *temp;
				}
				pos--;
				break;
			case 'c':
				/* character */
				dst[pos] = va_arg(arg, int);
				break;
			case '\0':
			case '%':
				dst[pos] = cc;
				break;
			}

			if(!cc) {
				break;
			}
		}
	}

	dst[pos++] = '\0';

	return pos;
}


/* This function prints the specified format into the provided buffer
 * taking care not to access beyond the end of the string. 
 */
char *snprintf(char *dst, int len, const char *fmt, ...)
{
	va_list	arg;

	va_start(arg, fmt);
	(void) vsnprintf(dst, len, fmt, arg);
	va_end(arg);

	return dst;
}

