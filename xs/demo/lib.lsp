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

;(define (list . args) args)

(define (list* x . rest)
  (let down ((x x) (rest rest) (stack '()))
    (if (null? rest)
        (let up ((stack stack) (val x))
          (if (null? stack)
              val
              (up (cdr stack) (cons (car stack) val))))
        (down (car rest) (cdr rest) (cons x stack)))))

(define (list-ref x n)
  (if (null? x)
      '()
      (if (<= n 0)
          (car x)
          (list-ref (cdr x) (- n 1)))))

(define (append x . rest)
  (let down ((x x) (rest rest) (stack '()))
    (if (null? rest)
        (let up ((stack stack) (val x))
          (if (null? stack)
              val
              (up (cdr stack) (cons (car stack) val))))
        (let down1 ((x x) (stack stack))
          (if (null? x)
              (down (car rest) (cdr rest) stack)
              (down1 (cdr x) (cons (car x) stack)))))))

(define (assoc x y)
  (if (null? y)
      '()
      (if (eq? (car (car y)) x)
          (car y)
          (assoc x (cdr y)))))

(define (member x y)
  (if (null? y)
      '()
      (if (eq? (car y) x)
          y
          (member x (cdr y)))))

(define (length x)
  (let aux ((x x) (n 0))
    (if (null? x)
        n
        (aux (cdr x) (+ n 1)))))

(define (reverse x)
  (let aux ((x x) (val '()))
    (if (null? x)
        val
        (aux (cdr x) (cons (car x) val)))))
