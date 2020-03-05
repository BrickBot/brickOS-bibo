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

(let ((dir :off))
  ;; attach rotation sensor to port 1.
  ;; attach touch sensor to port 3.
  (speed :a (speed :c (speed :b :max-speed)))
  (rotation-on 1)
  (let loop ()
    (if (touched? 2)
      	(begin
      	  (motor :a (motor :c (motor :b :off)))
      	  (rotation-off 2))
      	(begin
      	  (if (touched? 3)
      	      (begin
      	        (set! dir
      	          (if (= dir :off) :forward
      	            (if (= dir :forward) :brake
      	              (if (= dir :brake) :back :off))))
      	        (motor :b (if (= dir :back) :back :off))
      	        (motor :a (motor :c dir))
      	        (rotation-on 1)
      	        (sleep 3)))
      	  (let ((rot (rotation 1)))
      	    (if (>= rot 0)
      	      (begin
      	        (speed :a
      	          (- :max-speed
      	            (/ (* :max-speed (if (> rot 4) 4 rot)) 4)))
      	        (speed :c :max-speed))
      	      (begin
      	        (speed :c
      	          (- :max-speed
      	            (/ (* :max-speed (if (< rot -4) 4 (- rot))) 4)))
      	        (speed :a :max-speed))))
      	  (loop)))))
