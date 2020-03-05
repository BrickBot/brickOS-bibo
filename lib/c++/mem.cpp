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
 *  Contributor(s): Henner Zeller <H.Zeller@acm.org>
 */

#include <stdlib.h>	// for malloc(), free(), size_t def'ns

// if we are using 3.x compiler then define new style new/delete
#if __GNUC__ >= 3

void* operator new(size_t size) {
  return malloc(size);
}


void* operator new[] (size_t size) {
  return malloc(size);
}


void operator delete (void *p) {
  free(p);
}


void operator delete[] (void *p) {
  free(p);
}

#endif

