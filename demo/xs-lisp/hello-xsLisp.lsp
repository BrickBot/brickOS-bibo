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
 (putc-native-user :char-H :char-E :char-parallel)
 (msleep 1000)

 (putc-native-user :char-L :char-E :char-G)
 (sleep 1)

 (cls)
 (sleep 1)

 (putc-native-user :char-F :char-L :char-I)
 (putc-native :char-dash 5)
 (sleep 1)

 (cls)
 (sleep 1)

)
