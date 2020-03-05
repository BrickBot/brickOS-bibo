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

(speed :a (speed :c 100))

(define (start f . args)
  (write-string "Press Prgm button when ready")
  (wait-until (pressed?))
  (wait-until (not (pressed?)))
  (catch 'end
    (with-watcher (((pressed?) (throw 'end 0)))
      (apply f args)))
  (motor :a (motor :c :off))
  )
(define (approach)
  (with-watcher (((touched? 2) (throw 'end 0)))
    (let loop ()
      (look)
      (bugle1) (wait-until (not (playing?)))
      (motor :a (motor :c :forward)) 
      (wait-until (not (linked?)))
      (motor :b (motor :a (motor :c :back)))
      (sleep 10)
      (motor :b :off)
      (loop)))
  )
(define (look)
  (turn-left) (wait-until (not (linked?)))
  (turn-right) (wait-until (linked?))
  (reset-time)
  (wait-until (not (linked?)))
  (turn-left)
  (sleep (+ (/ (time) 2) 4))
  (motor :a (motor :c :off)) 
  )
(define (turn-right)
  (motor :a :forward) (motor :c :back)
  )
(define (turn-left)
  (motor :a :back) (motor :c :forward)
  )

;; this code does not work any more (03/12/14)

(define (bugle)
  (play '((:Do4 . 2) (:Do4 . 1) (:Do4 . 2) (:Do4 . 1) (:Do4 . 2) (:Do4 . 1)
          (:So4 . 2) (:Mi4 . 1) (:So4 . 2) (:Mi4 . 1) (:So4 . 2) (:Mi4 . 1)
          (:Do4 . 2) (:Do4 . 1) (:Do4 . 2) (:Do4 . 1) (:Do4 . 2) (:Do4 . 1)
          (:So4 . 2) (:Mi4 . 1) (:So4 . 2) (:Mi4 . 1) (:So4 . 2) (:Mi4 . 1)
          (:Do4 . 6)))
  )
(define (bugle1)
  (play '((:So3 . 1) (:So3 . 1) (:Do4 . 2) (:Do4 . 2) (:Do4 . 2)
          (:So3 . 1) (:Do4 . 1) (:Mi4 . 2) (:Mi4 . 2) (:Mi4 . 2)
          (:Do4 . 1) (:Mi4 . 1) (:So4 . 2) (:Mi4 . 1) (:Do4 . 1)
          (:So4 . 2) (:Mi4 . 1) (:Do4 . 1)
          (:So3 . 2) (:So3 . 1) (:So3 . 1) (:So3 . 2)))
  )
