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

typedef int object;

#ifdef RCX
#define TAGMASK   0x8003  // 1.............11
#define ITAGMASK  0x0003  // ..............11
#define TAGBITS   2
#else
#define TAGMASK   0x0007  // .............111
#define ITAGMASK  0x0007  // .............111
#define TAGBITS   3
#endif

#define INDEXPOS  (TAGBITS+6)

#ifdef RCX
#define PAIRTAG   0x8000  // 1.............00
#define MISCTAG   0x0000  // 0.............00
#define LAMBDATAG 0x8001  // 1.............01
#define SUBRTAG   0x0001  // 0.............01
#define GVARTAG   0x8002  // 1.............10
#define CONSTTAG  0x0002  // 0.............10
#define INTTAG    0x0003  // ..............11
#else
#define PAIRTAG   0x0000  // .............000
#define MISCTAG   0x0001  // .............001
#define LAMBDATAG 0x0002  // .............010
#define SUBRTAG   0x0003  // .............011
#define GVARTAG   0x0004  // .............100
#define CONSTTAG  0x0005  // .............101
#define INTTAG    0x0007  // .............111
#endif

#define PAIRP(obj)   (((obj)&TAGMASK)==PAIRTAG)
#define MISCP(obj)   (((obj)&TAGMASK)==MISCTAG)
#define LAMBDAP(obj) (((obj)&TAGMASK)==LAMBDATAG)
#define SUBRP(obj)   (((obj)&TAGMASK)==SUBRTAG)
#define GVARP(obj)   (((obj)&TAGMASK)==GVARTAG)
#define CONSTP(obj)  (((obj)&TAGMASK)==CONSTTAG)
#define INTP(obj)    (((obj)&ITAGMASK)==INTTAG)

#ifdef RCX
#define GVARTAG1     CONSTTAG
#define REFP(obj)    (((obj)&0x8000)&&!INTP(obj))
#define SYMBOLP(obj) (((obj)&0x0003)==0x0002) // ..............10
#define PROCP(obj)   (((obj)&0x0003)==0x0001) // ..............01
#else
#define GVARTAG1     GVARTAG
#define REFP(obj)    (!((obj)&0x1))           // ..............0
#define SYMBOLP(obj) (((obj)&0x0006)==0x0004) // .............10.
#define PROCP(obj)   (((obj)&0x0006)==0x0002) // .............01.
#endif

#define SPECIALP(obj) (SUBRP(obj)&&(SUBRindex(obj)>=Fstart))

#define REF2CELL(obj) ((obj)&~ITAGMASK)

#define CAR(cell) (*((object *)cell))
#define CDR(cell) (*((object *)((cell)|GVARTAG1)))

#define MKMISC(index) (((index)<<TAGBITS)|MISCTAG)

#define MKLAMBDA(cell) ((cell)|LAMBDATAG)
#define MKLAMBDAnargs(nreq,restp) valINT(((nreq)<<1)|((restp)?1:0))
#define LAMBDAenv(obj) CAR(REF2CELL(obj))
#define LAMBDArestp(obj) (INTval(CAR(CDR(REF2CELL(obj))))&1)
#define LAMBDAnreq(obj) (INTval(CAR(CDR(REF2CELL(obj))))>>1)
#define LAMBDAbody(obj) CDR(CDR(REF2CELL(obj)))
#define maxLAMBDAnreq 16

#define SUBRindex(obj) ((obj)>>INDEXPOS)
#define SUBRnreq(obj)  (((obj)>>(TAGBITS+4))&3)
#define SUBRnmax(obj)  (((obj)>>(TAGBITS+1))&7)
#define SUBRrestp(obj) ((obj)&(1<<TAGBITS))

#define GENSYM(cell) ((cell)|GVARTAG1)
#ifdef RCX
#define GVARid(obj) (((obj)&~0x8000)>>TAGBITS)
#else
#define GVARid(obj) ((obj)>>TAGBITS)
#endif
#define idGVAR(id) (((id)<<TAGBITS)|GVARTAG)
#define GVARval(obj) (*((object *)obj)) // the CDR part

#ifdef RCX
#define CONST2SUBR(obj) ((obj)^3)
#else
#define CONST2SUBR(obj) ((obj)^6)
#endif
#define CONSTindex(obj) ((obj)>>INDEXPOS)
#define MKCONST(index, nreq, nopt, restp) \
	(((index) << INDEXPOS)                \
	 | ((nreq) << (TAGBITS+4))            \
	 | ((nreq+nopt) << (TAGBITS+1))       \
	 | (restp ? (1<<TAGBITS) : 0)         \
	 | CONSTTAG)
#define MKSPECIAL(index) (((index) << INDEXPOS) | CONSTTAG)

#define INTval(obj) (((int)(obj))>>TAGBITS)
#define valINT(n) (((n)<<TAGBITS)|INTTAG)

#define COMMANDP(b) MISCP(b)
#define MKCOMMAND(b) MKMISC(b)
#define CMDindex(b) ((b)>>TAGBITS)

// COMMAND index
typedef enum {
	CXERR,
	CXUNDEFINED,
	CXFALSE,
	CXTRUE,
	CXNIL,
	CXLIST1,
	CXLIST2,
	CXLIST3,
	CXLIST4,
	CXLISTA4,

	CXLISTA3,
	CXLISTA2,
	CXCONS,
	CXEVAL,      CXREADCHAR = CXEVAL,
	CXDEFINE,    CXREADLINE = CXDEFINE,
 	CXUNTRACE,   CXWRITECHAR = CXUNTRACE,
	CXLASTVAL,   CXWRITESTRING = CXLASTVAL,
	CXGENSYM,
	CXMSG,
	CXPING,

	CXBYE,
	CXMORE,
	CXABORT,     CXTRACEIN = CXABORT,
	CXINTERRUPT, CXTRACEOUT = CXINTERRUPT,
	CXREAD,
	CXWRITE,
	CXACK,
	CXEND,       // This value may not be larger than 32 = 2^5.
} command_index;

// MISCs
#define ERR     MKMISC(CXERR)
#define UNDEFINED MKMISC(CXUNDEFINED)
#define FALSE     MKMISC(CXFALSE)
#define TRUE      MKMISC(CXTRUE)
#define NIL       MKMISC(CXNIL)

// COMMANDs
#define CMDERR     MKCOMMAND(CXERR)
//#define CMDUNDEFINED MKCOMMAND(CXUNDEFINED)
#define CMDFALSE     MKCOMMAND(CXFALSE)
#define CMDTRUE      MKCOMMAND(CXTRUE)
#define CMDNIL       MKCOMMAND(CXNIL)
#define CMDCONS      MKCOMMAND(CXCONS)
#define CMDLISTA2    MKCOMMAND(CXLISTA2)
#define CMDLISTA3    MKCOMMAND(CXLISTA3)
#define CMDLISTA4    MKCOMMAND(CXLISTA4)
#define CMDLIST1     MKCOMMAND(CXLIST1)
#define CMDLIST2     MKCOMMAND(CXLIST2)
#define CMDLIST3     MKCOMMAND(CXLIST3)
#define CMDLIST4     MKCOMMAND(CXLIST4)
#define CMDEVAL      MKCOMMAND(CXEVAL)
#define CMDDEFINE    MKCOMMAND(CXDEFINE)
#define CMDUNTRACE   MKCOMMAND(CXUNTRACE)
#define CMDLASTVAL   MKCOMMAND(CXLASTVAL)
#define CMDGENSYM    MKCOMMAND(CXGENSYM)
#define CMDMSG       MKCOMMAND(CXMSG)
#define CMDPING      MKCOMMAND(CXPING)
#define CMDBYE       MKCOMMAND(CXBYE)
#define CMDMORE      MKCOMMAND(CXMORE)
#define CMDABORT     MKCOMMAND(CXABORT)
#define CMDINTERRUPT MKCOMMAND(CXINTERRUPT)
#define CMDREAD      MKCOMMAND(CXREAD)
#define CMDWRITE     MKCOMMAND(CXWRITE)
#define CMDACK     MKCOMMAND(CXACK)
#define CMDREADCHAR  MKCOMMAND(CXREADCHAR)
#define CMDREADLINE  MKCOMMAND(CXREADLINE)
#define CMDWRITECHAR MKCOMMAND(CXWRITECHAR)
#define CMDWRITESTRING MKCOMMAND(CXWRITESTRING)
#define CMDTRACEIN   MKCOMMAND(CXTRACEIN)
#define CMDTRACEOUT  MKCOMMAND(CXTRACEOUT)
#define CMDEND       MKCOMMAND(CXEND)

#define MKFlambda(n) (((Flambda)<<INDEXPOS)|((n)<<TAGBITS)|SUBRTAG)
#define MKFwith_watcher(n) (((Fwith_watcher)<<INDEXPOS)|((n)<<TAGBITS)|SUBRTAG)
#define MKhoist(index, n) (((index)<<INDEXPOS)|((n)<<TAGBITS)|SUBRTAG)
#define MKVref(index, n) (((index)<<INDEXPOS)|((n)<<TAGBITS)|CONSTTAG)
#define FlambdaLOC(obj) (((obj)>>TAGBITS)&0x3f)
#define Fwith_watcherLOC(obj) (((obj)>>TAGBITS)&0x3f)
#define FhoistLOC(obj) (((obj)>>TAGBITS)&0x3f)
#define VrefLOC(obj) (((obj)>>TAGBITS)&0x3f)

// SUBR index
typedef enum {
	Lcar,
	Lcdr,
	Lcons,
	Lset_car,
	Lset_cdr,
	LpairP,
	LnullP,
	Lnot,

	Llist,
#ifdef LISTLIB
	LlistA,
	Llist_ref,
	Lappend,
	Lassoc,
	Lmember,
	Llength,
	Lreverse,
#endif

	Leq,
	LbooleanP,
	Lplus,
	Lminus,
	Ltimes,
	Lquotient,
	Lremainder,
	LLT,
	LGT,
	LEQ,
	LLE,
	LGE,
	Llogand,
	Llogior,
	Llogxor,
	Llogshl,
	Llogshr,
	Lrandom,
	LintegerP,
	Ltrace_call,
	Lapply,
	LfunctionP,
	LsymbolP,
	Lthrow,
	Lgc,

	Lmotor,
	Lspeed,
	Llight_on,
	Llight_off,
	Llight,
	Lrotation_on,
	Lrotation_off,
	Lrotation,
	Ltouched,
	Ltemperature,
	Lplay,
	Lplaying,
	Lpressed,
	Lputs,
	Lputc,
	Lcls,
	Lbattery,
	Lreset_time,
	Ltime,
	Lsleep,
	Llinked,
	Lread,
	Lwrite,
	Lread_char,
	Lwrite_char,
	Lread_line,
	Lwrite_string,

	Fstart,
	Fquote = Fstart, Vref = Fstart,
	Flambda, VrefCAR = Flambda,
	Fset, VrefCCB = Fset,
	Fif, InternalApply = Fif,
	Fbegin,
	Flet,
	FletA,
	Fletrec,
	Fand,
	For,
	Fcatch,
	Fwith_watcher,
	Fwait_until,
	Fend,

	Fhoist = Fend,
	Fhoist_arg,
	Ftail_call,
} subr_index;

// error index
typedef enum {
	EXbaboon,
	EXundefined_var,
	EXnot_pair,
	EXnot_list,
	EXnot_function,
	EXnot_integer,
	EXbad_number,
	EXbad_port_number,
	EXbad_alist,
	EXtoo_few_args,
	EXtoo_many_args,
	EXheap_full,
	EXno_catcher,
	EXunsupported_rcx_fun,
	EXinterrupt,
	EXvs_overflow,
	EXstack_overflow,
	EXfront_error,
} error_index;
