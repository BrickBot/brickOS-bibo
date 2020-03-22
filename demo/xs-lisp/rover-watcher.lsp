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

(define (forward)
  (motor :a (motor :c :forward))
  (motor :b :off)
  (play '((:Re4 . 2) (:Do4 . 1) (:Re4 . 1) (:Fa4 . 1) (:Re4 . 1)
          (:Re4 . 2) (:Fa4 . 2) (:So4 . 1) (:Do5 . 1) (:La4 . 2)
          (:Re4 . 2)))
)

(begin
  (speed :a (speed :c (speed :b :max-speed)))
  (wait-until (pressed?))
  (wait-until (not (pressed?)))
  (forward)
  (with-watcher
    (((touched? 2) 
      (motor :a (motor :c (motor :b :back)))
      (sleep 5)
      (motor (if (= (random 2) 0) :a :c) :forward)
      (sleep 5)
      (forward)
      ))
    (wait-until (pressed?))
    (motor :a (motor :c :off))
    )
  )
