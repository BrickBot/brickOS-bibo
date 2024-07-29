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

(begin
  (speed :a (speed :c (speed :b :max-speed)))
  (write-string "Press Prgm button to start")
  (wait-until (pressed?))
  (wait-until (not (pressed?)))
  (let loop ()
    (motor :a (motor :c :forward))
    (motor :b :off)
    (play '((:RE4 . 2) (:DO4 . 1) (:RE4 . 1) (:FA4 . 1) (:RE4 . 1)
            (:RE4 . 2) (:FA4 . 2) (:SO4 . 1) (:DO5 . 1) (:LA4 . 2)
            (:RE4 . 2)))
    (wait-until (or (touched? 2) (pressed?)))
    (if (pressed?)
        (motor :a (motor :c :off))
        (begin
          (motor :a (motor :c (motor :b :back)))
          (sleep 5)
          (motor (if (= (random 2) 0) :a :c) :forward)
          (sleep 5)
          (loop))
        )
    )
  )
