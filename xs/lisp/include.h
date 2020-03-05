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

// wtobj.c
#ifdef DEBUG
void print_object(object);
#endif

// test.c
#ifdef DEBUG
void OK();
void ok();
void NG();
void ng();
void assert(int);
void putnum(int);
#ifdef JOINT
void printObject(Object);
#endif
#endif

// front.c

Object readObject(FILE *, int);
void writeObject(FILE*, Object);

static void transmit();
static void transmitCMD(object);

void ppInit();
void ppFinish();
void ppObject(Object);
Object ppExpr(Object);

void wtObject(Object, int);
void wtExpr(Object, int);

// ircom.c

//void sigio_handler(int signo);
