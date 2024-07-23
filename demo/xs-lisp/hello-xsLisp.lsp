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
;;  Contributor(s): Matthew Sheets

;; hello world

(begin
 (putc-native :char-H 4)
 (putc-native :char-E 3)
 (putc-native :char-parallel 2)
 (putc-native :char-O 1)
 (msleep 1000)

 (putc-native :char-L 4)
 (putc-native :char-E 3)
 (putc-native :char-G 2)
 (putc-native :char-O 1)
 (sleep 1)

 (cls)
 (sleep 1)
)
