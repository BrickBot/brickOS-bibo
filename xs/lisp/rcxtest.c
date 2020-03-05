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
 *  The Original Code is XS code, released February 4, 2004.
 *
 *  The Initial Developer of the Original Code is Taiichi Yuasa.
 *  Portions created by Taiichi Yuasa are Copyright (C) 2004
 *  Taiichi Yuasa. All Rights Reserved.
 *
 *  Contributor(s): Taiichi Yuasa <yuasa@kuis.kyoto-u.ac.jp>
 */

#include <conio.h>
#include <dsound.h>
#include <unistd.h>
#include <stdlib.h>

static const note_t OKsound[] = { 
  { PITCH_C5, 3 },
  { PITCH_END, 0}
};
static const note_t oksound[] = { 
  { PITCH_C5, 1 },
  { PITCH_END, 0}
};
static const note_t NGsound[] = { 
  { PITCH_C1, 3 },
  { PITCH_END, 0}
};
static const note_t ngsound[] = { 
  { PITCH_C1, 1 },
  { PITCH_END, 0}
};
void *ps[10];

void OK() {
	dsound_play(OKsound);
    dsound_wait();
}

void ok() {
	dsound_play(oksound);
    dsound_wait();
}

void NG() {
	dsound_play(NGsound);
    dsound_wait();
}

void ng() {
	dsound_play(ngsound);
    dsound_wait();
}

void assert(int x) {
	if(x) OK(); else NG();
}

void putnum(int x) {
	cls();
	cputc_1(x%10+'0');
	x /= 10;
	cputc_2(x%10+'0');
	x /= 10;
	cputc_3(x%10+'0');
	x /= 10;
	cputc_4(x+'0');
}

void memtest() {
  int i, pp=0;
  for (i=10; i>0; ) {
    void *p = malloc(i*1024);
    cputc_4(i/10+'0');
    cputc_3(i%10+'0');
    if (p) {
      ps[pp++] = p;
      cputc_2('-');
      OK();
    } else {
      cputc_2(' ');
      ng();
      i--;
    }
  }
  while (pp>0)
    free(ps[--pp]);
}

static const note_t first[] = { 
  { PITCH_C5, 1 }, // do
  { PITCH_D5, 1 }, // re
  { PITCH_E5, 2 }, // mi
  { PITCH_D5, 1 },
  { PITCH_C5, 1 },
  { PITCH_END, 0 }
};
            
static const note_t second[] = { 
  { PITCH_C5, 1 },
  { PITCH_D5, 1 },
  { PITCH_E5, 1 },
  { PITCH_D5, 1 },
  { PITCH_C5, 1 },
  { PITCH_D5, 3 },
  { PITCH_END, 0 }
};
