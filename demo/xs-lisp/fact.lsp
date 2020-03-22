;;  The contents of this file are subject to the Mozilla Public License
;;  Version 1.0 (the "License"); you may not use this file except in
;;  compliance with the License. You may obtain a copy of the License at
;;  http://www.mozilla.org/MPL/
;;
;;  Software distributed under the License is distributed on an "AS IS"
;;  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
;;  License for the specific language governing rights and limitations
;;  under the License.
;;
;;  The Original Code is XS code, released February 4, 2004.
;;
;;  The Initial Developer of the Original Code is Taiichi Yuasa.
;;  Portions created by Taiichi Yuasa are Copyright (C) 2004
;;  Taiichi Yuasa. All Rights Reserved.
;;
;;  Contributor(s): Taiichi Yuasa <yuasa@kuis.kyoto-u.ac.jp>

(define (fact x) (if (= x 0) 1 (* x (fact (- x 1)))))
(trace fact)
(fact 4)
