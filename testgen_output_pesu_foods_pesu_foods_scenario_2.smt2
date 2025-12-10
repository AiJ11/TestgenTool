(set-logic ALL)
(set-option :produce-models true)

;; helper / uninterpreted declarations for predicates used by encoding
(declare-fun in (String (Array String Bool)) Bool)
(declare-fun add_to_set (String String) Bool)
(declare-fun not_empty (String) Bool)

;; Variable declarations (String type)
(declare-fun v1 () String)
(declare-fun v2 () String)
(declare-fun v3 () String)
(declare-fun v4 () String)
(declare-fun v5 () String)
(declare-fun v6 () String)
(declare-fun v7 () String)

;; Map declarations (Array String String/Bool)
; Map: T
(declare-const Dom_T (Array String Bool))
(declare-const Val_T (Array String String))

; Map: T_old
(declare-const Dom_T_old (Array String Bool))
(declare-const Val_T_old (Array String String))

; Map: U
(declare-const Dom_U (Array String Bool))
(declare-const Val_U (Array String String))

; Map: U_old
(declare-const Dom_U_old (Array String Bool))
(declare-const Val_U_old (Array String String))

;; Domain arrays left unconstrained initially

(assert (! (= v1 "B") :named c1))
(assert (! (= v2 "A") :named c2))
(assert (! (= v3 "C") :named c3))
(assert (! (not (select Dom_U_old v2)) :named c4))
(assert (! (and (select Dom_U v2) (= (select Val_U v2) v3)) :named c5))
(assert (! (= v4 "A") :named c6))
(assert (! (= v5 "C") :named c7))
(assert (! (and (select Dom_U v4) (= (select Val_U v4) v5)) :named c8))
(assert (! (and (select Dom_T v6) (= (select Val_T v6) v4)) :named c9))
(assert (! (= v7 "C") :named c10))
(assert (! (in v7 Dom_T) :named c11))
(assert (! (true) :named c12))

(check-sat)
(get-model)
