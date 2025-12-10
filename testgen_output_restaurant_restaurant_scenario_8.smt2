(set-logic ALL)
(set-option :produce-models true)

;; helper / uninterpreted declarations for predicates used by encoding
(declare-fun in (String (Array String Bool)) Bool)
(declare-fun add_to_set (String String) Bool)
(declare-fun not_empty (String) Bool)

(declare-fun authenticated (String) Bool)
(declare-fun token_present (String) Bool)
(declare-fun cart_contains (String String) Bool)
(declare-fun order_recorded (String String) Bool)
(declare-fun result_is_restaurant_list () Bool)
(declare-fun result_is_menu () Bool)
(declare-fun result_is_cart () Bool)
(declare-fun review_added () Bool)

;; Variable declarations (String type)
(declare-fun v1 () String)
(declare-fun v2 () String)
(declare-fun v3 () String)
(declare-fun v4 () String)
(declare-fun v5 () String)
(declare-fun v6 () String)
(declare-fun v7 () String)
(declare-fun v8 () String)
(declare-fun v9 () String)

;; Map declarations (Array String String/Bool)
; Map: T
(declare-const Dom_T (Array String Bool))
(declare-const Val_T (Array String String))

; Map: U
(declare-const Dom_U (Array String Bool))
(declare-const Val_U (Array String String))

;; Domain arrays left unconstrained initially

(assert (! (= v1 "A") :named c1))
(assert (! (= v2 "B") :named c2))
(assert (! (registered v1) :named c3))
(assert (! (authenticated v1) :named c4))
(assert (! (= v3 "B") :named c5))
(assert (! (and (token_present v4)) :named c6))
(assert (! (restaurant_created v3) :named c7))
(assert (! (= v5 "B") :named c8))
(assert (! (and (token_present v6)) :named c9))
(assert (! (menu_contains v5) :named c10))
(assert (! (= v7 "B") :named c11))
(assert (! (and (token_present v8)) :named c12))
(assert (! (menu_contains v7) :named c13))
(assert (! (and (token_present v9)) :named c14))
(assert (! (result_is_order_list) :named c15))

(check-sat)
(get-model)
