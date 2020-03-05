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

#ifdef RCX
#include <config.h>
// Due to the size of XS, we will allow building without CONF_ASCII and instead just limit character and string functionality
#ifndef CONF_ASCII
#warning CONF_ASCII is disabled, so string and character functions will only output preset characters on the RCX
#endif
#endif

#ifndef RCX
#include <stdio.h>
#endif

#include <stdlib.h>
#include "object.h"

#ifdef LISTLIB
//#define HPSIZE 512        // size of the heap, in cells
#define HPSIZE 1152        // size of the heap, in cells
#else
//#define HPSIZE 640        // size of the heap, in cells
#define HPSIZE 1280        // size of the heap, in cells
#endif

#define VSSIZE 256        // size of value_stack[]
#define BUFSIZE (256 - 3) // size of wtbuf[] and rdbuf[]

#ifdef RCX
#define WDBITS 16
#else
#define WDBITS 32
#endif

object value_stack[VSSIZE], *vs_base, *vs_top;
object *vs_bottom = value_stack - 1;
object *vs_limit = value_stack + VSSIZE - 1;
object heap0[HPSIZE*2+1], *heap;
int bit_table[HPSIZE/WDBITS];

object free_list;
object symbol_table;
#ifndef ONLINE
int symbol_count;
#endif
object dummy_lambda;
object last_val = NIL;
int last_error_number;
object last_error_val = NIL;
object last_backtrace = NIL;

struct catcher {
	object tag;
	struct catcher *prev;
};

struct catcher *last_catcher = 0;
struct catcher *throw_catcher = 0;
object throw_value = UNDEFINED;

struct watcher {
	object env, cond, body;
	struct watcher *prev;
};

struct watcher *last_watcher = 0;
struct watcher *active_watcher = 0;

object Eerror(int n, object x) {
	last_error_number = n;
	last_error_val = x;
	last_backtrace = NIL;
	return ERR;
}

object Evs_overflow() {
	return Eerror(EXvs_overflow, valINT(vs_top - vs_bottom));
}

unsigned char wtbuf[BUFSIZE];
unsigned char rdbuf[BUFSIZE];
unsigned char *wtbufp;
const unsigned char *rdbufp;

#ifdef RCX

#include <dsound.h>
#include <conio.h>
#include <dsensor.h>
#include <dmotor.h>
#include <sys/battery.h>
#include <time.h>

static note_t sound_buffer[32];

int random_initialized = 0;

time_t base_time;

void beep() {
    dsound_play(dsound_system_sounds[0]);
    dsound_wait();
}

#else
char lcd[5] = "     ";
void cputc(int c, int i) {
	if (i >= 0 && i <= 4)
		lcd[4-i] = c;
}
void show_lcd() {
	printf("[%s]", lcd);
}
void cls() {
	int i;
	for (i = 0; i <= 4; i++)
		lcd[i] = ' ';
}
#endif

#ifdef JOINT
#ifdef Native_Win32
 #include <windows.h>
#else
 #include <sys/time.h>
#endif

struct timeval base_time;

#endif

#ifdef ONLINE
int interrupt_flag = 0;
#ifdef JOINT
void notify_interrupt() {
	interrupt_flag = 1;
}

int interrupted() {
	if (interrupt_flag) {
		interrupt_flag = 0;
//		rdbuf_ready = 0;
		Eerror(EXinterrupt, UNDEFINED);
		return 1;
	} else
		return 0;
}

#endif
#endif

#ifdef IRCOM
#include <lnp.h>

int rdbuf_ready = 0;
unsigned char com_src;

int ack_flag = 0;

void input_handler(const unsigned char *buf, unsigned char len,
                    unsigned char src)
{
	com_src = src;
	if (*buf == CMDACK)
		ack_flag = 1;
	else if (*buf == CMDINTERRUPT)
		interrupt_flag = 1;
	else {
		int i;
		for (i = 0; i < len; i++)
			rdbuf[i] = buf[i];
		rdbufp = rdbuf;
		rdbuf_ready = 1;
	}
}

static int transmit1(int ntries) {
	int len = wtbufp-wtbuf;
	rdbuf_ready = 0;
	while (--ntries >= 0) {
		ack_flag = 0;
		lnp_addressing_write(wtbuf, len, com_src, 1);
        // NOTE: Timout here must be greater than the timeout on the host
		msleep(1000+(5*len));
		if (ack_flag)
			return 1;
	}
	//interrupt_flag = 1;
	return 0;
}

static void transmit() {
	transmit1(10);
}

static void transmitCMD(object cmd) {
	wtbuf[0] = cmd;
	wtbufp = wtbuf + 1;
	transmit();
}

wakeup_t rdbuf_ready_p(wakeup_t data) {
	return rdbuf_ready || interrupt_flag;
}

#define wait_rdbuf_ready() wait_event(rdbuf_ready_p, 0)

#endif

#ifdef RCX

object check_watcher();

time_t next_watcher_polling;
int checking_watcher = 0;

#define POLLING_INTERVAL 100 // msec

int interrupted() {
#ifdef IRCOM
	if (dkey == KEY_VIEW || interrupt_flag)
#else
	if (dkey == KEY_VIEW)
#endif
	{
#ifdef IRCOM
		interrupt_flag = 0;
		rdbuf_ready = 0;
#endif
		Eerror(EXinterrupt, UNDEFINED);
		return 1;
	} else {
		if (!checking_watcher && get_system_up_time() >= next_watcher_polling) {
			next_watcher_polling = get_system_up_time() + POLLING_INTERVAL;
			if (check_watcher() == ERR)
				return 1;
		}
		return 0;
	}
}

#endif

#ifdef JOINT
static void transmit() {
	receive0(wtbuf, wtbufp - wtbuf);
}

static void transmitCMD(object cmd) {
	wtbuf[0] = cmd;
	receive0(wtbuf, 1);
}

#define wait_rdbuf_ready() rdbufp = rdbuf

#endif

#ifdef DEBUG
#ifndef JOINT
#ifdef RCX
#include "rcxtest.c"
#else
#include "test.c"
#endif
#endif
#endif

#ifdef ONLINE
static int checkBufferSpace(int n) {
	int flag = 0;
	if (wtbufp >= wtbuf + BUFSIZE - n) {
		*wtbufp++ = CMDMORE;
		transmit();
		wait_rdbuf_ready();
		if (*rdbufp != CMDMORE)
			flag = 1;
		wtbufp = wtbuf;
	}
	return flag;
}

static int wt(object e) {
	if (COMMANDP(e)) {
		if (checkBufferSpace(1)) return 1;
		*wtbufp++ = e & 0xff;
	} else {
		if (LAMBDAP(e) && GVARP(LAMBDAenv(e)))
			e = MKLAMBDA(REF2CELL(LAMBDAenv(e)));
#ifdef RCX
		if (checkBufferSpace(2)) return 1;
		*wtbufp++ = e & 0xff;
		*wtbufp++ = e >> 8;
#else
		if (checkBufferSpace(4)) return 1;
		*wtbufp++ = e & 0xff;
		*wtbufp++ = e >> 8;
		*wtbufp++ = e >> 16;
		*wtbufp++ = e >> 24;
#endif
	}
	return 0;
}

static int wtCMD(object e) {
	if (checkBufferSpace(1)) return 1;
	*wtbufp++ = e & 0xff;
	return 0;
}

int write0(object e) {
	if (PAIRP(e)) {
		int n = 0;
		do {
			if (write0(CAR(e))) return 1;
			e = CDR(e);
			n++;
		} while (PAIRP(e));
		if (e == NIL)
			switch (n) {
			case 1: return wtCMD(CMDLIST1);
			case 2: return wtCMD(CMDLIST2);
			case 3: return wtCMD(CMDLIST3);
			case 4: return wtCMD(CMDLIST4);
			default: if (wtCMD(CMDLIST4)) return 1; n -= 4;
			}
		else
			if (write0(e)) return 1;
		while (1)
			switch (n) {
			case 1: return wtCMD(CMDCONS);
			case 2: return wtCMD(CMDLISTA2);
			case 3: return wtCMD(CMDLISTA3);
			case 4: return wtCMD(CMDLISTA4);
			default: if (wtCMD(CMDLISTA4)) return 1; n -= 4;
			}
	} else
		return wt(e);
}

void write1(object e, object cmd) {
	wtbufp = wtbuf;
	if (write0(e)) return;
	wtCMD(cmd);
	transmit();
}

void write2(object e1, object e2, object cmd) {
	wtbufp = wtbuf;
	if (write0(e1)) return;
	if (write0(e2)) return;
	wtCMD(cmd);
	transmit();
}

#else
static void wt(object e) {
	*wtbufp++ = e & 0xff;
	if (!COMMANDP(e)) {
		*wtbufp++ = e >> 8;
#ifndef RCX
		*wtbufp++ = e >> 16;
		*wtbufp++ = e >> 24;
#endif
	}
}

static void wtCMD(object e) {
	*wtbufp++ = e & 0xff;
}
#endif

#define MARKED(cell) (bit_table[((((object *)cell)-heap)/2)/WDBITS] \
                       &(1<<(((((object *)cell)-heap)/2)%WDBITS)))
#define MARK(cell) (bit_table[((((object *)cell)-heap)/2)/WDBITS] \
                     |=(1<<(((((object *)cell)-heap)/2)%WDBITS)))

void mark(object p) {
	object *top = vs_top;

	while (1) {
		if (REFP(p)) {
			p = REF2CELL(p);
			if (!MARKED(p)) {
				MARK(p);
				*++top = CDR(p);
				p = CAR(p);
				continue;
			}
		}
		if (top > vs_top)
			p = *top--;
		else
			return;
	}
}

int gc (){
	{
		int *btp;
		for (btp = bit_table; btp < bit_table + HPSIZE/WDBITS; btp++)
			*btp = 0;
	}
	// mark phase
	mark(dummy_lambda);
	mark(last_val);
	mark(last_error_val);
	mark(last_backtrace);
	{
		struct catcher *p;
		for (p = last_catcher; p; p = p->prev)
			mark(p->tag);
	}
	mark(throw_value);
	{
		struct watcher *p;
		for (p = last_watcher; p; p = p->prev) {
			mark(p->env);
			mark(p->cond);
			mark(p->body);
		}
	}
	{
		object p;
		for (p = symbol_table; p != NIL; p = CAR(p))
			mark(p);
	}
	{
		object *p;
		for (p = value_stack; p <= vs_top; p++)
			mark(*p);
	}
	// sweep phase
	free_list = NIL;
	{
		int n = 0;
		object *p;
		for (p = heap; p < heap + HPSIZE*2; p += 2)
			if (!MARKED((object) p)) {
				*p = free_list;
				free_list = (object) p;
				n++;
			}
		return n;
	}
}

object alloc_cell() {
	if (free_list != NIL) {
		object p = free_list;
		free_list = CAR(free_list);
		return p;
	}
	gc();
	if (free_list != NIL) {
		object p = free_list;
		free_list = CAR(free_list);
		return p;
	}
	last_error_number = EXheap_full;
	last_error_val = UNDEFINED;
	last_backtrace = NIL;
	return ERR;
}

object make_cons(object a, object d) {
	object x = alloc_cell();
	if (x != ERR) {
		CAR(x) = a;
		CDR(x) = d;
	}
	return x;
}

int vs_cons() {
	object x = alloc_cell();
	if (x != ERR) {
		CDR(x) = *vs_top--;
		CAR(x) = *vs_top;
		*vs_top = x;
		return 0;
	} else
		return 1;
}

int check_int_args(object *base) {
	for (; base <= vs_top; base++)
		if (!INTP(*base)) {
			Eerror(EXnot_integer, *base);
			return 1;
		}
	return 0;	
}

object eval(object);
object toplevel();

int eval_push(object x) {
	if ((x = eval(x)) == ERR)
		return 1;
	if (vs_top >= vs_limit) {
		Evs_overflow();
		return 1;
	}
	*++vs_top = x;
	return 0;
}

#define STACKWORDS (STACKSIZE/2) // size of the C stack
#define FRAMESIZE 20 // size of stack frame of eval, measured by experiment
int *stack_limit;

object compare(object *base, int gt, int eq, int lt) {
	object last;
	if (check_int_args(base)) return ERR;
	last = *vs_top--;
	for (; vs_top >= base; vs_top--) {
		if (*vs_top > last) {
			if (!gt) return FALSE;
		} else if (*vs_top == last) {
			if (!eq) return FALSE;
		} else {
			if (!lt) return FALSE;
		}
		last = *vs_top;
	}
	return TRUE;
}

#ifdef RCX

object check_watcher() {
	struct watcher *p = last_watcher;
	object *old_base = vs_base;
	vs_base = ++vs_top;

	for (; p; p = p->prev)
		if (p == active_watcher)
			break;
		else {
			object val;
			*vs_base = p->env;
			checking_watcher = 1;
			val = eval(p->cond);
			checking_watcher = 0;
			if (val == ERR)
				goto LERROR;
			else if (val != FALSE) {
				object body;
				struct watcher *prev_active = active_watcher;
				active_watcher = p;
				for (body = p->body; body != NIL; body = CDR(body))
					if (eval(CAR(body)) == ERR) {
						active_watcher = prev_active;
						goto LERROR;
					}
				active_watcher = prev_active;
				break;
			}
		}
	vs_top = vs_base - 1;
	vs_base = old_base;
	return NIL;

LERROR:
	if (throw_value == UNDEFINED && last_error_number != EXheap_full)
		last_backtrace = make_cons(NIL, last_backtrace);
	return ERR;
}

#endif

object with_watcher(object env, object ws, object body) {
	object val = NIL;
	struct watcher w;
	w.env = env;
	w.cond = CAR(CAR(ws));
	w.body = CDR(CAR(ws));
	w.prev = last_watcher;
	last_watcher = &w;

	ws = CDR(ws);
	if (ws != NIL)
		val = with_watcher(env, ws, body);
	else
		for (; body != NIL; body = CDR(body))
			if ((val = eval(CAR(body))) == ERR)
				break;
	last_watcher = w.prev;
	return val;
}

#ifdef RCX
volatile unsigned *get_port(object x) {
	if (INTP(x)) {
		if (x == valINT(1))
			return &SENSOR_1;
		else if (x == valINT(2))
			return &SENSOR_2;
		else if (x == valINT(3))
			return &SENSOR_3;
	}
	Eerror(EXbad_port_number, x);
	return 0;
}
#else
volatile unsigned *get_port(object x) {
	if (INTP(x)) {
		if (x == valINT(1))
			return (unsigned *)1;
		else if (x == valINT(2))
			return (unsigned *)2;
		else if (x == valINT(3))
			return (unsigned *)3;
	}
	Eerror(EXbad_port_number, x);
	return 0;
}
#endif

#ifdef Native_Win32
int oto2freq[]={
//  C    Cm   D    Dm   E    F    Fm   G    Gm   A    Am   H
                                                28,  29,  31,//0
   33,  35,  37,  39,  41,  44,  46,  49,  52,  55,  58,  62,//1
   65,  69,  73,  78,  82,  87,  92,  98, 104, 110, 117, 123,//2
  131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247,//3
  262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,//4
  523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,//5
 1046,1109,1175,1244,1318,1397,1480,1568,1661,1760,1865,1976,//6
 2093,2218,2349,2489,2637,2794,2960,3136,3322,3520,3729,3951,//7
 4186,4435,4699,4978,5274,5588,5920,6272,6645,7040           //8
};

void play_note(int oto, int len) {
    if (len > 0) {
        if (oto >= 0 && oto < 97)
            Beep(oto2freq[oto], len*200);
        else
            usleep(len*200000);
    }
}
#endif

object eval(object e) {

	if (&e < stack_limit)
		return Eerror(EXstack_overflow, valINT(stack_limit - &e));
LOOP:

#ifdef RCX
	if (interrupted()) return ERR;
#endif
#ifdef JOINT
	if (interrupted()) return ERR;
#endif
	if (vs_top >= vs_limit)
		return Evs_overflow();

	if (PAIRP(e)) {
		object fun = CAR(e);
		int tail_call = 0;
		e = CDR(e);

		if (SPECIALP(fun))
			switch (SUBRindex(fun)) {
			case Fquote:
				return e;

			case Flambda: {
				e = make_cons(FlambdaLOC(fun) == 0
				               ? (GVARP(LAMBDAenv(vs_base[0]))
				                   ? NIL
				                   : LAMBDAenv(vs_base[0]))
				               : vs_base[FlambdaLOC(fun)],
				               e);
				if (e == ERR) goto LERROR;
				return MKLAMBDA(e);
			}
			case Fset: {
				object val = eval(CDR(e));
				if (val == ERR) goto LERROR;
				e = CAR(e);
				if (GVARP(e)) {
					if (GVARval(e) == UNDEFINED) {
						Eerror(EXundefined_var, e);
						goto LERROR;
					}
					GVARval(e) = val;
				} else {
					switch (CONSTindex(e)) {
					case Vref:
						vs_base[VrefLOC(e)] = val;
						break;
					case VrefCAR:
						CAR(vs_base[VrefLOC(e)]) = val;
						break;
					case VrefCCB: {
						int n = VrefLOC(e);
						e = LAMBDAenv(vs_base[0]);
						for (; n > 0; n--)
							e = CDR(e);
						CAR(e) = val;
						break;
					}
					}
				}
				return val;
			}
			case Fif: {
				object val = eval(CAR(e));
				if (val == ERR)
					goto LERROR;
				else if (val != FALSE)
					e = CAR(CDR(e));
				else
					e = CDR(CDR(e));
				goto LOOP;
			}
			case Fbegin:
				for (; CDR(e) != NIL; e = CDR(e))
					if (eval(CAR(e)) == ERR)
						goto LERROR;
				e = CAR(e);
				goto LOOP;

			case Flet:
			case FletA:
			case Fletrec: {
				object *top = vs_top, x;
				for (x = CAR(e); x != NIL; x = CDR(x))
					if (eval_push(CAR(x))) goto LERROR;
				x = NIL;
				while ((e = CDR(e)) != NIL) {
					if (CDR(e) == TRUE) {
						e = CAR(e);
						goto LOOP;
					}
					if ((x = eval(CAR(e))) == ERR) goto LERROR;
				}
				vs_top = top;
				return x;
			}
			case Fand:
				for (; CDR(e) != NIL; e = CDR(e)) {
					object val = eval(CAR(e));
					if (val == ERR)
						goto LERROR;
					else if (val == FALSE)
						return FALSE;
				}
				e = CAR(e);
				goto LOOP;

			case For:
				for (; CDR(e) != NIL; e = CDR(e)) {
					object val = eval(CAR(e));
					if (val == ERR)
						goto LERROR;
					else if (val != FALSE)
						return val;
				}
				e = CAR(e);
				goto LOOP;

			case Fcatch: {
				struct catcher c;
				object val = eval(CAR(e));
				if (val == ERR) goto LERROR;
				c.tag = val;
				c.prev = last_catcher;
				last_catcher = &c;

				val = NIL;
				while ((e = CDR(e)) != NIL)
					if ((val = eval(CAR(e))) == ERR) {
						if (throw_catcher == &c) {
							val = throw_value;
							throw_catcher = 0;
							throw_value = UNDEFINED;
							break;
						}
						last_catcher = c.prev;
						goto LERROR;
					}
				last_catcher = c.prev;
				return val;
			}

			case Fwith_watcher: {
				object x;
				if ((x = make_cons(MKLAMBDAnargs(0,0), NIL)) == ERR)
					return ERR;
				*++vs_top = x;
				if ((x = alloc_cell()) == ERR) return ERR;
				CAR(x) = Fwith_watcherLOC(fun) == 0
				         ? LAMBDAenv(vs_base[0])
				         : vs_base[Fwith_watcherLOC(fun)];
				CDR(x) = *vs_top--;
				x = MKLAMBDA(x);
				if ((x = with_watcher(x, CAR(e), CDR(e))) == ERR)
					goto LERROR;
				return x;
			}
			case Fwait_until: {
				object val;
				while ((val = eval(e)) == FALSE) {
#ifdef RCX
					if (interrupted()) goto LERROR;
					msleep(50);
#else
#ifdef JOINT
					if (interrupted()) goto LERROR;
#endif
					usleep(50000);
#endif
				}
				if (val == ERR) goto LERROR;
				return val;
			}
			case Fhoist:
				// If an error is detected, return ERR rather than
				// goto LERROR, so that Fhoist is not included
				// in the backtrace.
				if ((e = eval(e)) == ERR) return ERR;
				*++vs_top = e;
				if ((e = alloc_cell()) == ERR) return ERR;
				CAR(e) = *vs_top--;
				CDR(e) = FhoistLOC(fun) == 0 ? LAMBDAenv(vs_base[0])
				                             : vs_base[FhoistLOC(fun)];
				return e;

			case Fhoist_arg: {
				// If an error is detected, return ERR rather than
				// goto LERROR, so that Fhoist-arg is not included
				// in the backtrace.
				int loc = INTval(e);
				vs_base[loc] = make_cons(vs_base[loc],
				                          FhoistLOC(fun) == 0
				                          ? LAMBDAenv(vs_base[0])
				                          : vs_base[FhoistLOC(fun)]);
				if (vs_base[loc] == ERR) return ERR;
				return NIL;
			}
			case Ftail_call:
				tail_call = 1;
				fun = CAR(e);
				e = CDR(e);
				break;

			default:
				Eerror(EXbaboon, fun);
				goto LERROR;
			}

		{
		object *base = vs_top + 1;
		fun = eval(fun);
		if (fun == ERR)
			return ERR;
		else if (SUBRP(fun)) {
			for (; e != NIL; e = CDR(e))
				if (eval_push(CAR(e))) return ERR;
			goto SUBR_CALL;
		} else if (LAMBDAP(fun)) {
			*++vs_top = fun;
			for (; e != NIL; e = CDR(e))
				if (eval_push(CAR(e))) return ERR;
			goto LAMBDA_CALL;
		} else
			return Eerror(EXnot_function, fun);

	SUBR_CALL:
		if (vs_top - base + 1 < SUBRnreq(fun)) {
			Eerror(EXtoo_few_args, fun);
			goto LERROR;
		} else if (!SUBRrestp(fun) && vs_top - base + 1 > SUBRnmax(fun)) {
			Eerror(EXtoo_many_args, fun);
			goto LERROR;
		}
		switch (SUBRindex(fun)) {
		case Lcar:
			e = base[0];
			if (!PAIRP(e)) goto NOTPAIR;
			e = CAR(e);
			break;

		case Lcdr:
			e = base[0];
			if (!PAIRP(e)) goto NOTPAIR;
			e = CDR(e);
			break;

		case Lcons:
			if ((e = make_cons(base[0], base[1])) == ERR) goto LERROR;
			break;

		case Lset_car:
			e = base[0];
			if (!PAIRP(e)) goto NOTPAIR;
			e = CAR(e) = base[1];
			break;

		case Lset_cdr:
			e = base[0];
			if (!PAIRP(e)) goto NOTPAIR;
			e = CDR(e) = base[1];
			break;

		case LpairP:
			e = PAIRP(base[0]) ? TRUE : FALSE;
			break;

		case LnullP:
			e = (base[0] == NIL) ? TRUE : FALSE;
			break;

		case Lnot:
			e = (base[0] == FALSE) ? TRUE : FALSE;
			break;

		case Llist:
			*++vs_top = NIL;
			// through
#ifdef LISTLIB
		case LlistA:
#endif
			while (vs_top > base)
				if (vs_cons()) goto LERROR;
			e = base[0];
			break;
#ifdef LISTLIB
		case Llist_ref: {
			int n;
			e = base[1];
			if (!INTP(e)) goto NOTINTEGER;
			n = INTval(e);
			e = base[0];
			while (1) {
				if (e == NIL) break;
				if (!PAIRP(e)) goto NOTLIST;
				if (n <= 0) {
					e = CAR(e);
					break;
				}
				n--;
				e = CDR(e);
			}
			break;
		}
		case Lappend: {
			object *p, x;
			*vs_top = x = make_cons(*vs_top, NIL);
			if (x == ERR) goto LERROR;
			for (p = base; p < vs_top; p++)
				for (e = *p; e != NIL; e = CDR(e)) {
					if (!PAIRP(e)) goto NOTLIST;
					x = CDR(x) = make_cons(CAR(e), NIL);
					if (x == ERR) goto LERROR;
				}
			CDR(x) = CAR(*vs_top);
			e = CDR(*vs_top);
			break;
		}
		case Lassoc:
			for (e = base[1]; e != NIL; e = CDR(e)) {
				if (!PAIRP(e) || !PAIRP(CAR(e))) goto BADALIST;
				if (CAR(CAR(e)) == base[0]) {
					e = CAR(e);
					break;
				}
			}
			break;

		case Lmember:
			for (e = base[1]; e != NIL; e = CDR(e)) {
				if (!PAIRP(e)) goto NOTLIST;
				if (CAR(e) == base[0])
					break;
			}
			break;

		case Llength: {
			int n = 0;
			for (e = base[0]; e != NIL; e = CDR(e)) {
				if (!PAIRP(e)) goto NOTLIST;
				n++;
			}
			e = valINT(n);
			break;
		}
		case Lreverse:
			*++vs_top = NIL;
			for (e = base[0]; e != NIL; e = CDR(e)) {
				if (!PAIRP(e)) goto NOTLIST;
				if ((*vs_top = make_cons(CAR(e), *vs_top)) == ERR)
					goto LERROR;
			}
			e = *vs_top;
			break;
#endif

		case Leq:
			e = (base[0] == base[1]) ? TRUE : FALSE;
			break;

		case LbooleanP:
			e = (base[0] == TRUE || base[0] == FALSE) ? TRUE : FALSE;
			break;

		case Lplus:
			if (check_int_args(base)) goto LERROR;
			e = 0;
			for (; vs_top >= base; vs_top--)
				e += INTval(*vs_top);
			e = valINT(e);
			break;

		case Lminus:
			if (check_int_args(base)) goto LERROR;
			e = INTval(base[0]);
			if (vs_top == base)
				e = - e;
			else
				for (; vs_top > base; vs_top--)
					e -= INTval(*vs_top);
			e = valINT(e);
			break;

		case Ltimes:
			if (check_int_args(base)) goto LERROR;
			e = 1;
			for (; vs_top >= base; vs_top--)
				e *= INTval(*vs_top);
			e = valINT(e);
			break;

		case Lquotient:
			if (check_int_args(base)) goto LERROR;
			if (base[1] == valINT(0)) {
				Eerror(EXbad_number, base[1]);
				goto LERROR;
			}
			e = valINT(INTval(base[0])/INTval(base[1]));
			break;

		case Lremainder:
			if (check_int_args(base)) goto LERROR;
			if (base[1] == valINT(0)) {
				Eerror(EXbad_number, base[1]);
				goto LERROR;
			}
			e = valINT(INTval(base[0])%INTval(base[1]));
			break;

		case LLT:
			if ((e = compare(base, 0, 0, 1)) == ERR) goto LERROR;
			break;

		case LGT:
			if ((e = compare(base, 1, 0, 0)) == ERR) goto LERROR;
			break;

		case LEQ:
			if ((e = compare(base, 0, 1, 0)) == ERR) goto LERROR;
			break;

		case LLE:
			if ((e = compare(base, 0, 1, 1)) == ERR) goto LERROR;
			break;

		case LGE:
			if ((e = compare(base, 1, 1, 0)) == ERR) goto LERROR;
			break;

		case Llogand:
			if (check_int_args(base)) goto LERROR;
			e = base[0]&base[1];
			break;

		case Llogior:
			if (check_int_args(base)) goto LERROR;
			e = base[0]|base[1];
			break;

		case Llogxor:
			if (check_int_args(base)) goto LERROR;
			e = (base[0]^base[1])|INTTAG;
			break;

		case Llogshl:
			if (check_int_args(base)) goto LERROR;
			e = ((base[0]^INTTAG)<<INTval(base[1]))|INTTAG;
			break;

		case Llogshr:
			if (check_int_args(base)) goto LERROR;
			e = ((base[0]^INTTAG)>>INTval(base[1]))|INTTAG;
			break;

		case Lrandom:
			e = base[0];
			if (!INTP(e)) goto NOTINTEGER;
			e = INTval(e);
			if (e <= 0) {
				Eerror(EXbad_number, base[0]);
				goto LERROR;
			}
#ifdef RCX
			if (!random_initialized) {
				srandom(1);
				random_initialized = 1;
			}
			e = valINT(random()%e);
#else
			e = valINT(rand()%e);
#endif
			break;

		case LintegerP:
			e = INTP(base[0]) ? TRUE : FALSE;
			break;

		case Ltrace_call:
#ifdef ONLINE
			write2(base[0], base[2], CMDTRACEIN);
			wait_rdbuf_ready();
			if (toplevel() == ERR) goto LERROR;
#endif
			if (vs_top >= vs_limit - 1) {
				Evs_overflow();
				goto LERROR;
			}
			*++vs_top = base[1];
			*++vs_top = base[2];
			if ((e = eval(MKSPECIAL(InternalApply))) == ERR) goto LERROR;
#ifdef ONLINE
			write2(base[0], e, CMDTRACEOUT);
			wait_rdbuf_ready();
			if (toplevel() == ERR) goto LERROR;
#endif
			break;

		INTERNAL_APPLY:
			fun = CONST2SUBR(MKCONST(Lapply,1,0,1));
			tail_call = 0;
			base = vs_top - 1;
		case Lapply:
			for (e = *vs_top--; e != NIL; e = CDR(e)) {
				if (!PAIRP(e)) goto NOTLIST;
				if (vs_top >= vs_limit) {
					Evs_overflow();
					goto LERROR;
				}
				*++vs_top = CAR(e);
			}
			e = base[0];
			if (SUBRP(e)) {
				object *p = base - 1;
				vs_top--;
				for (; p <= vs_top; p++)
					p[0] = p[1];
				fun = e;
				goto SUBR_CALL;
			} else if (LAMBDAP(e)) {
				fun = e;
				goto LAMBDA_CALL;
			} else {
				Eerror(EXnot_function, e);
				goto LERROR;
			}
		case LfunctionP:
			e = PROCP(base[0]) ? TRUE : FALSE;
			break;

		case LsymbolP:
			e = SYMBOLP(base[0]) ? TRUE : FALSE;
			break;

		case Lthrow: {
			struct catcher *p;
			for (p = last_catcher; p; p = p->prev)
				if (p->tag == base[0]) {
					throw_catcher = p;
					throw_value = base[1];
					return ERR;
				}
			Eerror(EXno_catcher, base[0]);
			goto LERROR;
		}
		case Lgc:
			e = valINT(gc());
			break;

		case Lmotor:
			if (check_int_args(base) || !get_port(base[0])) goto LERROR;
#ifdef RCX
			switch (base[0]) {
			case valINT(1):
				motor_a_dir(INTval(base[1])); break;
			case valINT(2):
				motor_b_dir(INTval(base[1])); break;
			default:
				motor_c_dir(INTval(base[1])); break;
			}
#endif
			e = base[1];
			break;

		case Lspeed:
			if (check_int_args(base) || !get_port(base[0])) goto LERROR;
#ifdef RCX
			switch (base[0]) {
			case valINT(1):
				motor_a_speed(INTval(*vs_top)); break;
			case valINT(2):
				motor_b_speed(INTval(*vs_top)); break;
			default:
				motor_c_speed(INTval(*vs_top)); break;
			}
#endif
			e = base[1];
			break;

		case Llight_on: {
			volatile unsigned *port;
			if (!(port = get_port(e = base[0]))) goto LERROR;
#ifdef RCX
			ds_active(port);
#endif
			break;
		}
		case Llight_off: {
			volatile unsigned *port;
			if (!(port = get_port(e = base[0]))) goto LERROR;
#ifdef RCX
			ds_passive(port);
#endif
			break;
		}
		case Llight: {
			volatile unsigned *port;
			if (!(port = get_port(base[0]))) goto LERROR;
#ifdef RCX
			// The LIGHT macro of LegOS 0.2.3 does not work.
			// The code below was borrowed from LegOS 0.2.4.
			e = valINT(147 - ds_scale(*port)/7);
#else
			e = valINT(1);
#endif
			break;
		}
		case Lrotation_on: {
			volatile unsigned *port;
			if (!(port = get_port(e = base[0]))) goto LERROR;
#if defined(RCX) && defined(CONF_DSENSOR_ROTATION)
			ds_active(port);
			ds_rotation_on(port);
			ds_rotation_set(port, 0);
			msleep(100);
#endif
			break;
		}
		case Lrotation_off: {
			volatile unsigned *port;
			if (!(port = get_port(e = base[0]))) goto LERROR;
#if defined(RCX) && defined(CONF_DSENSOR_ROTATION)
			ds_passive(port);
			ds_rotation_off(port);
#endif
			break;
		}
		case Lrotation:
			if (!get_port(base[0])) goto LERROR;
#if defined(RCX) && defined(CONF_DSENSOR_ROTATION)
			e = valINT(base[0] == valINT(1) ? ROTATION_1
			            : (base[0] == valINT(2) ? ROTATION_2 : ROTATION_3));
#else
			e = valINT(0);
#endif
			break;

		case Ltouched: {
			volatile unsigned *port;
			if (!(port = get_port(base[0]))) goto LERROR;
#ifdef RCX
			e = ((*port < 0xf000) ? TRUE : FALSE);
#else
			e = FALSE;
#endif
			break;
		}
		case Ltemperature: {
			volatile unsigned *port;
			if (!(port = get_port(base[0]))) goto LERROR;
			// Found this formula at news.lugnet.com.
#ifdef RCX
			e = valINT((785 - ds_scale(*port))/8);
#else
			e = valINT(8162);
#endif
			break;
		}
		case Lplay: {
			int i;
			for (i = 0, e = base[0]; i < 31 && e != NIL; i++, e = CDR(e)) {
				if (!PAIRP(e) || !PAIRP(CAR(e))
				     || !INTP(CAR(CAR(e))) || !INTP(CDR(CAR(e))))
					goto BADALIST;
#ifdef RCX
				sound_buffer[i].pitch = INTval(CAR(CAR(e)));
				sound_buffer[i].length = INTval(CDR(CAR(e)));
#endif
#ifdef Native_Win32
				play_note(INTval(CAR(CAR(e))), INTval(CDR(CAR(e))));
#endif
			}
#ifdef RCX
			sound_buffer[i].pitch = PITCH_END;
			dsound_play(sound_buffer);
#endif
			break;
		}
		case Lplaying:
#ifdef RCX
			e = dsound_playing() ? TRUE : FALSE;
#else
			e = FALSE;
#endif
			break;

		case Lpressed:
#ifdef RCX
			e = (dkey == KEY_PRGM ? TRUE : FALSE);
#else
			e = FALSE;
#endif
			break;

		case Lputs: {
			int i;
			for (i = 4, e = base[0]; i >=0 && PAIRP(e); i--, e = CDR(e)) {
#if ((defined(RCX) && defined(CONF_ASCII)) || (!defined(RCX))
				cputc(INTval(CAR(e)), i);
#elif (defined(RCX) && defined(CONF_CONIO))
				// ASCII is not enabled on the RCX, so we cannot display the string
				// Display a default text message instead: "-ASCII"
				cputc_native_5(CHAR_DASH);
				cputc_native_user(CHAR_A, CHAR_S, CHAR_C, CHAR_PARALLEL);  // ASCII
#endif
			}
#ifndef RCX
			show_lcd();
#endif
			break;
		}
		case Lputc:
			if (check_int_args(base)) goto LERROR;
			// the second arg is between 0 (right-most) and 4 (left-most)
#if ((defined(RCX) && defined(CONF_ASCII)) || (!defined(RCX))
			cputc(INTval(e = base[0]), INTval(base[1]));
#elif (defined(RCX) && defined(CONF_CONIO))
            // ASCII is not enabled on the RCX, so we cannot display the ASCII character
            // Display a generic '-' in the requested position instead
            e = base[0];
            cputc_native(CHAR_DASH, INTval(base[1]));
#endif
#ifndef RCX
			show_lcd();
#endif
			break;

		case Lcls:
			cls();
#ifndef RCX
			show_lcd();
#endif
			e = NIL;
			break;

		case Lbattery:
#ifdef RCX
			e = valINT(get_battery_mv()/100);
#else
			e = valINT(90);
#endif
			break;

		case Lreset_time:
#ifdef RCX
			base_time = get_system_up_time();
#else
#ifdef JOINT
			gettimeofday(&base_time, 0);
#endif
#endif
			// through

		case Ltime:
#ifdef RCX
			e = valINT((get_system_up_time() - base_time)/100);
#else
#ifdef JOINT
		{
			struct timeval now;
			gettimeofday(&now, 0);
			e = valINT((now.tv_sec  - base_time.tv_sec )*10
			           + (now.tv_usec - base_time.tv_usec)/100000);
		}
#else
			e = valINT(0);
#endif
#endif
			break;

		case Lsleep: {
			int ts;
			if (check_int_args(base)) goto LERROR;
			ts = INTval(e = base[0]);
#ifdef RCX
			while (ts-- > 0) {
				if (interrupted()) goto LERROR;
				msleep(100);
			}
#else
#ifdef JOINT
			while (ts-- > 0) {
				if (interrupted()) goto LERROR;
				usleep(100000);
			}
#endif
			usleep(ts*100000);
#endif
			break;
		}
		case Llinked:
#ifdef IRCOM
			wtbuf[0] = CMDPING;
			wtbufp = wtbuf + 1;
			e = transmit1(1) ? TRUE : FALSE;
#else
			e = TRUE;
#endif
			break;

#ifdef ONLINE
		case Lread:
			write1(vs_top == base ? base[0] : valINT(0), CMDREAD);
			while (1) {
				wait_rdbuf_ready();
				if (*rdbufp != CMDGENSYM) break;
				if ((e = toplevel()) == ERR) goto LERROR;
				write1(e, CMDEND);
			}
			if ((e = toplevel()) == ERR) goto LERROR;
			break;

		case Lwrite:
			e = base[0];
			write2(e, vs_top > base ? base[1] : valINT(1), CMDWRITE);
			wait_rdbuf_ready();
			if (toplevel() == ERR) goto LERROR;
			break;

		case Lread_char:
			write1(vs_top == base ? base[0] : valINT(0), CMDREADCHAR);
			wait_rdbuf_ready();
			if ((e = toplevel()) == ERR) goto LERROR;
			break;

		case Lwrite_char:
			e = base[0];
			write2(e, vs_top > base ? base[1] : valINT(1), CMDWRITECHAR);
			wait_rdbuf_ready();
			if (toplevel() == ERR) goto LERROR;
			break;

		case Lread_line:
			write1(vs_top == base ? base[0] : valINT(0), CMDREADLINE);
			wait_rdbuf_ready();
			if ((e = toplevel()) == ERR) goto LERROR;
			break;

		case Lwrite_string:
			e = base[0];
			write2(e, vs_top > base ? base[1] : valINT(1), CMDWRITESTRING);
			wait_rdbuf_ready();
			if (toplevel() == ERR) goto LERROR;
			break;
#else
		case Lread:
		case Lwrite:
		case Lread_char:
		case Lwrite_char:
		case Lread_line:
		case Lwrite_string:
			Eerror(EXunsupported_rcx_fun, fun);
			goto LERROR;
#endif
		default:
			Eerror(EXbaboon, fun);
			goto LERROR;
		}

		vs_top = base - 1;
		return e;

	LAMBDA_CALL:
		// check nargs and setup &rest parameter
		if (LAMBDArestp(fun)) {
			object *p = base + LAMBDAnreq(fun);
			if (vs_top < p) {
				Eerror(EXtoo_few_args, fun);
				goto LERROR;
			}
			p++;
			*++vs_top = NIL;
			while (vs_top > p)
				if(vs_cons()) goto LERROR;
		} else if (vs_top - base != LAMBDAnreq(fun)) {
			if (vs_top - base < LAMBDAnreq(fun))
				Eerror(EXtoo_few_args, fun);
			else
				Eerror(EXtoo_many_args, fun);
			goto LERROR;
		}

		if (tail_call) {
			object *top = vs_top;
			vs_top = vs_base - 1;
			while (base <= top)
				*++vs_top = *base++;
			e = LAMBDAbody(fun);
			if (e == NIL)
				return NIL;
			for (; CDR(e) != NIL; e = CDR(e))
				if (eval(CAR(e)) == ERR) goto LERROR;
			e = CAR(e);
			goto LOOP;
		} else {
			object *old_base = vs_base;
			object value = NIL;
			vs_base = base;
			for (e = LAMBDAbody(fun); e != NIL; e = CDR(e))
				if ((value = eval(CAR(e))) == ERR) {
					fun = base[0];
					goto LERROR;
				}
			vs_base = old_base;
			vs_top = base - 1;
			return value;
		}
		}
	NOTINTEGER:
		Eerror(EXnot_integer, e);
		goto LERROR;
	BADALIST:
		Eerror(EXbad_alist, e);
		goto LERROR;
	NOTLIST:
		Eerror(EXnot_list, e);
		goto LERROR;
	NOTPAIR:
		Eerror(EXnot_pair, e);
		goto LERROR;
	LERROR:
		if (throw_value == UNDEFINED && last_error_number != EXheap_full) {
			last_backtrace = make_cons(fun, last_backtrace);
		}
		return ERR;

	} else if (GVARP(e)) {
		object x = GVARval(e);
		if (x != UNDEFINED)
			return x;
		else
			return Eerror(EXundefined_var, e);
	} else if (CONSTP(e))
		switch (CONSTindex(e)) {
		case Vref:
			return vs_base[VrefLOC(e)];
		case VrefCAR:
			return CAR(vs_base[VrefLOC(e)]);
		case VrefCCB: {
			object p = LAMBDAenv(vs_base[0]);
			int n;
			for (n = VrefLOC(e); n > 0; n--)
				p = CDR(p);
			return CAR(p);
		}
		case InternalApply:
			goto INTERNAL_APPLY;
		default:
			return CONST2SUBR(e);
		}
	else
		return e;
}

object eval_top(object e) {
	*++vs_top = dummy_lambda;
	vs_base = vs_top;
	last_val = UNDEFINED;
	last_backtrace = NIL;

	return eval(e);

}

object toplevel() {
#ifndef IRCOM
	rdbufp = rdbuf;
#endif
	while(1) {
		object b;
#ifdef RCX
		if (interrupted()) return ERR;
#endif
#ifdef JOINT
		if (interrupted()) return ERR;
#endif
		b = *rdbufp++;
		if (COMMANDP(b))
			switch(CMDindex(b)) {
			case CXERR:
				return Eerror(EXfront_error, UNDEFINED);

			case CXUNDEFINED:
				return Eerror(EXbaboon, b);

			case CXFALSE:
			case CXTRUE:
			case CXNIL:
				*++vs_top = b;
				break;

			case CXLIST1:
				*++vs_top = NIL;
				goto CONS;
			case CXLIST2:
				*++vs_top = NIL;
				goto LISTA2;
			case CXLIST3:
				*++vs_top = NIL;
				goto LISTA3;
			case CXLIST4:
				*++vs_top = NIL;
				// through
			case CXLISTA4:
				if (vs_cons()) return ERR;
				// through
			LISTA3:
				case CXLISTA3:
				if (vs_cons()) return ERR;
				// through
			LISTA2:
				case CXLISTA2:
				if (vs_cons()) return ERR;
				// through
			CONS:
			case CXCONS:
				if (vs_cons()) return ERR;
				break;

			case CXEVAL:
				return last_val = eval_top(*vs_top);

			case CXDEFINE:
				b = *vs_top--;
				if ((last_val = eval_top(*vs_top)) != ERR) {
					if (LAMBDAP(last_val) && LAMBDAenv(last_val) == NIL)
						LAMBDAenv(last_val) = b;
					GVARval(b) = last_val;
					last_val = b;
				}
				return last_val;

			case CXUNTRACE:
				last_val = *vs_top--;
				b = GVARval(last_val);
				if (LAMBDAP(b) && PAIRP(LAMBDAenv(b))
				     && LAMBDAP(CAR(LAMBDAenv(b))))
					GVARval(last_val) = CAR(LAMBDAenv(b));
				else
					last_val = FALSE;
				return last_val;

			case CXLASTVAL:
				if (last_val != ERR)
					return last_val;
				else
					return CMDMSG;

			case CXGENSYM:
				if ((b = make_cons(symbol_table, UNDEFINED)) == ERR)
					return ERR;
				else {
					symbol_table = b;
#ifndef ONLINE
					symbol_count++;
#endif
					return GENSYM(b);
				}
			case CXMSG:
				return b;

			case CXPING:
				return Eerror(EXbaboon, b);

			case CXBYE:
			case CXMORE:
			case CXABORT:
				return b;

			case CXINTERRUPT:
				return Eerror(EXbaboon, b);

			case CXREAD:
				return *vs_top--;

			case CXWRITE:
				return b;

			case CXEND:
			default:
				return Eerror(EXbaboon, b);
			}
		else {
			b |= (*rdbufp++ << 8);
#ifndef RCX
			b |= (*rdbufp++ << 16);
			b |= (*rdbufp++ << 24);
#endif
#ifndef ONLINE
			if (GVARP(b)) {
				int n = symbol_count - GVARid(b);
				for (b = symbol_table; n > 0; n--, b = CAR(b));
				b = GENSYM(b);
			}
#endif
			*++vs_top = b;
		}
	}
}

void begin_rcx() {
	object *p;

	heap = PAIRP((object)heap0) ? heap0 : heap0+1;

	free_list = NIL;
	for (p = heap; p < heap + HPSIZE*2; p += 2) {
		*p = free_list;
		free_list = (object) p;
	}

	symbol_table = NIL;
#ifndef ONLINE
	symbol_count = 0;
#endif
	dummy_lambda = MKLAMBDA(make_cons(NIL, NIL));

	vs_top = vs_bottom;

#ifdef IRCOM
	lnp_addressing_set_handler(1, &input_handler);
#endif

#ifdef JOINT
	gettimeofday(&base_time, 0);
#endif
}

void end_rcx() {
}

#ifndef JOINT //(

#ifndef RCX //(

#include "errmsg.c"
#include "wtobj.c"

void print_result(object x) {
	if (x == ERR) {
		if (INTP(last_backtrace))
			fputs("Error: heap full\n", stdout);
		else {
			printf("Error: %s -- ",
			        error_message(INTval(CAR(last_backtrace))));
			write_object(CDR(last_backtrace));
			putchar('\n');
		}
	} else {
		write_object(x);
		putchar('\n');
	}
}
#endif //)

#ifdef RCX
static const note_t begin_beep[]={{39, 1}, {51, 1}, {PITCH_END, 0}};
static const note_t end_beep[]={{51, 1}, {39, 1}, {PITCH_END, 0}};
#endif

int main(int argc, char **argv) {
	begin_rcx();

#ifdef RCX
	stack_limit = &argc - (STACKWORDS - FRAMESIZE * 2);
#ifdef DEBUG
	memtest();
#endif
	dsound_play(begin_beep);
	base_time = get_system_up_time();
	next_watcher_polling = get_system_up_time() + POLLING_INTERVAL;
#endif

#ifdef ONLINE
	while (1) {
		object x;
		wait_rdbuf_ready();
		x = toplevel();

		if (x != CMDMORE)
			vs_top = vs_bottom;
		if (x == CMDMSG) {
			wtbufp = wtbuf;
			if (!write0(last_backtrace)
			     && !write0(last_error_val)
			     && !write0(valINT(last_error_number))) {
				wtCMD(CMDMSG);
				transmit();
			}
		} else if (x == CMDBYE)
			break;
		else if (x != CMDABORT)
			write1(x, CMDEND);
		beep();
	}
	msleep(500);
#else
#ifdef RCX
#include "xsout-rcx"
#else
#include "xsout-host"
#endif
#endif

#ifdef RCX
	dsound_play(end_beep);
#endif

	end_rcx();
	return 0;
}
#endif //)

#ifdef JOINT
void toplevel0() {
	object x = toplevel();
	if (x != CMDMORE)
		vs_top = vs_bottom;
	if (x == CMDMSG) {
		wtbufp = wtbuf;
		if (!write0(last_backtrace)
		     && !write0(last_error_val)
		     && !write0(valINT(last_error_number))) {
			wtCMD(CMDMSG);
			transmit();
		}
	} else if (x != CMDABORT)
		write1(x, CMDEND);
}

void copy_buf(unsigned char *src, int n) {
	int i;
	unsigned char *p = rdbuf;

	for (i = 0; i < n; i++)
		*p++ = src[i];
}
#endif
