(define threshold 50)

(define (on-black) (< (light 1) threshold))
(define (on-white) (> (light 1) threshold))

(define (left)
  (motor :c :off)
  (wait-until (on-black))
  (wait-until (on-white))
  (motor :c :back)
  (right))

(define (right)
  (motor :a :off)
  (wait-until (on-black))
  (wait-until (on-white))
  (motor :a :back)
  (left))

(define (start)
  (write-string "Press Prgm button to start")
  (wait-until (pressed?))
  (wait-until (not (pressed?)))
  
  (speed :a (speed :c :max-speed))
  (motor :a (motor :c :back))
  (light-on 1)

  (catch 'end
    (with-watcher (((pressed?) (throw 'end 0)))
      (wait-until (on-black))
      (wait-until (on-white))
      (left)))

  (motor :a (motor :c :off))
  (light-off 1)
  )

(start)
