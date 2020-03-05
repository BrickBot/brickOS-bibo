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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#include "object.h"

// Command-line argument processing
#if (defined(__sun__) && defined(__svr4__)) || defined(BSD)	// Solaris||BSD
#undef HAVE_GETOPT_LONG
#else
#define HAVE_GETOPT_LONG
#endif

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>

static const struct option long_options[]={
  {"tty",    required_argument,0,'t'},
  {"baud",   required_argument,0,'b'},
  {"timeout",required_argument,0,'o'},
  {"help",   no_argument      ,0,'h'},
  {"verbose",no_argument      ,0,'v'},
  {0        ,0                ,0,0  }
};

#else // HAVE_GETOPT_LONG
  #define getopt_long(ac, av, opt, lopt, lidx) (getopt((ac), (av), (opt)))
#endif // HAVE_GETOPT_LONG

// Object (frontend objects) definitions

enum type {
	t_Integer,
	t_Cons,
	t_Symbol,
	t_Binding,
	t_Wrapper,
	t_Misc,
	t_String
};

typedef union unionObject *Object;
typedef struct structInteger *Integer;
typedef struct structCons *Cons;
typedef struct structSymbol *Symbol;
typedef struct structBinding *Binding;
typedef struct structWrapper *Wrapper;
typedef struct structMisc *Misc;
typedef struct structString *String;

struct structInteger {
	int t;
	Integer oblink;
	int	intVal;
};

struct structCons {
	int t;
	Cons oblink;
	Object car;
	Object cdr;
};

struct structSymbol {
	int t;
	Symbol hashlink;
	char *name;
	object clone;
	Symbol next;
};

struct structBinding {
	int t;
	Binding oblink;
	Binding prev;
	Symbol var;
	int offset;
	Binding ccbref;
};

struct structWrapper {
	int t;
	Wrapper oblink;
	object clone;
};

struct structMisc {
	int t;
	char *rep;
};

struct structString {
	int t;
	String oblink;
	char *self;
};

union unionObject {
	struct structInteger i;
	struct structCons c;
	struct structSymbol s;
	struct structBinding b;
	struct structWrapper w;
	struct structMisc m;
	struct structString st;
};

#define	typeOf(obj) ((obj)->c.t)

#include "include.h"

// Errors

jmp_buf topEnv;
jmp_buf mainEnv;
jmp_buf *pEnv = &topEnv;

void error(char *msg) {
	printf("Error: %s\n", msg);
	longjmp(*pEnv, 1);
}

void baboon(char *msg) {
	printf("System Error: %s\n", msg);
	longjmp(*pEnv, 1);
}

void error1(char *msg, Object x) {
	printf("Error: %s -- ", msg);
	writeObject(stdout, x);
	putchar('\n');
	longjmp(*pEnv, 1);
}

void errorf(char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	fputs("Error: ", stdout);
	vprintf(msg, ap);
	putchar('\n');
	va_end(ap);
	longjmp(*pEnv, 1);
}

int com_debug=0; /* true to print out debugging info */
int baud = 0;
int timeout = 250;
char *tty_name = NULL;

//#ifndef Native_Win32

// Port

#define MAX_PORT 256

FILE* portTable[MAX_PORT];
void initPort()
{
	int i;

	portTable[0] = stdin;
	portTable[1] = stdout;
	portTable[2] = stderr;
	for (i = 3; i < MAX_PORT; i++)
		portTable[i] = NULL;
}
void portErrMsg(char* rw, int port)
{
	switch (errno) {
	case EBADF:
		fprintf(stdout, "Error: port is not for %s -- %d\n", rw, port);
		break;
	case EPIPE:
		fprintf(stdout, "Error: closed port -- %d\n", port);
		break;
	default:
		fprintf(stdout, "Error: %s -- %d\n", strerror(errno), port);
		break;
	}
}
//#endif // ifndef Native_Win32

#ifdef Native_Win32
#include <windows.h>
#include <io.h>

int getc2(FILE *file) {
	int c;
	while ((c = getc(file)) == -1 && _isatty(_fileno(file)))
		;
	return c;
}
                 
#else

#define getc2(x) getc(x)

#endif

//#ifndef Native_Win32

int portGetc(int port)
{
	int c;
	if (port < 0 || port >= MAX_PORT || portTable[port] == NULL) {
		fprintf(stdout, "Error: wrong port number -- %d", port);
		return EOF;
	}
	if ((c = getc2(portTable[port])) == EOF) {
		portErrMsg("reading", port);
		return EOF;
	}
	return c;
}
Object portReadObject(int port, int tokenAllowed)
{
	if (port < 0 || port >= MAX_PORT || portTable[port] == NULL)
		errorf("Error: wrong port number -- %d", port);
	return readObject(portTable[port], tokenAllowed);
}
int portPuts(char* s, int port)
{
	int ret;
	if (port < 0 || port >= MAX_PORT || portTable[port] == NULL) {
		fprintf(stdout, "Error: wrong port number -- %d", port);
		return EOF;
	}
	if ((ret = fputs(s, portTable[port])) == EOF) {
		portErrMsg("writing", port);
		return EOF;
	}
	return ret;
}
int portPutc(int c, int port)
{
	if (port < 0 || port >= MAX_PORT || portTable[port] == NULL) {
		fprintf(stdout, "Error: wrong port number -- %d", port);
		return EOF;
	}
	if (putc(c, portTable[port]) == EOF) {
		portErrMsg("writing", port);
		return EOF;
	}
	return c;
}
int portWriteObject(int port, Object x)
{
	if (port < 0 || port >= MAX_PORT || portTable[port] == NULL) {
		fprintf(stdout, "Error: wrong port number -- %d", port);
		return EOF;
	}
	errno = 0;
	writeObject(portTable[port], x);
	if (errno != 0) {
		portErrMsg("writing", port);
		return EOF;
	}
	return 0;
}
int portFlush(int port) 
{
	if (port < 0 || port >= MAX_PORT || portTable[port] == NULL) {
		fprintf(stdout, "Error: wrong port number -- %d", port);
		return EOF;
	}
	if (fflush(portTable[port]) == EOF) {
		portErrMsg("writing", port);
		return EOF;
	}
	return 0;
}

//#endif // ifndef Native_Win32

// Object allocator

Integer IntegerLink, IntegerFree;
Cons ConsLink, ConsFree;
Binding BindingLink, BindingFree;
Wrapper WrapperLink, WrapperFree;
String StringLink, StringFree;

#define STABSIZE 512

Symbol symbolTable[STABSIZE];
Symbol gvarTable[STABSIZE];
Object subrTable[Fend];

void initAllocator() {
	IntegerLink = IntegerFree = 0;
	ConsLink = ConsFree = 0;
	BindingLink = BindingFree = 0;
	WrapperLink = WrapperFree = 0;
	StringLink = StringFree = 0;
	{
		int i;
		for (i = 0; i < STABSIZE; i++) {
			symbolTable[i] = 0;
			gvarTable[i] = 0;
		}
	}
}

void freeObjects() {
	IntegerFree = IntegerLink;
	ConsFree = ConsLink;
	BindingFree = BindingLink;
	WrapperFree = WrapperLink;
	{
		String st;
		for (st = StringLink; st != StringFree; st = st->oblink)
			free(st->self);
		StringFree = StringLink;
	}
}

Object intern(char *name, int needCopy) {
	unsigned char *s;
	int h = 0;
	Symbol p;
	for (s = name; *s != '\0'; s++)
		h += *s * 12345 + 1;
	if (h < 0)
		h = -h;
	h %= STABSIZE;
	for (p = symbolTable[h]; p; p = p->hashlink) {
		if (strcmp(p->name, name) == 0)
			return (Object) p;
	}
	p = (Symbol) malloc(sizeof(struct structSymbol));
	p->t = t_Symbol;
	if (needCopy) {
		p->name = (char *) malloc(strlen(name) + 1);
		strcpy(p->name, name);
	} else
		p->name = name;
	p->clone = UNDEFINED;
	p->hashlink = symbolTable[h];
	symbolTable[h] = p;
	return (Object) p;
}

Object def(char *name, object clone) {
	Object sym = intern(name, 0);
	sym->s.clone = clone;
	subrTable[SUBRindex(clone)] = sym;
	return sym;
}

void registerGvar(Symbol sym) {
	int h = GVARid(sym->clone);
	if (h < 0)
		h = -h;
	h %= STABSIZE;
	sym->next = gvarTable[h];
	gvarTable[h] = sym;
}

Object gvarSymbol(object gv) {
	Symbol p;
	int h = GVARid(gv);
	if (h < 0)
		h = -h;
	h %= STABSIZE;
	for (p = gvarTable[h]; p; p = p->next) {
		if (p->clone == gv)
			return (Object) p;
	}
	return 0;
}

Object newInteger(int n) {
	Integer p;
	if (IntegerFree) {
		p = IntegerFree;
		IntegerFree = IntegerFree->oblink;
	} else {
		p = (Integer) malloc(sizeof(struct structInteger));
		p->t = t_Integer;
		p->oblink = IntegerLink;
		IntegerLink = p;
	}
	p->intVal = n;
	return (Object) p;
}

Object newCons(Object car, Object cdr) {
	Cons p;
	if (ConsFree) {
		p = ConsFree;
		ConsFree = ConsFree->oblink;
	} else {
		p = (Cons) malloc(sizeof(struct structCons));
		p->t = t_Cons;
		p->oblink = ConsLink;
		ConsLink = p;
	}
	p->car = car;
	p->cdr = cdr;
	return (Object) p;
}

Binding newBinding(Symbol v, Binding prev, int offset) {
	Binding p;
	if (BindingFree) {
		p = BindingFree;
		BindingFree = BindingFree->oblink;
	} else {
		p = (Binding) malloc(sizeof(struct structBinding));
		p->t = t_Binding;
		p->oblink = BindingLink;
		BindingLink = p;
	}
	p->var = v;
	p->offset = offset;
	p->ccbref = 0;
	p->prev = prev;
	return p;
}

Object newWrapper(object x) {
	Wrapper p;
	if (WrapperFree) {
		p = WrapperFree;
		WrapperFree = WrapperFree->oblink;
	} else {
		p = (Wrapper) malloc(sizeof(struct structWrapper));
		p->t = t_Wrapper;
		p->oblink = WrapperLink;
		WrapperLink = p;
	}
	p->clone = x;
	return (Object) p;
}

Object newMisc(char *s) {
	Misc p = (Misc) malloc(sizeof(struct structMisc));
	p->t = t_Misc;
	p->rep = s;
	return (Object) p;
}

Object newString(char *s) {
	String p;
	if (StringFree) {
		p = StringFree;
		StringFree = StringFree->oblink;
	} else {
		p = (String) malloc(sizeof(struct structSymbol));
		p->t = t_String;
		p->oblink = StringLink;
		StringLink = p;
	}
	p->self = (char *) malloc(strlen(s) + 1);
	strcpy(p->self, s);
	return (Object) p;
}

// Global Objects

Object errorObject;
Object undefinedObject;
Object nullObject;
Object trueObject;
Object falseObject;
Object eofObject;
Object pingObject;
Object byeObject;
Object moreObject;
Object abortObject;
Object messageObject;
Object READObject;
Object WRITEObject;
Object kokkaToken;
Object dotToken;

Object Squote;
Object Slambda;
Object Sdefine;
Object Slast_value;
Object Sload;
Object Sbye;
Object Sset;
Object Sif;
Object Sbegin;
Object Swhile;
Object Slet;
Object SletA;
Object Sletrec;
Object Sand;
Object Sor;
Object Scatch;
Object Swith_watcher;
Object Swait_until;
//Object Sping;

#ifdef Native_Win32
Object Strace_call, Strace, Suntrace, Sname, Sfun, Sargs;
#else
Object Strace_call, Strace, Suntrace, Sfork, Sname, Sfun, Sargs;
#endif

Symbol Sccb;
Binding Bccbref;

void initGlobals() {
	int i;

	errorObject = newMisc("#<error>");
	undefinedObject = newMisc("#<undefined>");
	nullObject = newMisc("()");
	trueObject = newMisc("#t");
	falseObject = newMisc("#f");
	eofObject = newMisc("#<end-of-file>");
	pingObject = newMisc("#<ping>");
	byeObject = newMisc("#<bye>");
	moreObject = newMisc("#<more>");
	abortObject = newMisc("#<abort>");
	messageObject = newMisc("#<message>");
	READObject = newMisc("#<read>");
	WRITEObject = newMisc("#<write>");
	kokkaToken = newMisc("#<right-parenthesis>");
	dotToken = newMisc("#<dot>");

	for (i = 0; i < Fend; i++)
		subrTable[i] = 0;

	def("car", MKCONST(Lcar,1,0,0));
	def("cdr", MKCONST(Lcdr,1,0,0));
	def("cons", MKCONST(Lcons,2,0,0));
	def("set-car!", MKCONST(Lset_car,2,0,0));
	def("set-cdr!", MKCONST(Lset_cdr,2,0,0));
	def("pair?", MKCONST(LpairP,1,0,0));
	def("null?", MKCONST(LnullP,1,0,0));
	def("not", MKCONST(Lnot,1,0,0));

	def("list", MKCONST(Llist,0,0,1));
#ifdef LISTLIB
	def("list*", MKCONST(LlistA,1,0,1));
	def("list-ref", MKCONST(Llist_ref,2,0,0));
	def("append", MKCONST(Lappend,1,0,1));
	def("assoc", MKCONST(Lassoc,2,0,0));
	def("member", MKCONST(Lmember,2,0,0));
	def("length", MKCONST(Llength,1,0,0));
	def("reverse", MKCONST(Lreverse,1,0,0));
#endif

	def("eq?", MKCONST(Leq,2,0,0));
	def("boolean?", MKCONST(LbooleanP,1,0,0));
	def("+", MKCONST(Lplus,0,0,1));
	def("-", MKCONST(Lminus,1,0,1));
	def("*", MKCONST(Ltimes,0,0,1));
	def("/", MKCONST(Lquotient,2,0,0));
	def("remainder", MKCONST(Lremainder,2,0,0));
	def("<", MKCONST(LLT,1,0,1));
	def(">", MKCONST(LGT,1,0,1));
	def("=", MKCONST(LEQ,1,0,1));
	def("<=", MKCONST(LLE,1,0,1));
	def(">=", MKCONST(LGE,1,0,1));
	def("logand", MKCONST(Llogand,2,0,0));
	def("logior", MKCONST(Llogior,2,0,0));
	def("logxor", MKCONST(Llogxor,2,0,0));
	def("logshl", MKCONST(Llogshl,2,0,0));
	def("logshr", MKCONST(Llogshr,2,0,0));
	def("random", MKCONST(Lrandom,1,0,0));
	def("integer?", MKCONST(LintegerP,1,0,0));
	Strace_call = def("trace-call", MKCONST(Ltrace_call,3,0,0));
	def("apply", MKCONST(Lapply,2,0,1));
	def("function?", MKCONST(LfunctionP,1,0,0));
	def("symbol?", MKCONST(LsymbolP,1,0,0));
	def("throw", MKCONST(Lthrow,2,0,0));
	def("gc", MKCONST(Lgc,0,0,0));
	def("motor", MKCONST(Lmotor,2,0,0));
	def("speed", MKCONST(Lspeed,2,0,0));
	def("light-on", MKCONST(Llight_on,1,0,0));
	def("light-off", MKCONST(Llight_off,1,0,0));
	def("light", MKCONST(Llight,1,0,0));
	def("rotation-on", MKCONST(Lrotation_on,1,0,0));
	def("rotation-off", MKCONST(Lrotation_off,1,0,0));
	def("rotation", MKCONST(Lrotation,1,0,0));
	def("touched?", MKCONST(Ltouched,1,0,0));
	def("temperature", MKCONST(Ltemperature,1,0,0));
	def("play", MKCONST(Lplay,1,0,0));
	def("playing?", MKCONST(Lplaying,0,0,0));
	def("pressed?", MKCONST(Lpressed,0,0,0));
	def("puts", MKCONST(Lputs,1,0,0));
	def("putc", MKCONST(Lputc,2,0,0));
	def("cls", MKCONST(Lcls,0,0,0));
	def("battery", MKCONST(Lbattery,0,0,0));
	def("reset-time", MKCONST(Lreset_time,0,0,0));
	def("time", MKCONST(Ltime,0,0,0));
	def("sleep", MKCONST(Lsleep,1,0,0));
	def("linked?", MKCONST(Llinked,0,0,0));
	def("read", MKCONST(Lread,0,1,0));
	def("read-char", MKCONST(Lread_char,0,1,0));
	def("read-line", MKCONST(Lread_line,0,1,0));
	def("write", MKCONST(Lwrite,1,1,0));
	def("write-char", MKCONST(Lwrite_char,1,1,0));
	def("write-string", MKCONST(Lwrite_string,1,1,0));

	Squote = def("quote", MKSPECIAL(Fquote));
	Slambda = def("lambda", MKSPECIAL(Flambda));
	Sset = def("set!", MKSPECIAL(Fset));
	Sif = def("if", MKSPECIAL(Fif));
	Sbegin = def("begin", MKSPECIAL(Fbegin));
	Slet = def("let", MKSPECIAL(Flet));
	SletA = def("let*", MKSPECIAL(FletA));
	Sletrec = def("letrec", MKSPECIAL(Fletrec));
	Sand = def("and", MKSPECIAL(Fand));
	Sor = def("or", MKSPECIAL(For));
	Scatch = def("catch", MKSPECIAL(Fcatch));
	Swith_watcher = def("with-watcher", MKSPECIAL(Fwith_watcher));
	Swait_until = def("wait-until", MKSPECIAL(Fwait_until));

	for (i = 0; i < Fend; i++)
		if (subrTable[i] == 0)
			printf("SUBR %d is not defined\n", i);

	Sdefine = intern("define", 0);
	Slast_value = intern("last-value", 0);
//	Sping = intern("ping", 0);
	Sload = intern("load", 0);
	Sbye = intern("bye", 0);

	Strace = intern("trace", 0);
	Suntrace = intern("untrace", 0);
#ifndef Native_Win32
	Sfork = intern("fork", 0);
#endif
	Sname = intern("name", 0);
	Sfun = intern("fun", 0);
	Sargs = intern("args", 0);

	Sccb = (Symbol) malloc(sizeof(struct structSymbol));
	Sccb->t = t_Symbol;
	Sccb->clone = UNDEFINED;

	Bccbref = (Binding) malloc(sizeof(struct structBinding));
}

Object list1(Object x) {
	return newCons(x, nullObject);
}

Object list2(Object x, Object y) {
	return newCons(x, newCons(y, nullObject));
}

Object list3(Object x, Object y, Object z) {
	return newCons(x, newCons(y, newCons(z, nullObject)));
}

Object list4(Object x, Object y, Object z, Object w) {
	return newCons(x, newCons(y, newCons(z, newCons(w, nullObject))));
}

Object listA3(Object x, Object y, Object z) {
	return newCons(x, newCons(y, z));
}

Object listA4(Object x, Object y, Object z, Object w) {
	return newCons(x, newCons(y, newCons(z, w)));
}

Object nreverse(Object x) {
	Object val = nullObject;
	while (x != nullObject) {
		Object this = x;
		x = this->c.cdr;
		this->c.cdr = val;
		val = this;
	}
	return val;
}

// The Reader

#define multipleEscapeChar '|'
#define singleEscapeChar '\\'

#define TAeof 1
#define TAkokka 2
#define TAdot 4

#define CAwhitespace 0
#define CAterminating 1
#define CAnonTerminating 2
#define CAsingleEscape 3
#define CAmultipleEscape 4
#define CAconstituent 5

int charAttribute[128];

int cat(int c) {
	if (c >= 0 && c < 128)
		return charAttribute[c];
	else
		return CAconstituent;
}

char inRead(FILE *in) {
	char c = getc2(in);
	if (c == EOF)
		error("unexpected EOF while reading a character");
	return c;
}

char *tokenBuffer, *endTokenBuffer, *tokenPointer;

void initTokenBuffer() {
	tokenBuffer = (char *) malloc(128);
	endTokenBuffer = tokenBuffer + 128;
}

void openTokenBuffer() {
	tokenPointer = tokenBuffer;
}

void closeTokenBuffer() {
	*tokenPointer = '\0';
}

void addToToken(char c) {
	*tokenPointer++ = c;
	if (tokenPointer >= endTokenBuffer) {
		int len = endTokenBuffer - tokenBuffer;
		char *s = (char *) malloc(len * 2);
		strncpy(s, tokenBuffer, len);
		tokenBuffer = s;
		endTokenBuffer = s + len * 2;
		tokenPointer = s + len;
	}
}

void readToken(FILE *in) {
	char c;
	int a;
	openTokenBuffer();
	for (;;) {
		if ((c = getc2(in)) == EOF)
			break;
		else if ((a = cat(c)) == CAterminating) {
			ungetc(c, in);
			break;
		} else if (a == CAwhitespace)
			break;
		else if (a == CAsingleEscape || a == CAmultipleEscape)
			errorf("escape '%c' not allowed", c);
		else
			addToToken(c);
	}
	closeTokenBuffer();
}

Object readNumber(char *bp, int radix, int errorp) {
	char *original = bp;
	char c = *bp++;
	int sign = 0;
	int val = 0;
	if (c == '-') {
		sign = 1;
		c = *bp++;
	} else if (c == '+')
		c = *bp++;

	if (c == '\0')
	   goto LERROR;

	do {
		int digit;
		if (c >= '0' && c <= '9')
			digit = c - '0';
		else if (c >= 'a' && c <= 'f')
			digit = c - 'a' + 10;
		else
			goto LERROR;
		if (digit >= radix)
			goto LERROR;
		val = val * radix + digit;
	} while ((c = *bp++) != '\0');
	if (sign)
		val = -val;
	return newInteger(val);

LERROR:
	if (errorp)
		errorf("wrong number format \"%s\"", original);
	return 0;
}

Object sharpSignMacroReader(FILE *in) {
	char c = inRead(in);
	switch (c) {
	case 't':
		readToken(in);
		if (tokenBuffer[0] != '\0')
			errorf("#t followed by garbage \"%s\"", tokenBuffer);
		return trueObject;
	case 'f':
		readToken(in);
		if (tokenBuffer[0] != '\0')
			errorf("#f followed by garbage \"%s\"", tokenBuffer);
		return falseObject;
	case '\\':
		c = inRead(in);
		readToken(in);
		if (tokenBuffer[0] == '\0') {
			// NOTE: Processing of character literals happens here
			// TODO: Convert ASCII characters to CONIO predefined characters for non-ASCII RCX
	    	return newInteger(c);
	    } else if (c == 's') {
	    	if (strcmp(tokenBuffer, "pace") == 0) return newInteger(' ');
	    } else if (c == 'n') {
	    	if (strcmp(tokenBuffer, "ewline") == 0) return newInteger('\n');
	    } else if (c == 't') {
	    	if (strcmp(tokenBuffer, "ab") == 0) return newInteger('\t');
	    } else if (c == 'p') {
	    	if (strcmp(tokenBuffer, "age") == 0) return newInteger('\f');
	    } else if (c == 'r') {
	    	if (strcmp(tokenBuffer, "eturn") == 0) return newInteger('\r');
	    }
	    errorf("unknown character #\\%c%s", c, tokenBuffer);
	case 'b':
		readToken(in);
		return readNumber(tokenBuffer, 2, 1);
	case 'o':
		readToken(in);
		return readNumber(tokenBuffer, 8, 1);
	case 'd':
		readToken(in);
		return readNumber(tokenBuffer, 10, 1);
	case 'x':
		readToken(in);
		return readNumber(tokenBuffer, 16, 1);
	default:
		errorf("unknown syntax #%c", c);
		return 0; // just to suppress compiler warning
	}
}

typedef struct structReaderConstant {
	char *name;
	Object val;
	struct structReaderConstant *hashlink;
} readerConstant;

#define RCTABSIZE 128

readerConstant *rcTable[RCTABSIZE];

int readerConstantHashValue(char *name) {
	unsigned char *s;
	int h = 0;
	for (s = name; *s != '\0'; s++)
		h += *s * 12345 + 1;
	if (h < 0)
		h = -h;
	return h % RCTABSIZE;
}

void defReaderConstant(char *name, int val) {
	int h = readerConstantHashValue(name);
	readerConstant *p = (readerConstant *) malloc(sizeof(readerConstant));
	p->name = name;
	p->val = newInteger(val);
	p->hashlink = rcTable[h];
	rcTable[h] = p;
}

Object charMacroReader(char c, FILE *in, int tokenAllowed) {
	switch (c) {
	case '(': {
		Object val;
		Cons last;
		Object x = readObject(in, TAkokka);
		if (x == kokkaToken)
			return nullObject;

		val = newCons(x, nullObject);
		last = (Cons) val;
		for (;;) {
			x = readObject(in, TAkokka | TAdot);
			if (x == kokkaToken)
				return val;
			else if (x == dotToken) {
				last->cdr = readObject(in, 0);
				if (readObject(in, TAkokka) != kokkaToken)
					error("right parenthesis ')' missing");
				return val;
			} else {
				Object y = newCons(x, nullObject);
				last->cdr = y;
				last = (Cons) y;
			}
		}
	}
	case ')':
		if ((tokenAllowed & TAkokka) != 0)
			return kokkaToken;
		else
			error("right parenthesis ')' in a wrong place");
	case '\'':
		return list2(Squote, readObject(in, 0));
	case '`':
		error("backquote syntax not allowed");
	case ',':
		error("comma syntax not allowed");
	case '"':
		openTokenBuffer();
		while ((c = inRead(in)) != '"') {
			if (cat(c) == CAsingleEscape) {
				c = inRead(in);
			}
			// TODO: Convert ASCII character string to a string of CONIO predefined characters for non-ASCII RCX
			addToToken(c);
		}
		closeTokenBuffer();
		return newString(tokenBuffer);
	case ';':
		while ((c = getc2(in)) != '\n')
			if (c == EOF) return 0;
		return 0;
	case '#':
		return sharpSignMacroReader(in);
	case ':':
		readToken(in);
		{
			int h = readerConstantHashValue(tokenBuffer);
			readerConstant *p;
			for (p = rcTable[h]; p; p = p->hashlink)
				if (strcmp(p->name, tokenBuffer) == 0)
					return p->val;
			errorf("undefined reader constant \":%s\"", tokenBuffer);
		}
	default:
		baboon("unknown macro char");
		return 0; // just to suppress compiler warning
	}
}

Object readObject(FILE *in, int tokenAllowed) {
	char c;
	int a, escape;
	for (;;) {
		do
			if ((c = getc2(in)) == EOF) {
				if ((tokenAllowed & TAeof) != 0)
					return eofObject;
				else
					error("unexpected EOF while reading an object");
			}
		while ((a = cat(c)) == CAwhitespace);
		if (a == CAterminating || a == CAnonTerminating) {
			Object x = charMacroReader(c, in, tokenAllowed);
			if (x)
				return x;
			continue;
		}
		openTokenBuffer();
		escape = 0;
		for (;;) {
			if (a == CAconstituent || a == CAnonTerminating)
				addToToken(c);
			else if (a == CAsingleEscape) {
				escape = 1;
				addToToken(inRead(in));
			} else if (a == CAmultipleEscape) {
				escape = 1;
				for (;;) {
					c = inRead(in);
					a = cat(c);
					if (a == CAmultipleEscape)
						break;
					else if (a == CAsingleEscape)
						c = inRead(in);
					addToToken(c);
				}
			} else if (a == CAterminating) {
				ungetc(c, in);
				break;
			} else if (a == CAwhitespace)
				break;

			if ((c = getc2(in)) == EOF) break;
			a = cat(c);
		}
		closeTokenBuffer();
		if (escape)
			return intern(tokenBuffer, 1);
		else if (strcmp(tokenBuffer, ".")==0) {
			if ((tokenAllowed & TAdot) != 0)
				return dotToken;
			else
				error("dot '.' in a wrong place");
		} else {
			Object x = readNumber(tokenBuffer, 10, 0);
			if (x)
				return x;
			else
				return intern(tokenBuffer, 1);
		}
	}
}

void initReader() {
	int i;

	initTokenBuffer();

	for (i = 0; i < 128; i++)
		charAttribute[i] = CAconstituent;

	charAttribute['\t'] = CAwhitespace;
	charAttribute['\n'] = CAwhitespace;
	charAttribute['\013'] = CAwhitespace;
	charAttribute['\f'] = CAwhitespace;
	charAttribute['\r'] = CAwhitespace;
	charAttribute['\034'] = CAwhitespace;
	charAttribute['\035'] = CAwhitespace;
	charAttribute['\036'] = CAwhitespace;
	charAttribute['\037'] = CAwhitespace;
	charAttribute[' '] = CAwhitespace;

	charAttribute[singleEscapeChar] = CAsingleEscape;
	charAttribute[multipleEscapeChar] = CAmultipleEscape;

	charAttribute['('] = CAterminating;
	charAttribute[')'] = CAterminating;
	charAttribute['\''] = CAterminating;
	charAttribute['`'] = CAterminating;
	charAttribute[','] = CAterminating;
	charAttribute['"'] = CAterminating;
	charAttribute[';'] = CAterminating;
	charAttribute['#'] = CAnonTerminating;
	charAttribute[':'] = CAnonTerminating;

#ifdef RCX
	defReaderConstant("most-positive-integer", (1<<(15-TAGBITS))-1);
	defReaderConstant("most-negative-integer", -(1<<(15-TAGBITS)));
#else
	defReaderConstant("most-positive-integer", (1<<(31-TAGBITS))-1);
	defReaderConstant("most-negative-integer", -(1<<(31-TAGBITS)));
#endif

	defReaderConstant("a", 1);
	defReaderConstant("b", 2);
	defReaderConstant("c", 3);
	defReaderConstant("off", 0);
	defReaderConstant("forward", 1);
	defReaderConstant("back", 2);
	defReaderConstant("brake", 3);
	defReaderConstant("max-speed", 255);

	defReaderConstant("white", 98);
	defReaderConstant("black", 0);

	defReaderConstant("stdin", 0);
	defReaderConstant("stdout", 1);
	defReaderConstant("stderr", 2);

	defReaderConstant("A0", 0);
	defReaderConstant("Am0", 1);
	defReaderConstant("H0", 2);
	defReaderConstant("C1", 3);
	defReaderConstant("Cm1", 4);
	defReaderConstant("D1", 5);
	defReaderConstant("Dm1", 6);
	defReaderConstant("E1", 7);
	defReaderConstant("F1", 8);
	defReaderConstant("Fm1", 9);
	defReaderConstant("G1", 10);
	defReaderConstant("Gm1", 11);
	defReaderConstant("A1", 12);
	defReaderConstant("Am1", 13);
	defReaderConstant("H1", 14);
	defReaderConstant("C2", 15);
	defReaderConstant("Cm2", 16);
	defReaderConstant("D2", 17);
	defReaderConstant("Dm2", 18);
	defReaderConstant("E2", 19);
	defReaderConstant("F2", 20);
	defReaderConstant("Fm2", 21);
	defReaderConstant("G2", 22);
	defReaderConstant("Gm2", 23);
	defReaderConstant("A2", 24);
	defReaderConstant("Am2", 25);
	defReaderConstant("H2", 26);
	defReaderConstant("C3", 27);
	defReaderConstant("Cm3", 28);
	defReaderConstant("D3", 29);
	defReaderConstant("Dm3", 30);
	defReaderConstant("E3", 31);
	defReaderConstant("F3", 32);
	defReaderConstant("Fm3", 33);
	defReaderConstant("G3", 34);
	defReaderConstant("Gm3", 35);
	defReaderConstant("A3", 36);
	defReaderConstant("Am3", 37);
	defReaderConstant("H3", 38);
	defReaderConstant("C4", 39);
	defReaderConstant("Cm4", 40);
	defReaderConstant("D4", 41);
	defReaderConstant("Dm4", 42);
	defReaderConstant("E4", 43);
	defReaderConstant("F4", 44);
	defReaderConstant("Fm4", 45);
	defReaderConstant("G4", 46);
	defReaderConstant("Gm4", 47);
	defReaderConstant("A4", 48);
	defReaderConstant("Am4", 49);
	defReaderConstant("H4", 50);
	defReaderConstant("C5", 51);
	defReaderConstant("Cm5", 52);
	defReaderConstant("D5", 53);
	defReaderConstant("Dm5", 54);
	defReaderConstant("E5", 55);
	defReaderConstant("F5", 56);
	defReaderConstant("Fm5", 57);
	defReaderConstant("G5", 58);
	defReaderConstant("Gm5", 59);
	defReaderConstant("A5", 60);
	defReaderConstant("Am5", 61);
	defReaderConstant("H5", 62);
	defReaderConstant("C6", 63);
	defReaderConstant("Cm6", 64);
	defReaderConstant("D6", 65);
	defReaderConstant("Dm6", 66);
	defReaderConstant("E6", 67);
	defReaderConstant("F6", 68);
	defReaderConstant("Fm6", 69);
	defReaderConstant("G6", 70);
	defReaderConstant("Gm6", 71);
	defReaderConstant("A6", 72);
	defReaderConstant("Am6", 73);
	defReaderConstant("H6", 74);
	defReaderConstant("C7", 75);
	defReaderConstant("Cm7", 76);
	defReaderConstant("D7", 77);
	defReaderConstant("Dm7", 78);
	defReaderConstant("E7", 79);
	defReaderConstant("F7", 80);
	defReaderConstant("Fm7", 81);
	defReaderConstant("G7", 82);
	defReaderConstant("Gm7", 83);
	defReaderConstant("A7", 84);
	defReaderConstant("Am7", 85);
	defReaderConstant("H7", 86);
	defReaderConstant("C8", 87);
	defReaderConstant("Cm8", 88);
	defReaderConstant("D8", 89);
	defReaderConstant("Dm8", 90);
	defReaderConstant("E8", 91);
	defReaderConstant("F8", 92);
	defReaderConstant("Fm8", 93);
	defReaderConstant("G8", 94);
	defReaderConstant("Gm8", 95);
	defReaderConstant("A8", 96);

	defReaderConstant("La0", 0);
	defReaderConstant("La#0", 1);
	defReaderConstant("Si0", 2);
	defReaderConstant("Do1", 3);
	defReaderConstant("Do#1", 4);
	defReaderConstant("Re1", 5);
	defReaderConstant("Re#1", 6);
	defReaderConstant("Mi1", 7);
	defReaderConstant("Fa1", 8);
	defReaderConstant("Fa#1", 9);
	defReaderConstant("So1", 10);
	defReaderConstant("So#1", 11);
	defReaderConstant("La1", 12);
	defReaderConstant("La#1", 13);
	defReaderConstant("Si1", 14);
	defReaderConstant("Do2", 15);
	defReaderConstant("Do#2", 16);
	defReaderConstant("Re2", 17);
	defReaderConstant("Re#2", 18);
	defReaderConstant("Mi2", 19);
	defReaderConstant("Fa2", 20);
	defReaderConstant("Fa#2", 21);
	defReaderConstant("So2", 22);
	defReaderConstant("So#2", 23);
	defReaderConstant("La2", 24);
	defReaderConstant("La#2", 25);
	defReaderConstant("Si2", 26);
	defReaderConstant("Do3", 27);
	defReaderConstant("Do#3", 28);
	defReaderConstant("Re3", 29);
	defReaderConstant("Re#3", 30);
	defReaderConstant("Mi3", 31);
	defReaderConstant("Fa3", 32);
	defReaderConstant("Fa#3", 33);
	defReaderConstant("So3", 34);
	defReaderConstant("So#3", 35);
	defReaderConstant("La3", 36);
	defReaderConstant("La#3", 37);
	defReaderConstant("Si3", 38);
	defReaderConstant("Do4", 39);
	defReaderConstant("Do#4", 40);
	defReaderConstant("Re4", 41);
	defReaderConstant("Re#4", 42);
	defReaderConstant("Mi4", 43);
	defReaderConstant("Fa4", 44);
	defReaderConstant("Fa#4", 45);
	defReaderConstant("So4", 46);
	defReaderConstant("So#4", 47);
	defReaderConstant("La4", 48);
	defReaderConstant("La#4", 49);
	defReaderConstant("Si4", 50);
	defReaderConstant("Do5", 51);
	defReaderConstant("Do#5", 52);
	defReaderConstant("Re5", 53);
	defReaderConstant("Re#5", 54);
	defReaderConstant("Mi5", 55);
	defReaderConstant("Fa5", 56);
	defReaderConstant("Fa#5", 57);
	defReaderConstant("So5", 58);
	defReaderConstant("So#5", 59);
	defReaderConstant("La5", 60);
	defReaderConstant("La#5", 61);
	defReaderConstant("Si5", 62);
	defReaderConstant("Do6", 63);
	defReaderConstant("Do#6", 64);
	defReaderConstant("Re6", 65);
	defReaderConstant("Re#6", 66);
	defReaderConstant("Mi6", 67);
	defReaderConstant("Fa6", 68);
	defReaderConstant("Fa#6", 69);
	defReaderConstant("So6", 70);
	defReaderConstant("So#6", 71);
	defReaderConstant("La6", 72);
	defReaderConstant("La#6", 73);
	defReaderConstant("Si6", 74);
	defReaderConstant("Do7", 75);
	defReaderConstant("Do#7", 76);
	defReaderConstant("Re7", 77);
	defReaderConstant("Re#7", 78);
	defReaderConstant("Mi7", 79);
	defReaderConstant("Fa7", 80);
	defReaderConstant("Fa#7", 81);
	defReaderConstant("So7", 82);
	defReaderConstant("So#7", 83);
	defReaderConstant("La7", 84);
	defReaderConstant("La#7", 85);
	defReaderConstant("Si7", 86);
	defReaderConstant("Do8", 87);
	defReaderConstant("Do#8", 88);
	defReaderConstant("Re8", 89);
	defReaderConstant("Re#8", 90);
	defReaderConstant("Mi8", 91);
	defReaderConstant("Fa8", 92);
	defReaderConstant("Fa#8", 93);
	defReaderConstant("So8", 94);
	defReaderConstant("So#8", 95);
	defReaderConstant("La8", 96);

	defReaderConstant("pause", 97);
}

// The Printer

void writeObject(FILE* fp, Object x) {
	if (x == 0)
		fputs("#<null>", fp);
	else
		switch (typeOf(x)) {
		case t_Integer:
			fprintf(fp, "%d", x->i.intVal);
			break;
		case t_Cons: {
			if (x->c.car == Squote
			     && typeOf(x->c.cdr) == t_Cons
			     && x->c.cdr->c.cdr == nullObject)
			{
				putc('\'', fp);
				writeObject(fp, x->c.cdr->c.car);
			} else {
				putc('(', fp);
				writeObject(fp, x->c.car);
				while (typeOf(x = x->c.cdr) == t_Cons) {
					putc(' ', fp);
					writeObject(fp, x->c.car);
				}
				if (x != nullObject) {
					fputs(" . ", fp);
					writeObject(fp, x);
				}
				putc(')', fp);
			}
			break;
		}
		case t_Symbol:
			if (readNumber(x->s.name, 10, 0))
				fprintf(fp, "|%s|", x->s.name);
			else
				fputs(x->s.name, fp);
			break;
		case t_Binding:
			fprintf(fp, "#<%sbinding of %s at offset %d>",
			        (x->b.ccbref == Bccbref ? "ccb " :
			          (x->b.ccbref ? "hoisted " : "")),
			        x->b.var->name,
			        x->b.offset);
			break;
		case t_Wrapper: {
			object y = x->w.clone;
			if (LAMBDAP(y)) {
				Object sym = gvarSymbol(GENSYM(REF2CELL(y)));
				if (sym)
					fprintf(fp, "#<function %s>", sym->s.name);
				else
					fputs("#<function>", fp);
			} else if (SUBRP(y))
				fprintf(fp, "#<function %s>", subrTable[SUBRindex(y)]->s.name);
			else if (GVARP(y)) {
				Object sym = gvarSymbol(y);
				if (sym)
					writeObject(fp, sym);
				else
					fputs("#<uninterned symbol>", fp);
			} else if (CONSTP(y))
				writeObject(fp, subrTable[SUBRindex(y)]);
			else if (INTP(y)) {
#ifdef RCX
				if (y & 0x8000)
					y |= 0xffff0000;
#endif
				fprintf(fp, "%d", INTval(y));

			} else
				fprintf(fp, "#<object #x%x>", y);
			break;
		}
		case t_Misc:
			fputs(x->m.rep, fp);
			break;
		case t_String: {
			char *s = x->st.self;
			putc('"', fp);
			for (s = x->st.self; *s != '\0'; s++) {
				if (*s == '"' || *s == '\\')
					putc('\\', fp);
				putc(*s, fp);
			}
			putc('"', fp);
			break;
		}
		default:
			baboon("unknown Object");
		}
}

#ifdef DEBUG
#include "wtobj.c"
#include "test.c"
#endif

#define BUFSIZE (256 - 3) // size of wtbuf[]

#define VSSIZE 256
int vs_len;
int partly_transmitted;

void vs_inc() {
	if (vs_len >= VSSIZE) {
		if (partly_transmitted)
			transmitCMD(CMDABORT);
		error("Sorry, RCX cannot receive such a long expression");
	}
	vs_len++;
}

void simulate_vs(object e) {
	switch(CMDindex(e)) {
	case CXERR:
		vs_inc(); break;
	case CXUNDEFINED:
		baboon("wrong command in vs simulation -- CXUNDEFINED");
	case CXFALSE:
	case CXTRUE:
	case CXNIL:
		vs_inc(); break;
	case CXLIST1:
		vs_inc(); vs_len--; break;
	case CXLIST2:
		vs_inc(); vs_len -= 2; break;
	case CXLIST3:
		vs_inc(); vs_len -= 3; break;
	case CXLIST4:
		vs_inc(); vs_len -= 4; break;
	case CXLISTA4:
		vs_len -= 4; break;
	case CXLISTA3:
		vs_len -= 3; break;
	case CXLISTA2:
		vs_len -= 2; break;
	case CXCONS:
	case CXEVAL:
		vs_len--; break;
	case CXDEFINE:
		vs_len -= 2; break;
	case CXUNTRACE:
		vs_len--; break;
	case CXLASTVAL:
	case CXGENSYM:
	case CXMSG:
	case CXPING:
	case CXBYE:
	case CXMORE:
		break;
	case CXABORT:
		baboon("wrong command in vs simulation");
	case CXINTERRUPT:
	case CXREAD:
	case CXWRITE:
		break;
	case CXACK:
	case CXEND:
	default:
		baboon("wrong command in vs simulation");
	}
}

#ifdef ONLINE
unsigned char wtbuf[BUFSIZE];
unsigned char CMDbuf[1];
unsigned char rdbuf[BUFSIZE];
unsigned char *wtbufp;
const unsigned char *rdbufp;

#ifdef IRCOM

#ifndef _WIN32
 #include <unistd.h>
#endif

#include <termios.h>

#include <lnp.h>
#include <rcx_comm.h>

tty_t tty = { BADFILE, tty_t_undefined };
char lnp_buffer[RCX_BUFFERSIZE];

int rdbuf_ready = 0;

void input_handler(const unsigned char *buf, unsigned char len,
                    unsigned char src)
{
	int i;

	if (rdbuf_ready) {
		printf("lost a packet: ");
    	for (i = 0; i < len; i++)
    		printf("0x%x ", buf[i]);
    	printf("\n");
		fflush(stdout);
    } else {
    	CMDbuf[0] = CMDACK;
    	lnp_addressing_write(CMDbuf, 1, 1, 0);
    	for (i = 0; i < len; i++)
    		rdbuf[i] = buf[i];
    	rdbufp = rdbuf;
    	rdbuf_ready = 1;
    }
}

void begin_rcx() {
	int i, j;

 	rcx_init(&tty, tty_name, baud, timeout, TRUE); 

	lnp_addressing_set_handler(0, &input_handler);

	CMDbuf[0] = CMDPING;
	for (i = 0; i < 5; i++) {
		lnp_addressing_write(CMDbuf, 1, 1, 0);
		for (j = 0; j < 10; j++) {
            rcx_recv_lnp(&tty, lnp_buffer, sizeof(lnp_buffer), timeout);
			if (rdbuf_ready) {
				rdbuf_ready = 0;
				return;
			}
			usleep(100000);
		}
	}
	fputs("RCX is not responding.\n", stdout);
	fputs("Make sure RCX is running, and try again.\n", stdout);
	exit(0);
}

void end_rcx() {
}

static void transmit() {
	rdbuf_ready = 0;
	lnp_addressing_write(wtbuf, wtbufp-wtbuf, 1, 0);
}

static void transmitCMD(object x) {
	CMDbuf[0] = x;
	rdbuf_ready = 0;
	lnp_addressing_write(CMDbuf, 1, 1, 0);
}

#endif

#ifdef ONLINE

#ifdef IRCOM
int interrupt_flag = 0;
#endif
int interrupt_enabled = 0;

void sigint(int x) {
	if (interrupt_enabled)
#ifdef IRCOM
		interrupt_flag = 1;
#else
		notify_interrupt();
#endif
	else
		fputs("terminal interrupt ignored\n", stdout);
	signal(SIGINT, sigint);
}
#endif

#ifdef JOINT
static void transmit() {
	copy_buf(wtbuf, wtbufp - wtbuf);
	interrupt_enabled = 1;
	toplevel0();
	interrupt_enabled = 0;
}

static void transmitCMD(object x) {
	CMDbuf[0] = x;
	copy_buf(CMDbuf, 1);
	interrupt_enabled = 1;
	toplevel0();
	interrupt_enabled = 0;
}
#endif

void wtInit() {
	wtbufp = wtbuf;
	vs_len = 0;
	partly_transmitted = 0;
}

static void checkBufferSpace(int n) {
	if (wtbufp >= wtbuf + BUFSIZE - n) {
		*wtbufp++ = (CMDMORE & 0xff);
		transmit();

#ifdef IRCOM
		while (!rdbuf_ready) {
            rcx_recv_lnp(&tty, lnp_buffer, sizeof(lnp_buffer), timeout);
			usleep(10000);
		}
#else
		rdbufp = rdbuf;
#endif
		if (*rdbufp != CMDMORE)
			fputs("RCX cannot receive more message\n", stdout);
		wtbufp = wtbuf;
		partly_transmitted = 1;
	}
}

static void wtCMD(object e) {
	simulate_vs(e);
	checkBufferSpace(1);
	*wtbufp++ = e & 0xff;
}

static void wt(object e) {
	if (COMMANDP(e))
		wtCMD(e);
	else {
		vs_inc();
#ifdef RCX
		checkBufferSpace(2);
		*wtbufp++ = e & 0xff;
		*wtbufp++ = e >> 8;
#else
		checkBufferSpace(4);
		*wtbufp++ = e & 0xff;
		*wtbufp++ = e >> 8;
		*wtbufp++ = e >> 16;
		*wtbufp++ = e >> 24;
#endif
	}
}

#include "errmsg.c"

// The message receiver

#define RSSIZE 1024 // may not be larger than the heap size, to avoid
                    // infinite loop caused by circular structure

Object receive_stack[RSSIZE];
Object *rs_limit = receive_stack + RSSIZE;
Object *rs_top = receive_stack;

int traceLevel = 0;

#define rs_push(x) \
	if (rs_top >= rs_limit) return abortObject; \
	*rs_top++ = x;

void print_message(int id, Object val, Object bt) {
	if (id == EXheap_full)
		fputs("Error: heap full\n", stdout);
	else {
		if (id != EXfront_error) {
			if (val == undefinedObject)
				printf("Error: %s\n", error_message(id));
			else {
				printf("Error: %s -- ", error_message(id));
				writeObject(stdout, val);
				putchar('\n');
			}
		}
		if (bt == nullObject)
			fputs("at toplevel\n", stdout);
		else {
			fputs("Backtrace: ", stdout);
			while (1) {
				if (bt->c.car == nullObject)
					fputs("polling-watchers", stdout);
				else {
					object f = bt->c.car->w.clone;
					if (SUBRP(f))
						if (SUBRindex(f) < Fend)
							fputs(subrTable[SUBRindex(f)]->s.name, stdout);
						else
							fputs("???", stdout);
					else if (LAMBDAP(f)) {
						Object sym = gvarSymbol(GENSYM(REF2CELL(f)));
						if (sym)
							fputs(sym->s.name, stdout);
						else
							fputs("#<function>", stdout);
					} else
						writeObject(stdout, bt->c.car);
				}
				if ((bt = bt->c.cdr) == nullObject)
					break;
				fputs(" > ", stdout);
			}
			putchar('\n');
		}
	}
}

Object receive() {
#ifndef IRCOM
	rdbufp = rdbuf;
#endif
	while (1) {
		object b = *rdbufp++;
		if (COMMANDP(b))
			switch(CMDindex(b)) {
			case CXERR:
				return errorObject;
			case CXUNDEFINED:
				rs_push(undefinedObject);
				break;
			case CXFALSE:
				rs_push(falseObject);
				break;
			case CXTRUE:
				rs_push(trueObject);
				break;
			case CXNIL:
				rs_push(nullObject);
				break;
			case CXLIST1:
				rs_push(nullObject);
				goto CONS;
			case CXLIST2:
				rs_push(nullObject);
				goto LISTA2;
			case CXLIST3:
				rs_push(nullObject);
				goto LISTA3;
			case CXLIST4:
				rs_push(nullObject);
				// through
			case CXLISTA4: {
				Object d = *--rs_top;
				rs_top[-1] = newCons(rs_top[-1], d);
			}
				// through
			LISTA3:
			case CXLISTA3: {
				Object d = *--rs_top;
				rs_top[-1] = newCons(rs_top[-1], d);
			}
				// through
			LISTA2:
			case CXLISTA2: {
				Object d = *--rs_top;
				rs_top[-1] = newCons(rs_top[-1], d);
			}
				// through
			CONS:
			case CXCONS: {
				Object d = *--rs_top;
				rs_top[-1] = newCons(rs_top[-1], d);
				break;
			}
			// CXEVAL = CXREADCHAR
			// CXDEFINE = CXREADLINE
			// CXUNTRACE = CXWRITECHAR
			// CXLASTVAL = CXWRITESTRING
			case CXGENSYM:
				baboon("wrong command received -- GENSYM");
			case CXMSG:
				print_message(INTval(rs_top[-1]->w.clone),
				               rs_top[-2], rs_top[-3]);
				rs_top -= 3;
				return messageObject;
			case CXPING:
				return pingObject;

			case CXBYE:
				return byeObject;
			case CXMORE:
				return moreObject;
			// CXABORT = CXTRACEIN
			// CXINTERRUPT = CXTRACEOUT
			case CXREAD: {
				Object port = *--rs_top;
				if (!(typeOf(port) == t_Wrapper && INTP(port->w.clone))) {
					fputs("Error: wrong port number -- ", stdout);
					writeObject(stdout, port);
					putchar('\n');
					wtInit();
					wtCMD(CMDERR);
				} else {
					jmp_buf thisEnv;
					jmp_buf *prevEnv = pEnv;
					pEnv = &thisEnv;
					if (setjmp(thisEnv) == 0) {
						Object x;
#ifdef ONLINE
						interrupt_enabled = 1;
						x = portReadObject(INTval(port->w.clone), 0);
						interrupt_enabled = 0;
#else
						x = portReadObject(INTval(port->w.clone), 0);
#endif
						ppInit();
						ppObject(x);
						ppFinish();
						wtInit();
						wtObject(x, 0);
						wtCMD(CMDREAD);
					} else {
						wtInit();
						wtCMD(CMDERR);
					}
					pEnv = prevEnv;
				}
				return READObject;
			}
			case CXWRITE: {
				Object port = *--rs_top;
				Object x = *--rs_top;
				if (!(typeOf(port) == t_Wrapper && INTP(port->w.clone))) {
					fputs("Error: wrong port number -- ", stdout);
					writeObject(stdout, port);
					putchar('\n');
					wtInit();
					wtCMD(CMDERR);
				} else {
					if (portWriteObject(INTval(port->w.clone), x) == EOF ||
					    portFlush(INTval(port->w.clone)) == EOF) {
						wtInit();
						wtCMD(CMDERR);
					} else {
						wtInit();
						wtCMD(CMDWRITE);
					}
				}
				return WRITEObject;
			}
			case CXREADCHAR: {
				Object port = *--rs_top;
				if (!(typeOf(port) == t_Wrapper && INTP(port->w.clone))) {
					fputs("Error: wrong port number -- ", stdout);
					writeObject(stdout, port);
					putchar('\n');
					wtInit();
					wtCMD(CMDERR);
				} else {
					int c;
#ifdef ONLINE
					interrupt_enabled = 1;
					c = portGetc(INTval(port->w.clone));
					interrupt_enabled = 0;
#else
					c = portGetc(INTval(port->w.clone));
#endif
					if (c == EOF) {
						wtInit();
						wtCMD(CMDERR);
					} else {
						wtInit();
						wt(valINT(c));
						wtCMD(CMDREAD);
					}
				}
				return READObject;
			}
			case CXWRITECHAR: {
				Object port = *--rs_top;
				Object c = *--rs_top;
				if (!(typeOf(port) == t_Wrapper && INTP(port->w.clone))) {
					fputs("Error: wront port number -- ", stdout);
					writeObject(stdout, port);
					putchar('\n');
					wtInit();
					wtCMD(CMDERR);
				} else if (!(typeOf(c) == t_Wrapper && INTP(c->w.clone))) {
					fputs("Error: not a character -- ", stdout);
					writeObject(stdout, c);
					putchar('\n');
					wtInit();
					wtCMD(CMDERR);
				} else {
					if (portPutc(INTval(c->w.clone),
						     INTval(port->w.clone)) == EOF ||
					    portFlush(INTval(port->w.clone)) == EOF) {
						wtInit();
						wtCMD(CMDERR);
					} else {
						wtInit();
						wtCMD(CMDWRITE);
					}
				}
				return WRITEObject;
			}
			case CXREADLINE: {
				Object port = *--rs_top;
				Object val = nullObject;
				int c;
				if (!(typeOf(port) == t_Wrapper && INTP(port->w.clone))) {
					fputs("Error: wrong port number -- ", stdout);
					writeObject(stdout, port);
					putchar('\n');
					wtInit();
					wtCMD(CMDERR);
				} else {
#ifdef ONLINE
					interrupt_enabled = 1;
					while ((c = portGetc(INTval(port->w.clone))) != '\n' && c != EOF)
						val = newCons(newInteger(c), val);
					interrupt_enabled = 0;
#else
					while ((c = portGetc(INTval(port->w.clone))) != '\n' && c != EOF)
						val = newCons(newInteger(c), val);
#endif
					wtInit();
					wtObject(nreverse(val), 0);
					wtCMD(CMDREAD);
				}
				return READObject;
			}
			case CXWRITESTRING: {
				Object port = *--rs_top;
				Object s0 = *--rs_top;
				if (!(typeOf(port) == t_Wrapper && INTP(port->w.clone))) {
					fputs("Error: wrong port number -- ", stdout);
					writeObject(stdout, port);
					putchar('\n');
					wtInit();
					wtCMD(CMDERR);
				} else {
					Object s;
					int errflag = 0;

					for (s = s0; typeOf(s) == t_Cons; s = s->c.cdr) {
						Object c = s->c.car;
						if (typeOf(c) == t_Wrapper && INTP(c->w.clone)) {
							if (portPutc(INTval(c->w.clone), INTval(port->w.clone)) == EOF) {
								errflag = 2;
								break;
							}
						} else {
							errflag = 1;
							break;
						}
					}
					if (errflag != 2) {
						portFlush(INTval(port->w.clone));
					}
					if (errflag || s != nullObject) {
						if (errflag == 1) {
							fputs("Error: not a string -- ", stdout);
							writeObject(stdout, s0);
							putchar('\n');
						}
						wtInit();
						wtCMD(CMDERR);
					} else {
						wtInit();
						wtCMD(CMDWRITE);
					}
				}
				return WRITEObject;
			}
			case CXTRACEIN: {
				Object args = *--rs_top;
				int i;
				for (i = traceLevel; i > 0; i--)
					fputs("  ", stdout);
				printf("%d>", traceLevel);
				traceLevel++;
				writeObject(stdout, newCons(*--rs_top, args));
				putchar('\n');
				fflush(stdout);
				wtInit();
				wtCMD(CMDWRITE);
				return WRITEObject;
			}
			case CXTRACEOUT: {
				Object args = *--rs_top;
				int i;
				traceLevel--;
				for (i = traceLevel; i > 0; i--)
					fputs("  ", stdout);
				printf("%d<", traceLevel);
				writeObject(stdout, list2(*--rs_top, args));
				putchar('\n');
				fflush(stdout);
				wtInit();
				wtCMD(CMDWRITE);
				return WRITEObject;
			}
			case CXACK:
				baboon("wrong command received -- ACK");
			case CXEND:
				return *--rs_top;
			default:
				baboon("wrong command received -- default");
			}
		else {
			b |= (*rdbufp++ << 8);
#ifndef RCX
			b |= (*rdbufp++ << 16);
			b |= (*rdbufp++ << 24);
#endif
			rs_push(newWrapper(b));
		}
	}
}

#ifdef IRCOM
#ifndef Native_Win32
 #include <sys/time.h>
#endif

Object receive1() {
	Object x;
	int interrupt_waiting = 0;
	int loop_count = 0;
	struct timeval last_time, now;

	interrupt_enabled = 1;
	while (1) {
        rcx_recv_lnp(&tty, lnp_buffer, sizeof(lnp_buffer), timeout);
		if (rdbuf_ready) {
			interrupt_enabled = 0;
			x = receive();
			interrupt_enabled = 1;
			if (interrupt_waiting) {
				if (x == errorObject) {
					break;
				} else {
					rdbuf_ready = 0;
					continue;
				}
			}
			if (x == moreObject) {
				transmitCMD(CMDMORE);
				continue;
			} else if (x == abortObject)
				transmitCMD(CMDABORT);
			else if (x == READObject || x == WRITEObject) {
				transmit();
				continue;
			} else if (x == pingObject) {
				rdbuf_ready = 0;
				continue;
			}
			rs_top = receive_stack;
			break;
		} else if (interrupt_flag) {
			if (!interrupt_waiting) {
				gettimeofday(&last_time, 0);
				transmitCMD(CMDINTERRUPT);
				interrupt_waiting = 1;
				loop_count = 0;
				interrupt_flag = 0;
				continue;
			} else {
				gettimeofday(&now, 0);
				if (1000000*(now.tv_sec  - last_time.tv_sec )
				     + now.tv_usec - last_time.tv_usec
				     >= 1000000)
				{
					fputs("RCX didn't respond to the previous interrupt.\n",
					       stdout);
					fputs("Type 'c' to continue, or type 'e' to exit XS: ",
					       stdout);
					fflush(stdout);
					while (1) {
						int c, answer = 0, count = 0;
						while ((c = getc2(stdin)) != '\n' && c != EOF)
							if (c > ' ') {
								count++;
								answer = c;
							}
							if (count == 1) {
								if (answer == 'c')
									break;
								else if (answer == 'e')
									longjmp(mainEnv, 1);
							}
						if (count > 0) {
							fputs("Type 'c' or 'e': ", stdout);
							fflush(stdout);
						}
					}
				}
				last_time = now;
				interrupt_flag = 0;
			}
		}
		usleep(10000);
		if (interrupt_waiting && ++loop_count >= 20) {
			transmitCMD(CMDINTERRUPT);
			loop_count = 0;
		}
	}
	interrupt_enabled = 0;
	return x;
}
#endif

#ifdef JOINT
Object received_value;

Object receive1() {
	rs_top = receive_stack;
	return received_value;
}

void receive0(unsigned char *src, int n) {
	int i;
	unsigned char *p = rdbuf;
	Object x;
	for (i = 0; i < n; i++)
		*p++ = src[i];
	x = receive();
	if (x == moreObject) {
		CMDbuf[0] = CMDMORE;
		copy_buf(CMDbuf, 1);
	} else if (x == abortObject) {
		CMDbuf[0] = CMDABORT;
		copy_buf(CMDbuf, 1);
	} else if (x == READObject || x == WRITEObject) {
		copy_buf(wtbuf, wtbufp - wtbuf);
	} else
		received_value = x;
}
#endif

#else // ifndef ONLINE

FILE *xsout;

void begin_rcx() {
#ifdef RCX
	xsout = fopen("xsout-rcx", "w");
#else
	xsout = fopen("xsout-host", "w");
#endif
}

void end_rcx() {
	fclose(xsout);
}

static void transmit() {
#ifdef RCX
	fputs("toplevel();\n", xsout);
	fputs("//assert(last_val==valINT(0));\n", xsout);
#else
	fputs("print_result(toplevel());\n", xsout);
#endif
	fputs("vs_top = value_stack;\n\n", xsout);
	fflush(xsout);
	fputs("xsout written\n", stdout);
}

int nextRcxSymbolIndex = 1;

object newRcxSymbol() {
	return idGVAR(nextRcxSymbolIndex++);
}

int messageBytes;

void wtInit() {
	fputs("wtbufp = rdbuf;\n", xsout);
	messageBytes = 0;
	vs_len = 0;
	partly_transmitted = 0;
}

static void checkBufferSpace(int n) {
	if (messageBytes >= BUFSIZE - n) {
		fputs("wtCMD(CMDMORE);\n", xsout);
		fputs("toplevel();\n", xsout);

		fputs("wtbufp = rdbuf;\n", xsout);
		messageBytes = 0;
		partly_transmitted = 1;
	}
	messageBytes += n;
}

char *CMDname(object e) {
	switch (CMDindex(e & 0xff)) {
	case CXERR: return "CMDERR";
	case CXUNDEFINED: return "CMDUNDEFINED";
	case CXFALSE: return "CMDFALSE";
	case CXTRUE: return "CMDTRUE";
	case CXNIL: return "CMDNIL";
	case CXLIST1: return "CMDLIST1";
	case CXLIST2: return "CMDLIST2";
	case CXLIST3: return "CMDLIST3";
	case CXLIST4: return "CMDLIST4";
	case CXLISTA4: return "CMDLISTA4";
	case CXLISTA3: return "CMDLISTA3";
	case CXLISTA2: return "CMDLISTA2";
	case CXCONS: return "CMDCONS";
	case CXEVAL: return "CMDEVAL";
	case CXDEFINE: return "CMDDEFINE";
	case CXUNTRACE: return "CMDUNTRACE";
	case CXLASTVAL: return "CMDLASTVAL";
	case CXGENSYM: return "CMDGENSYM";
	case CXMSG: return "CMDMSG";
	case CXPING: return "CMDPING";
	case CXBYE: return "CMDBYE";
	case CXMORE: return "CMDMORE";
	case CXABORT: return "CMDABORT";
	case CXINTERRUPT: return "CMDINTERRUPT";
	case CXREAD: return "CMDREAD";
	case CXWRITE: return "CMDWRITE";
	case CXACK: return "CMDACK";
	case CXEND: return "CMDEND";
	default: baboon("cannot find command name");
	}
}

void wtCMD(object e) {
	simulate_vs(e);
	checkBufferSpace(1);
	fprintf(xsout, "wtCMD(%s);\n", CMDname(e));
}

void wt(object e) {
	if (COMMANDP(e))
		wtCMD(e);
	else {
		vs_inc();
#ifdef RCX
		checkBufferSpace(2);
		fprintf(xsout, "wt(0x%04x); //", e & 0xffff);
#else
		checkBufferSpace(4);
		fprintf(xsout, "wt(0x%08x); //", e);
#endif
		if (SUBRP(e)) {
			if (SUBRindex(e) < Fstart)
				fprintf(xsout, "function %s\n",
				        subrTable[SUBRindex(e)]->s.name);
			else {
				char *name;
				if (SUBRindex(e) < Fend)
					name = subrTable[SUBRindex(e)]->s.name;
				else if (SUBRindex(e) == Fhoist)
					name = "hoist";
				else if (SUBRindex(e) == Fhoist_arg)
					name = "hoist-arg";
				else if (SUBRindex(e) == Ftail_call)
					name = "tail-call";
				else
					baboon("unknown special form");
				fprintf(xsout, "special form %s(%d)\n",
				         name, FlambdaLOC(e));
			}
		} else if (CONSTP(e)) {
			switch (CONSTindex(e)) {
			case Vref:
				fprintf(xsout, "Vref(%d) or ", VrefLOC(e));
				break;
			case VrefCAR:
				fprintf(xsout, "VrefCAR(%d) or ", VrefLOC(e));
				break;
			case VrefCCB:
				fprintf(xsout, "VrefCCB(%d) or ", VrefLOC(e));
				break;
			}
			fprintf(xsout, "symbol %s\n", subrTable[SUBRindex(e)]->s.name);
		} else if (INTP(e))
			fprintf(xsout, "integer %d\n", INTval(e));
	    else if (GVARP(e)) {
	    	Object sym = gvarSymbol(e);
	    	if (sym)
	    		fprintf(xsout, "symbol %s\n", sym->s.name);
	    	else
	    		fputs("uninterned symbol\n", xsout);
		} else
			baboon("unknown object to wt");
	}
}

static void transmitCMD(object e) {
	wtInit();
	fprintf(xsout, "wtCMD(%s);\n", CMDname(e));
	transmit();
}

#endif // ifndef ONLINE

// The preprocessor

Binding currentBinding;
int vsOffset;

Symbol undefinedGvars;

void ppInit() {
	undefinedGvars = 0;
}

void ppFinish() {
	Symbol sym, next;
	for (sym = undefinedGvars; sym; sym = sym->next)
		sym->clone = UNDEFINED;
	for (sym = undefinedGvars; sym; sym = next) {
		transmitCMD(CMDGENSYM);
#ifdef ONLINE
		{
			Object x = receive1();
			if (!(typeOf(x) == t_Wrapper && SYMBOLP(x->w.clone)))
				error1("cannot allocate symbol", (Object) sym);

			sym->clone = x->w.clone;
		}
#else
		sym->clone = newRcxSymbol();
#endif
		next = sym->next;
		registerGvar(sym);
	}
}

void ppMarkCCB() {
	currentBinding = newBinding(Sccb, currentBinding, 0);
}

void checkSymbol(Object x) {
	if (typeOf(x) != t_Symbol)
		error1("not a symbol", x);
}

int endp(Object x) {
	if (x == nullObject)
		return 1;
	else if (typeOf(x) != t_Cons)
		error1("not a list", x);
	return 0; // just to suppress compiler warning
}

void checkBinding(Object binding) {
	if (endp(binding) || endp(binding->c.cdr))
		error1("binding with too few items", binding);
	else if (!endp(binding->c.cdr->c.cdr))
		error1("binding with too many items", binding);
}

void too_few_args(Object f, Object args) {
	printf("Error: too few arguments to %s -- ", f->s.name);
	writeObject(stdout, newCons(f, args));
	putchar('\n');
	longjmp(*pEnv, 1);
}

void too_many_args(Object f, Object args) {
	printf("Error: too many arguments to %s -- ", f->s.name);
	writeObject(stdout, newCons(f, args));
	putchar('\n');
	longjmp(*pEnv, 1);
}

Object ppBind(Object v) {
	checkSymbol(v);
	if (CONSTP(v->s.clone) && SUBRindex(v->s.clone) >= Fstart)
		error1("special form name may not be bound", v);
	currentBinding = newBinding((Symbol) v, currentBinding, vsOffset++);
	return (Object) currentBinding;
}

Object ppVref(Object v) {
	Binding b;
	int ccb = 0;
	checkSymbol(v);
	if (CONSTP(v->s.clone) && SUBRindex(v->s.clone) >= Fstart)
		error1("special form name may not be referenced", v);
	for (b = currentBinding; b; b = b->prev) {
		if (b->var == (Symbol) v) {
			if (ccb)
				if (b->ccbref)
					return (Object) b->ccbref;
				else {
					Binding b1 = newBinding(b->var, 0, 0);
					b1->ccbref = Bccbref;
					b->ccbref = b1;
					return (Object) b1;
				}
			else
				return (Object) b;
		}
		if (b->var == Sccb)
			ccb = 1;
	}
	if (v->s.clone == UNDEFINED) {
		v->s.clone = NIL;
		v->s.next = undefinedGvars;
		undefinedGvars = (Symbol) v;
	}
	return v;
}

void ppObject(Object x) {
	switch (typeOf(x)) {
	case t_Cons:
		ppObject(x->c.car);
		ppObject(x->c.cdr);
		break;
	case t_Symbol:
		if (x->s.clone == UNDEFINED) {
			x->s.clone = NIL;
			x->s.next = undefinedGvars;
			undefinedGvars = (Symbol) x;
		}
		break;
	}
}

Object ppExprList(Object args) {
	if (endp(args))
		return nullObject;
	else
		return newCons(ppExpr(args->c.car), ppExprList(args->c.cdr));
}

Object ppSpecial (Object f, Object args) {
	if (f == Squote) {
		if (endp(args))
			too_few_args(f, args);
		if (!endp(args->c.cdr))
			too_many_args(f, args);

		ppObject(args->c.car);
		switch (typeOf(args->c.car)) {
		case t_Cons:
		case t_Symbol:
			return newCons(f, args->c.car);
		case t_String:
			if (args->c.car->st.self[0] == '\0')
				return nullObject;
			else
				return newCons(f, args->c.car);
		default:
			return args->c.car;
		}
	} else if (f == Slambda) {
		Binding b = currentBinding;
		int prevOffset = vsOffset;
		Object bs = nullObject, params, body;
		int nreq = 0, restp = 0;

		vsOffset = 1;
		ppMarkCCB();

		if (endp(args))
			too_few_args(f, args);
		params = args->c.car;
		for (; typeOf(params) == t_Cons; params = params->c.cdr) {
			bs = newCons(ppBind(params->c.car), bs);
			nreq++;
		}
		if (nreq > maxLAMBDAnreq)
			error1("too many parameters", args->c.car);

		if (params != nullObject) {
			bs = newCons(ppBind(params), bs);
			restp = 1;
		}
		bs = nreverse(bs);

		body = ppExprList(args->c.cdr);

		currentBinding = b;
		vsOffset = prevOffset;

		return listA4(f, newWrapper(MKLAMBDAnargs(nreq, restp)), bs, body);

	} else if (f == Sset) {
		if (endp(args) || endp(args->c.cdr))
			too_few_args(f, args);
		if (!endp(args->c.cdr->c.cdr))
			too_many_args(f, args);

		return listA3(f, ppVref(args->c.car), ppExpr(args->c.cdr->c.car));

	} else if (f == Sif) {
		if (endp(args) || endp(args->c.cdr))
			too_few_args(f, args);
		if (endp(args->c.cdr->c.cdr))
			return listA4(f, ppExpr(args->c.car),
			                 ppExpr(args->c.cdr->c.car),
			                 nullObject);
		else if (endp(args->c.cdr->c.cdr->c.cdr))
			return listA4(f, ppExpr(args->c.car),
			                 ppExpr(args->c.cdr->c.car),
			                 ppExpr(args->c.cdr->c.cdr->c.car));
		else
			too_many_args(f, args);

	} else if (f == Sbegin) {
		if (endp(args))
			return nullObject;
		else if (endp(args->c.cdr))
			return ppExpr(args->c.car);
		else
			return newCons(f, ppExprList(args));

	} else if (f == Swhile) {
		if (endp(args))
			too_few_args(f, args);
		return newCons(f, ppExprList(args));

	} else if (f == Slet) {
		if (endp(args))
			too_few_args(f, args);

		if (typeOf(args->c.car) == t_Symbol) {
			Binding b = currentBinding;
			int prevOffset = vsOffset;

			Object name = args->c.car;
			Object defs, bs, body;
			Object params = nullObject;
			Object vals = nullObject;

			args = args->c.cdr;
			if (endp(args))
				too_few_args(f, args);

			for (defs = args->c.car; !endp(defs); defs = defs->c.cdr)
			{
				Object binding = defs->c.car;
				checkBinding(binding);
				params = newCons(binding->c.car, params);
				vals = newCons(ppExpr(binding->c.cdr->c.car), vals);
			}
			bs = list1(newCons(ppBind(name), nullObject));
			body = list2(ppExpr(list3(Sset, name, 
			                           listA3(Slambda, nreverse(params),
			                                   args->c.cdr))),
			             newCons(ppVref(name), nreverse(vals)));
			currentBinding = b;
			vsOffset = prevOffset;

			return listA3(f, bs, body);

		} else if (args->c.car == nullObject)
			return ppExpr(newCons(Sbegin, args->c.cdr));

		else {
			Binding b = currentBinding;
			int prevOffset = vsOffset;

			Object defs, bs = nullObject, body;
			for (defs = args->c.car; !endp(defs); defs = defs->c.cdr) {
				Object binding = defs->c.car;
				checkBinding(binding);
				bs = newCons(newCons(binding->c.car,
				                      ppExpr(binding->c.cdr->c.car)),
				             bs);
			}
			bs = nreverse(bs);
			for (defs = bs; !endp(defs); defs = defs->c.cdr)
				defs->c.car->c.car = ppBind(defs->c.car->c.car);

			body = ppExprList(args->c.cdr);

			currentBinding = b;
			vsOffset = prevOffset;

			return listA3(f, bs, body);
		}
	} else if (f == SletA) {
		if (endp(args))
			too_few_args(f, args);

		if (args->c.car == nullObject)
			return ppExpr(newCons(Sbegin, args->c.cdr));
		else {
			Binding b = currentBinding;
			int prevOffset = vsOffset;

			Object defs, bs = nullObject, body;
			for (defs = args->c.car; !endp(defs); defs = defs->c.cdr)
			{
				Object binding = defs->c.car, init;
				checkBinding(binding);
				init = ppExpr(binding->c.cdr->c.car);
				bs = newCons(newCons(ppBind(binding->c.car), init), bs);
			}
			bs = nreverse(bs);

			body = ppExprList(args->c.cdr);

			currentBinding = b;
			vsOffset = prevOffset;

			return listA3(f, bs, body);
		}
	} else if (f == Sletrec) {
		if (endp(args))
			too_few_args(f, args);

		if (args->c.car == nullObject)
			return ppExpr(newCons(Sbegin, args->c.cdr));
		else {
			Binding b = currentBinding;
			int prevOffset = vsOffset;

			Object defs, bs = nullObject, ass = nullObject,
			body = args->c.cdr;
			for (defs = args->c.car; !endp(defs); defs = defs->c.cdr)
			{
				Object binding = defs->c.car;
				checkBinding(binding);
				
				bs = newCons(newCons(ppBind(binding->c.car), nullObject),
				              bs);
				ass = newCons(newCons(Sset, binding), ass);
			}
			bs = nreverse(bs);
			for (; !endp(ass); ass = ass->c.cdr)
				body = newCons(ass->c.car, body);

			body = ppExprList(body);

			currentBinding = b;
			vsOffset = prevOffset;

			return listA3(f, bs, body);
		}
	} else if (f == Sand) {
		if (endp(args))
			return trueObject;
		else if (endp(args->c.cdr))
			return ppExpr(args->c.car);
		else
			return newCons(f, ppExprList(args));

	} else if (f == Sor) {
		if (endp(args))
			return falseObject;
		else if (endp(args->c.cdr))
			return ppExpr(args->c.car);
		else
			return newCons(f, ppExprList(args));

	} else if (f == Scatch) {
		if (endp(args))
			too_few_args(f, args);
		return listA3(f, ppExpr(args->c.car), ppExprList(args->c.cdr));

	} else if (f == Swith_watcher) {
		if (endp(args))
			too_few_args(f, args);

		if (args->c.car == nullObject)
			return ppExpr(newCons(Sbegin, args->c.cdr));
		else {
			Binding b = currentBinding;
			int prevOffset = vsOffset;
			Object ws = nullObject, defs;

			vsOffset = 1;
			ppMarkCCB();

			for (defs = args->c.car; !endp(defs); defs = defs->c.cdr) {
				Object def = defs->c.car, cond, body;
				if (endp(def))
					error("no watcher condition");
				cond = ppExpr(def->c.car);
				body = ppExprList(def->c.cdr);
				ws = newCons(newCons(cond, body), ws);
			}
			ws = nreverse(ws);

			currentBinding = b;
			vsOffset = prevOffset;

			return listA3(f, ws, ppExprList(args->c.cdr));
		}
	} else if (f == Swait_until) {
		if (endp(args))
			too_few_args(f, args);
		if (!endp(args->c.cdr))
			too_many_args(f, args);
		return newCons(f, ppExpr(args->c.car));

	} else
		error1("special form in a bad place", f);

	return 0; // just to suppress compiler warning
}

Object ppFuncall(Object args) {
	if (endp(args))
		return nullObject;
	else {
		Object x = ppExpr(args->c.car);
		vsOffset++;
		return newCons(x, ppFuncall(args->c.cdr));
	}
}

Object ppExpr(Object x) {
	switch (typeOf(x)) {
	case t_Cons: {
		Object f = x->c.car;
		if (typeOf(f) == t_Symbol
		     && CONSTP(f->s.clone)
		     && SUBRindex(f->s.clone) >= Fstart)
			return ppSpecial(f, x->c.cdr);
		else {
			int prevOffset = vsOffset;
			Object y = ppFuncall(x);
			vsOffset = prevOffset;
			return y;
		}
	}
	case t_Symbol:
		return ppVref(x);
	case t_Integer:
	case t_Misc:
		return x;
	case t_String:
		if (x->st.self[0] == '\0')
			return nullObject;
		else
			return newCons(Squote, x);
	default:
		baboon("unknown Object to pp");
	}

	return 0; // just to suppress compiler warning
}

Object ppTop(Object x) {

	vsOffset = 1;
	currentBinding = 0;

	ppInit();
	x = ppExpr(x);
	ppFinish();

	return x;
}

// The message generator

int lastHoistedLoc;
int enclosedBindingLength;
int ccbCount;

void wtListA(int n) {
	while (1)
		switch (n) {
		case 1: wtCMD(CMDCONS); return;
		case 2: wtCMD(CMDLISTA2); return;
		case 3: wtCMD(CMDLISTA3); return;
		case 4: wtCMD(CMDLISTA4); return;
		default: wtCMD(CMDLISTA4); n -= 4;
	}
}

void wtList(int n) {
	switch (n) {
	case 0: wtCMD(CMDNIL); return;
	case 1: wtCMD(CMDLIST1); return;
	case 2: wtCMD(CMDLIST2); return;
	case 3: wtCMD(CMDLIST3); return;
	case 4: wtCMD(CMDLIST4); return;
	default: wtCMD(CMDLIST4); wtListA(n - 4); return;
	}
}

void wtExprList(Object args, int isTail, int n) {
	if (args != nullObject) {
		for (; args->c.cdr != nullObject; args = args->c.cdr) {
			wtExpr(args->c.car, 0);
			n++;
		}
		wtExpr(args->c.car, isTail);
		n++;
	}
	wtList(n);
}

void wtObject(Object x, int n) {
	switch (typeOf(x)) {
	case t_Integer:
		wt(valINT(x->i.intVal));
		break;
	case t_Cons:
	case t_String:
		while (typeOf(x) == t_Cons) {
			wtObject(x->c.car, 0);
			x = x->c.cdr;
			n++;
		}
		if (x == nullObject)
			wtList(n);
		else if (typeOf(x) == t_String) {
			unsigned char *s = x->st.self;
			while (*s != '\0') {
				wt(valINT(*s++));
				n++;
			}
			wtList(n);
		} else {
			wtObject(x, 0);
			wtListA(n);
		}
		break;
	case t_Symbol:
		wt(x->s.clone);
		break;
	case t_Misc:
		if (x == nullObject)
			wtCMD(CMDNIL);
		else if (x == trueObject)
			wtCMD(CMDTRUE);
		else if (x == falseObject)
			wtCMD(CMDFALSE);
		else
			baboon("unknown Misc Object to wt");
		break;
	default:
		baboon("unknown Object to wt");
	}
}

void wtSpecial (Object f, Object args, int isTail) {
	if (f == Squote) {
		wt(CONST2SUBR(f->s.clone));
		switch (typeOf(args)) {
		case t_Cons:
		case t_String:
			wtObject(args, 1);
			break;
		case t_Symbol:
			wt(args->s.clone);
			wtCMD(CMDCONS);
			break;
		default:
			baboon("unknown Object to quote");
		}
	} else if (f == Slambda) {
		int prevLoc = lastHoistedLoc;
		int prevLength = enclosedBindingLength;
		Object bs;
		int n = 0;

		wt(MKFlambda(lastHoistedLoc));
		lastHoistedLoc = 0;
		enclosedBindingLength = ccbCount;

		wt(args->c.car->w.clone);
		for (bs = args->c.cdr->c.car; bs != nullObject; bs = bs->c.cdr)
			if (bs->c.car->b.ccbref) {
				wt(MKhoist(Fhoist_arg, lastHoistedLoc));
				wt(valINT(bs->c.car->b.offset));
				wtCMD(CMDCONS);
				lastHoistedLoc = bs->c.car->b.offset;
				bs->c.car->b.ccbref->offset = ++ccbCount;
				n++;
			}
		wtExprList(args->c.cdr->c.cdr, 1, n + 2);

		ccbCount = enclosedBindingLength;
		enclosedBindingLength = prevLength;
		lastHoistedLoc = prevLoc;
	} else if (f == Sset) {
		wt(CONST2SUBR(f->s.clone));
		wtExpr(args->c.car, 0);
		wtExpr(args->c.cdr, 0);
		wtCMD(CMDLISTA2);
	} else if (f == Sif) {
		wt(CONST2SUBR(f->s.clone));
		wtExpr(args->c.car, 0);
		wtExpr(args->c.cdr->c.car, isTail);
		wtExpr(args->c.cdr->c.cdr, isTail);
		wtCMD(CMDLISTA3);
	} else if (f == Sbegin || f == Sand || f == Sor) {
		wt(CONST2SUBR(f->s.clone));
		wtExprList(args, isTail, 1);
	} else if (f == Swhile) {
		wt(CONST2SUBR(f->s.clone));
		wtExprList(args, 0, 1);
	} else if (f == Slet || f == SletA || f == Sletrec) {
		int prevLoc = lastHoistedLoc;
		int prevCount = ccbCount;
		Object bs;
		int n;

		wt(CONST2SUBR(f->s.clone));
		for (bs = args->c.car, n = 0; bs != nullObject; bs = bs->c.cdr, n++)
			if (bs->c.car->c.car->b.ccbref) {
				wt(MKhoist(Fhoist, lastHoistedLoc));
				wtExpr(bs->c.car->c.cdr, 0);
				wtCMD(CMDCONS);
				lastHoistedLoc = bs->c.car->c.car->b.offset;
				bs->c.car->c.car->b.ccbref->offset = ++ccbCount;
			} else
				wtExpr(bs->c.car->c.cdr, 0);
		wtList(n);

		args = args->c.cdr;
		if (isTail) {
			n = 2;
			if (args == nullObject) {
				wtCMD(CMDNIL);
				n++;
			} else {
				for (; args->c.cdr != nullObject; args = args->c.cdr) {
					wtExpr(args->c.car, 0);
					n++;
				}
				wtExpr(args->c.car, 1);
				n++;
			}
			wtCMD(CMDTRUE);
			wtListA(n);
		} else
			wtExprList(args, 0, 2);

		lastHoistedLoc = prevLoc;
		ccbCount = prevCount;
	} else if (f == Scatch) {
		wt(CONST2SUBR(f->s.clone));
		wtExpr(args->c.car, 0);
		wtExprList(args->c.cdr, 0, 2);
	} else if (f == Swith_watcher) {
		int prevLoc = lastHoistedLoc;
		int prevLength = enclosedBindingLength;
		Object ws;
		int n = 0;

		wt(MKFwith_watcher(lastHoistedLoc));
		lastHoistedLoc = 0;
		enclosedBindingLength = ccbCount;

		for (ws = args->c.car; ws != nullObject; ws = ws->c.cdr) {
			wtExpr(ws->c.car->c.car, 0);
			wtExprList(ws->c.car->c.cdr, 0, 1);
			n++;
		}
		wtList(n);

		ccbCount = enclosedBindingLength;
		enclosedBindingLength = prevLength;
		lastHoistedLoc = prevLoc;

		wtExprList(args->c.cdr, 0, 2);

	} else if (f == Swait_until) {
		wt(CONST2SUBR(f->s.clone));
		wtExpr(args, 0);
		wtCMD(CMDCONS);
	} else
		error1("special form in a bad place", f);
}

void wtExpr(Object x, int isTail) {
	switch (typeOf(x)) {
	case t_Integer:
		wt(valINT(x->i.intVal));
		break;
	case t_Cons: {
		Object f = x->c.car;
		if (typeOf(f) == t_Symbol
		     && CONSTP(f->s.clone)
		     && SUBRindex(f->s.clone) >= Fstart)
			wtSpecial(f, x->c.cdr, isTail);
		else {
			if (isTail) {
				wt(CONST2SUBR(MKSPECIAL(Ftail_call)));
				wtExpr(f, 0);
				wtExprList(x->c.cdr, 0, 2);
			} else {
				wtExpr(f, 0);
				wtExprList(x->c.cdr, 0, 1);
			}
		}
		break;
	}
	case t_Symbol:
		wt(x->s.clone);
		break;
	case t_Binding:
		if (x->b.ccbref == Bccbref)
			wt(MKVref(VrefCCB, enclosedBindingLength - x->b.offset));
		else if (x->b.ccbref)
			wt(MKVref(VrefCAR, x->b.offset));
		else
			wt(MKVref(Vref, x->b.offset));
		break;
	case t_Misc:
		if (x == nullObject)
			wtCMD(CMDNIL);
		else if (x == trueObject)
			wtCMD(CMDTRUE);
		else if (x == falseObject)
			wtCMD(CMDFALSE);
		else
			baboon("unknown Misc Object to wt");
		break;
	default:
		baboon("unknown Object to wt");
	}
}

void wtTop(Object x) {
	lastHoistedLoc = 0;
	enclosedBindingLength = 0;
	ccbCount = 0;

	wtExpr(x, 0);
}

#ifndef Native_Win32

int forkSubprog(char* path, Object arglst, int* iport, int* oport)
{
	int len, i;
	char** args;
	int fd_in[2], fd_out[2];
	pid_t child;
	Object p;

	for (len = 1, p = arglst; !endp(p); len++, p = p->c.cdr)
		;
	args = alloca(sizeof(char*) * (len + 1));
	args[0] = path;
	for (i = 1, p = arglst; !endp(p); i++, p = p->c.cdr) {
		Object arg = p->c.car;
		if (typeOf(arg) != t_String)
			error1("not a string", arg);
		args[i] = arg->st.self;
	}
	args[i] = NULL;

	fd_in[0] = -1;
	fd_in[1] = -1;
	fd_out[0] = -1;
	fd_out[1] = -1;

	if (pipe(fd_in) != 0)
		goto ERROR;
	if (pipe(fd_out) != 0)
		goto ERROR;
	if ((child = fork()) == -1)
		goto ERROR;
	
	if (child == 0) {
		close(fd_in[0]);
		close(fd_out[1]);
		dup2(fd_in[1], 1);
		dup2(fd_out[0], 0);
		execvp(path, args);
		perror("exec");
		exit(1);
	}

	close(fd_in[1]);
	close(fd_out[0]);
	portTable[fd_in[0]] = fdopen(fd_in[0], "r");
	portTable[fd_out[1]] = fdopen(fd_out[1], "w");
	*iport = fd_in[0];
	*oport = fd_out[1];

	return 0;

 ERROR:
	close(fd_in[0]);
	close(fd_out[1]);
	close(fd_in[0]);
	close(fd_out[1]);
	return -1;
}

#endif // ifndef Native_Win32

// The top-level expression handler

void topExpr(Object x) {
	Object head;
#ifdef ONLINE
	traceLevel = 0;
#endif
	if (typeOf(x) == t_Cons && typeOf(head = x->c.car) == t_Symbol) {
		Object args = x->c.cdr;
		if (head == Sdefine) {
			Object name, init;
			if (endp(args))
				too_few_args(head, args);
			name = args->c.car;
			if (typeOf(name) == t_Symbol) {
				if (endp(args->c.cdr))
					too_few_args(head, args);
				if (!endp(args->c.cdr->c.cdr))
					too_many_args(head, args);
				init = args->c.cdr->c.car;
			} else {
				if (typeOf(name) != t_Cons)
					error1("not a symbol nor a pair", name);
				init = listA3(Slambda, name->c.cdr, args->c.cdr);
				name = name->c.car;
				checkSymbol(name);
			}
			if (CONSTP(name->s.clone)) {
				if (SUBRindex(name->s.clone) >= Fstart)
					error1("special form may not be redefined", name);
				else
					error1("built-in function may not be redefined", name);
			}
			init = ppTop(init);

			ppInit();
			ppObject(name);
			ppFinish();
			wtInit();
			wtTop(init);
			wt(name->s.clone);
			wtCMD(CMDDEFINE);
			transmit();
		} else if (head == Sload) {
			FILE *in;

			if (endp(args))
				too_few_args(head, args);
			if (!endp(args->c.cdr))
				too_many_args(head, args);
			if (typeOf(args->c.car) != t_String)
				error1("not a string", args->c.car);

			if (!(in = fopen(args->c.car->st.self, "r")))
				error1("file not found", args->c.car);
			while ((x = readObject(in, TAeof)) != eofObject)
				topExpr(x);
			fclose(in);
			return;
		} else if (head == Slast_value) {
			if (!endp(args))
				too_many_args(head, args);

			transmitCMD(CMDLASTVAL);
//		} else if (head == Sping) {
//			if (!endp(args))
//				too_many_args(head, args);
//
//			transmitCMD(CMDPING);
//
		} else if (head == Sbye) {
			if (!endp(args))
				too_many_args(head, args);

			longjmp(mainEnv, 1);
			
		} else if (head == Strace) {
			Object name, init;
			object sym;

			if (endp(args))
				too_few_args(head, args);
			if (!endp(args->c.cdr))
				too_many_args(head, args);

			name = x->c.cdr->c.car;
			checkSymbol(name);
			sym = name->s.clone;
			if (CONSTP(sym)) {
				if (SUBRindex(name->s.clone) >= Fstart)
					error1("special form may not be traced", name);
				else
					error1("built-in function may not be traced", name);
			} else if (sym == UNDEFINED)
				error1("function not defined", name);
			
			// (let ((name ',name) (fun ,name))
			//   (lambda args (trace-call name fun args)))
			init = list3(Slet,
			             list2(list2(Sname, list2(Squote, name)),
			                    list2(Sfun, name)),
			             list3(Slambda, Sargs,
			                    list4(Strace_call, Sname, Sfun, Sargs)));
			init = ppTop(init);
			wtInit();
			wtTop(init);
			wt(sym);
			wtCMD(CMDDEFINE);
			transmit();
		}
		else if (head == Suntrace) {
			Object name;
			object sym;

			if (endp(args))
				too_few_args(head, args);
			if (!endp(args->c.cdr))
				too_many_args(head, args);

			name = x->c.cdr->c.car;
			checkSymbol(name);
			sym = name->s.clone;
			if (!GVARP(sym))
				error1("not traced", name);
			wtInit();
			wt(sym);
			wtCMD(CMDUNTRACE);
			transmit();
		}

#ifndef Native_Win32
		else if (head == Sfork) {
			Object oname, iname, path;
			int iport, oport;
			if (endp(args) || endp(args->c.cdr) || endp(args->c.cdr->c.cdr))
				too_few_args(head, args);
			oname = args->c.car;
			iname = args->c.cdr->c.car;
			path = args->c.cdr->c.cdr->c.car;
			if (typeOf(path) != t_String)
				error1("not a string", path);
			args = args->c.cdr->c.cdr->c.cdr;

			forkSubprog(path->st.self, args, &iport, &oport);

			topExpr(list3(Sdefine, oname, newInteger(oport)));
			topExpr(list3(Sdefine, iname, newInteger(iport)));
			return;
		}
#endif // ifndef Native_Win32

		else {
			x = ppTop(x);
			wtInit();
			wtTop(x);
			wtCMD(CMDEVAL);
			transmit();
		}
	} else {
		x = ppTop(x);
		wtInit();
		wtTop(x);
		wtCMD(CMDEVAL);
		transmit();
	}
#ifdef ONLINE
AGAIN:
	x = receive1();
	if (x == abortObject)
		fputs("Error: cannot print the result, maybe a cyclic structure\n",
		       stdout);
	else if (x == errorObject) {
		transmitCMD(CMDMSG);
		goto AGAIN;
	} else if (x != messageObject) {
		writeObject(stdout, x);
		putchar('\n');
	}
#endif
}

int main(int argc, char **argv) {
	Object x;

#ifdef IRCOM
    baud = RCX_DEFAULT_BAUD;
    timeout = RCX_DEFAULT_TIMEOUT;
#endif

#ifndef Native_Win32
	signal(SIGCHLD, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
#endif
	initPort();
	initAllocator();
	initReader();
	initGlobals();

    // Command-line argument support
    int opt;
#ifdef HAVE_GETOPT_LONG
    int option_index;
#endif
    char *usage_string =
    "usage: %s [options]\n\n"
	"Options:\n"
	"  -t<device>   , --tty=<device>        set the IR device to <device>\n"
	"    <device> may take one of the following formats:\n"
#if defined(_WIN32) || defined (__CYGWIN__)
	"                 serial:<device name>  (e.g. serial:COM1)\n"
	"                 usb[:<device name>]   (e.g. usb or usb:\\\\\\\\.\\\\legotower1)\n"
#else
	"                 serial:<device name>  (e.g. serial:/dev/ttyS0)\n"
	"                 usb:<device name>     (e.g. usb:/dev/legousbtower0)\n"
#endif
	"                 tcp:<host>[:<port>]   (e.g. tcp:localhost:50637)\n"
	"                 ncd:<host>:<port>     (e.g. tcp:localhost:87)\n"
	"  -b<rate>     , --baud=<rate>         baud rate (e.g. 2400, 4800)\n"
	"  -o<timeout>  , --timeout=<mseconds>  timeout in milliseconds\n"
	"  -h           , --help                display this help\n"
	"  -v           , --verbose             verbose mode\n"
	"\n"
	"Default tty device can be set using one of the following methods:\n"
	" * Environment variable RCXTTY.\n"
	"      Eg:\tset RCXTTY=tcp:localhost:50637\n"
	" * Configuration file ~/.rcx/device.conf\n"
	"      Specify <device> as the first line of the file\n"
	"\n"
	;

    // Process command-line arguments
    while((opt=getopt_long(argc, argv, "t:b:o:hv",
                        (struct option *)long_options, &option_index) )!=-1) {
        switch(opt) {
          case 'b':
            baud = atoi(optarg);
            break;
          case 'o':
            timeout = atoi(optarg);
            break;
          case 'h':
            fprintf(stderr, usage_string, argv[0]);
            return 1;
            break;
          case 't':
            tty_name = optarg;
            break;
          case 'v':
 	    	com_debug = 1;
#ifdef IRCOM
            rcx_set_debug(1);
#endif
            break;
        }
    }

	begin_rcx();
#ifdef ONLINE
	signal(SIGINT,sigint);
#endif
	fputs("Welcome to XS: Lisp on Lego MindStorms\n\n", stdout);

	if (setjmp(mainEnv) == 0)
		while (1) {
			putchar('>'); fflush(stdout);
			if (setjmp(topEnv) == 0) {
				x = readObject(stdin, TAeof);
				if (x == eofObject)
					break;
				topExpr(x);
			} else
				printf("at toplevel\n");
			freeObjects();
		}
#ifdef ONLINE
	transmitCMD(CMDBYE);
#endif
	fputs("sayonara\n", stdout);
	end_rcx();

	return 0; // just to suppress compiler warning
}
