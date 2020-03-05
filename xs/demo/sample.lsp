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

;; hello world

(puts "hello")
(getc)
(puts "world")
(getc)
(cls)

;; direct output

(putc #\a 0) ; a
(putc #\b 1) ; b
(putc #\c 2) ; c
(putc #\d 3) ; d
(putc #\e 4) ; e

;; light sensor

(set-light 2 #t) ; on

(wait-until (< (light 2) 55))

(set-light 2 #f) ; off

;; robots

(define (robots n)
  (if (> n 0)
      (begin
        (play '((:Re4 . 2) (:Do4 . 1) (:Re4 . 1) (:Fa4 . 1) (:Re4 . 1)
                (:Re4 . 2) (:Fa4 . 2) (:So4 . 1) (:Do5 . 1) (:La4 . 2)
                (:Re4 . 2)))
        (wait-until (not (playing?)))
        (robots (- n 1)))))

(robots 3)

(define (robots2 n)
  (if (> n 0)
      (begin
        (play '((:D4 . 2) (:C4 . 1) (:D4 . 1) (:F4 . 1) (:D4 . 1)
                (:D4 . 2) (:F4 . 2) (:G4 . 1) (:C5 . 1) (:A4 . 2)
                (:D4 . 2)))
        (wait-until (not (playing?)))
        (robots2 (- n 1)))))

(robots2 3)

;; rover

(define (go n)
  (if (> n 0)
      (begin
        (motor :a :forward 128)
        (motor :c :forward 128)

        (wait-until (or (touched? 1) (touched? 3)))

        (let ((m (if (touched? 1) :a :c)))

          (motor :a :back)
          (motor :c :back)

          (sleep 5)

          (motor m :forward)
          )

        (sleep 5)

        (go (- n 1))
        )
      ))

(go 5)

;; even-odd

(define (even x) (if (= x 0) #t (odd (- x 1))))
(define (odd x) (if (= x 0) #f (even (- x 1))))
(trace even)
(even 10)

(define (even x) (letrec ((e (lambda (x) (if (= x 0) #t (o (- x 1)))))
                          (o (lambda (x) (if (= x 0) #f (e (- x 1))))))
                   (e x)))

;; buffer/value_stack overflow

(define (mk n) (mk1 n '()))

(define (mk1 n x) (if (<= n 0) x (mk1 (- n 1) (cons n x))))

(define x
  '(1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
     28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51
     52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75
     76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99
     100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117
     118 119 120 121 122 123 124 125 126 127 128 129 130 131 132 133 134 135
     136 137 138 139 140 141 142 143 144 145 146 147 148 149 150
     )
  )

(define y
  '(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
     27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49
     50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72
     73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95
     96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113
     114 115 116 117 118 119 120 121 122 123 124 125 126 127 128 129 130
     131 132 133 134 135 136 137 138 139 140 141 142 143 144 145 146 147
     148 149 150 151 152 153 154 155 156 157 158 159 160 161 162 163 164
     165 166 167 168 169 170 171 172 173 174 175 176 177 178 179 180 181
     182 183 184 185 186 187 188 189 190 191 192 193 194 195 196 197 198
     199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215
     216 217 218 219 220 221 222 223 224 225 226 227 228 229 230 231 232
     233 234 235 236 237 238 239 240 241 242 243 244 245 246 247 248 249
     250 251 252 253 254 255 256 257 258 259 260 261 262 263 264 265 266
     267 268 269 270 271 272 273 274 275 276 277 278 279 280 281 282 283
     284 285 286 287 288 289 290 291 292 293 294 295 296 297 298 299))

;; end
