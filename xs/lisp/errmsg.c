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

char *error_message(int id) {
	switch (id) {
	case EXbaboon: return "rcx baboon";
	case EXundefined_var: return "undefined variable";
	case EXnot_function: return "not a function";
	case EXnot_pair: return "not a pair";
	case EXnot_list: return "not a list";
	case EXnot_integer: return "not an integer";
	case EXbad_number: return "bad number";
	case EXbad_port_number: return "bad port number";
	case EXbad_alist: return "bad alist";
	case EXtoo_few_args: return "too few arguments";
	case EXtoo_many_args: return "too many arguments";
	case EXheap_full: return "heap full";
	case EXno_catcher: return "no catcher";
	case EXunsupported_rcx_fun: return "unsupported RCX function";
	case EXinterrupt: return "terminal interrupt";
	case EXvs_overflow: return "RCX value stack overflow";
	case EXstack_overflow: return "RCX C stack overflow";
	case EXfront_error: return "front-end error"; // never used
	default: return "unknown error number";
	}
}
