/*! \file   strcmp.c
    \brief  strcmp function
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

//! Compare two strings
/*! \param  s1 first string
    \param  s2 second string
    \return <0: s1<s2, ==0: s1==s2, >0: s1>s2
*/    
int strcmp(const char *s1,const char *s2) {
  while(*s1==*s2 && *s1!=0) {
    s1++;
    s2++;
  }
  if(*s1==*s2)
    return 0;
  if(*s1<*s2)
    return -1;
  return 1;
}

