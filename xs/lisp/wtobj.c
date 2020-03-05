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

char *subr_name(int index) {
	switch (index) {
		case Lcar: return "car";
		case Lcdr: return "cdr";
		case Lcons: return "cons";
		case Lset_car: return "set-car!";
		case Lset_cdr: return "set-cdr!";
		case LpairP: return "pair?";
		case LnullP: return "null?";
		case Lnot: return "not";

		case Llist: return "list";
#ifdef LISTLIB
		case LlistA: return "list*";
		case Llist_ref: return "list-ref";
		case Lappend: return "append";
		case Lassoc: return "assoc";
		case Lmember: return "member";
		case Llength: return "length";
		case Lreverse: return "reverse";
#endif

		case Leq: return "eq?";
		case LbooleanP: return "boolean?";
		case Lplus: return "+";
		case Lminus: return "-";
		case Ltimes: return "*";
		case Lquotient: return "/";
		case Lremainder: return "remainder";
		case LLT: return "<";
		case LGT: return ">";
		case LEQ: return "=";
		case LLE: return "<=";
		case LGE: return ">=";
		case Llogand: return "logand";
		case Llogior: return "logior";
		case Llogxor: return "logxor";
		case Llogshl: return "logxshl";
		case Llogshr: return "logxshr";
		case Lrandom: return "random";
		case LintegerP: return "integer?";
		case Ltrace_call: return "trace-call";
		case Lapply: return "apply";
		case LfunctionP: return "function?";
		case LsymbolP: return "symbol?";
		case Lthrow: return "throw";
		case Lmotor: return "motor";
		case Lspeed: return "speed";
		case Llight_on: return "light-on";
		case Llight_off: return "light-off";
		case Llight: return "light";
		case Lrotation_on: return "rotation-on";
		case Lrotation_off: return "rotation-off";
		case Lrotation: return "rotation";
		case Ltouched: return "touched?";
		case Ltemperature: return "temperature";
		case Lplay: return "play";
		case Lplaying: return "playing?";
		case Lpressed: return "pressed?";
		case Lputs: return "puts";
		case Lputc: return "putc";
		case Lcls: return "cls";
		case Lbattery: return "battery";
		case Lreset_time: return "reset-time";
		case Ltime: return "time";
		case Lsleep: return "sleep";
		case Llinked: return "linked?";
		case Lread: return "read";
		case Lwrite: return "write";
		case Lread_char: return "read-char";
		case Lwrite_char: return "write-char";
		case Lread_line: return "read-line";
		case Lwrite_string: return "write-string";

		case Fquote: return "quote";
		case Flambda: return "lambda";
		case Fset: return "set!";
		case Fif: return "if";
		case Fbegin: return "begin";
		case Flet: return "let";
		case FletA: return "let*";
		case Fletrec: return "letrec";
		case Fand: return "and";
		case For: return "or";
		case Fcatch: return "catch";
		case Fwith_watcher: return "with-watcher";
		case Fwait_until: return "wait-until";
		case Fhoist: return "hoist";
		case Fhoist_arg: return "hoist-arg";
		case Ftail_call: return "tail-call";
		default: return "#<unknown SUBR>";
	}
}

void write_object(object x) {
	if (PAIRP(x)) {
		putchar('(');
		write_object(CAR(x));
		while (PAIRP(x = CDR(x))) {
			putchar(' ');
			write_object(CAR(x));
		}
		if (x != NIL) {
			fputs(" . ", stdout);
			write_object(x);
		}
		putchar(')');
	} else if (LAMBDAP(x))
		fputs("#<function>", stdout);
	else if (SUBRP(x)) {
		if (SUBRindex(x)>=Fstart)
			printf("#<%s>", subr_name(SUBRindex(x)));
		else
			printf("#<function %s>", subr_name(SUBRindex(x)));
	} else if (GVARP(x))
#ifdef JOINT
		fputs(gvarSymbol(x)->s.name, stdout);
#else
		printf("#<symbol #x%x>", x);
#endif
	else if (CONSTP(x))
		fputs(subr_name(SUBRindex(x)), stdout);
	else if (INTP(x))
		printf("%d", INTval(x));
	else if (x == FALSE)
		fputs("#f", stdout);
	else if (x == TRUE)
		fputs("#t", stdout);
	else if (x == NIL)
		fputs("()", stdout);
	else if (x == UNDEFINED)
		fputs("#<undefined>", stdout);
	else if (x == ERR)
		fputs("#<error>", stdout);
	else
		printf("#<object #x%x>", x);
}

void print_object(object x) {
	write_object(x);
	putchar('\n');
}
