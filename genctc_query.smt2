(set-logic ALL)
(set-option :produce-models true)

;; helper / uninterpreted declarations for predicates used by encoding
(declare-fun in (String (Array String Bool)) Bool)
(declare-fun add_to_set (String String) Bool)
(declare-fun not_empty (String) Bool)

(declare-fun authenticated (String) Bool)
(declare-fun token_present (String) Bool)
(declare-fun cart_contains (String String) Bool)
(declare-fun order_recorded (String) Bool)
(declare-fun result_is_restaurant_list () Bool)
(declare-fun result_is_menu () Bool)
(declare-fun result_is_cart () Bool)
(declare-fun review_added () Bool)

(declare-fun order_exists (String) Bool)
(declare-fun registered (String) Bool)
(declare-fun not_registered (String) Bool)
(declare-fun restaurant_created (String) Bool)
(declare-fun restaurant_updated (String) Bool)
(declare-fun menu_contains (String) Bool)
(declare-fun order_assigned (String String) Bool)
(declare-fun order_status_updated (String String) Bool)
(declare-fun result_is_order_list () Bool)

(declare-fun availability_updated (String) Bool)
(declare-fun menu_item_deleted (String) Bool)
;; Last API status observed by visitor
(declare-const lastApiStatus Int)

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
(declare-fun v10 () String)

;; Map declarations (Array String String/Bool) - deduped
; Map domain: C
(declare-const Dom_C (Array String Bool))
(declare-const Val_C (Array String String))

(declare-const Dom_C_old (Array String Bool))
(declare-const Val_C_old (Array String String))

; Map domain: M
(declare-const Dom_M (Array String Bool))
(declare-const Val_M (Array String String))

(declare-const Dom_M_old (Array String Bool))
(declare-const Val_M_old (Array String String))

; Map domain: O
(declare-const Dom_O (Array String Bool))
(declare-const Val_O (Array String String))

(declare-const Dom_O_old (Array String Bool))
(declare-const Val_O_old (Array String String))

; Map domain: R
(declare-const Dom_R (Array String Bool))
(declare-const Val_R (Array String String))

(declare-const Dom_R_old (Array String Bool))
(declare-const Val_R_old (Array String String))

; Map domain: Rev
(declare-const Dom_Rev (Array String Bool))
(declare-const Val_Rev (Array String String))

(declare-const Dom_Rev_old (Array String Bool))
(declare-const Val_Rev_old (Array String String))

; Map domain: T
(declare-const Dom_T (Array String Bool))
(declare-const Val_T (Array String String))

(declare-const Dom_T_old (Array String Bool))
(declare-const Val_T_old (Array String String))

; Map domain: U
(declare-const Dom_U (Array String Bool))
(declare-const Val_U (Array String String))

(declare-const Dom_U_old (Array String Bool))
(declare-const Val_U_old (Array String String))

;; Domain/value arrays left unconstrained initially

;; Link predicates to state arrays for proper constraint solving
(assert (forall ((x String)) (= (registered x) (select Dom_U x))))
(assert (forall ((x String)) (= (not_registered x) (not (select Dom_U x)))))
(assert (forall ((x String)) (= (token_present x) (exists ((t String)) (and (select Dom_T t) (= (select Val_T t) x))))))
(assert (forall ((x String)) (= (authenticated x) (token_present x))))

;; Initialize _old state arrays to empty (simulates clean database)
(assert (= Dom_U_old ((as const (Array String Bool)) false)))
(assert (= Val_U_old ((as const (Array String String)) "")))
(assert (= Dom_T_old ((as const (Array String Bool)) false)))
(assert (= Val_T_old ((as const (Array String String)) "")))
(assert (= Dom_O_old ((as const (Array String Bool)) false)))
(assert (= Val_O_old ((as const (Array String String)) "")))
(assert (= Dom_C_old ((as const (Array String Bool)) false)))
(assert (= Val_C_old ((as const (Array String String)) "")))
(assert (= Dom_R_old ((as const (Array String Bool)) false)))
(assert (= Val_R_old ((as const (Array String String)) "")))
(assert (= Dom_M_old ((as const (Array String Bool)) false)))
(assert (= Val_M_old ((as const (Array String String)) "")))
(assert (= Dom_Rev_old ((as const (Array String Bool)) false)))
(assert (= Val_Rev_old ((as const (Array String String)) "")))

;; Frame axioms: maps not updated equal their old versions
(assert (= Dom_T Dom_T_old))
(assert (= Val_T Val_T_old))
(assert (= Dom_O Dom_O_old))
(assert (= Val_O Val_O_old))
(assert (= Dom_C Dom_C_old))
(assert (= Val_C Val_C_old))
(assert (= Dom_R Dom_R_old))
(assert (= Val_R Val_R_old))
(assert (= Dom_M Dom_M_old))
(assert (= Val_M Val_M_old))
(assert (= Dom_Rev Dom_Rev_old))
(assert (= Val_Rev Val_Rev_old))

(assert (! (= v1 "E") :named c1))
(assert (! (= v2 "A") :named c2))
(assert (! (= v3 "C") :named c3))
(assert (! (= v4 "B") :named c4))
(assert (! (= v5 "D") :named c5))
(assert (! true :named c6))
(assert (! (= Dom_U (store Dom_U_old v2 true)) :named c7))
(assert (! (= Val_U (store Val_U_old v2 v3)) :named c8))
(assert (! (select Dom_U v2) :named c9))
(assert (! (= v6 "E") :named c10))
(assert (! (= v7 "A") :named c11))
(assert (! (= v8 "C") :named c12))
(assert (! (= v9 "B") :named c13))
(assert (! (= v10 "D") :named c14))
(assert (! (not (= v7 "test@example.com")) :named c15))

(check-sat)
(get-model)
