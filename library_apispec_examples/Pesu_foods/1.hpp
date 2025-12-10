/*#include <iostream>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

using namespace std;

class Pesu_foods_example1 {
public:
    static void example(vector<unique_ptr<API>> &apis, SymbolTable &root) {
        // -------- Step 1: RegisterUserOK --------
        vector<unique_ptr<Expr>> pre1_args;
        pre1_args.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> dom_args;
        dom_args.push_back(make_unique<Var>("U"));
        pre1_args.push_back(make_unique<FuncCall>("dom", move(dom_args)));
        auto pre1 = make_unique<FuncCall>("not_in", move(pre1_args));

        vector<unique_ptr<Expr>> args1;
        args1.push_back(make_unique<Var>("username"));
        args1.push_back(make_unique<Var>("password"));
        auto call1 = make_unique<APIcall>(make_unique<FuncCall>("register_user", move(args1)),
                                          Response(HTTPResponseCode::CREATED_201, nullptr));

        vector<unique_ptr<Expr>> post1_args, index_args;
        index_args.push_back(make_unique<Var>("U"));
        index_args.push_back(make_unique<Var>("username"));
        post1_args.push_back(make_unique<FuncCall>("[]", move(index_args)));
        post1_args.push_back(make_unique<Var>("password"));
        auto post1 = make_unique<FuncCall>("=", move(post1_args));

        apis.push_back(make_unique<API>(move(pre1), move(call1), Response(HTTPResponseCode::CREATED_201, move(post1))));

        // -------- Step 2: AuthenticateOK --------
        vector<unique_ptr<Expr>> pre2_args, index2_args;
        index2_args.push_back(make_unique<Var>("U"));
        index2_args.push_back(make_unique<Var>("username"));
        pre2_args.push_back(make_unique<FuncCall>("[]", move(index2_args)));
        pre2_args.push_back(make_unique<Var>("password"));
        auto pre2 = make_unique<FuncCall>("=", move(pre2_args));

        vector<unique_ptr<Expr>> args2;
        args2.push_back(make_unique<Var>("username"));
        args2.push_back(make_unique<Var>("password"));
        auto call2 = make_unique<APIcall>(make_unique<FuncCall>("authenticate", move(args2)),
                                          Response(HTTPResponseCode::OK_200, nullptr));

        vector<unique_ptr<Expr>> post2_args, post2_index;
        post2_index.push_back(make_unique<Var>("token"));
        post2_index.push_back(make_unique<Var>("username"));
        post2_args.push_back(make_unique<FuncCall>("[]", move(post2_index)));
        post2_args.push_back(make_unique<Var>("tokenVal"));
        auto post2 = make_unique<FuncCall>("=", move(post2_args));

        apis.push_back(make_unique<API>(move(pre2), move(call2), Response(HTTPResponseCode::OK_200, move(post2))));

        // -------- Step 3: AddToCartOK(ItemFromCanteenX) --------
        vector<unique_ptr<Expr>> pre3_args;
        pre3_args.push_back(make_unique<Var>("ItemFromCanteenX"));
        pre3_args.push_back(make_unique<Var>("MenuList"));
        auto pre3 = make_unique<FuncCall>("in", move(pre3_args));

        vector<unique_ptr<Expr>> args3;
        args3.push_back(make_unique<Var>("ItemFromCanteenX"));
        auto call3 = make_unique<APIcall>(make_unique<FuncCall>("add_to_cart", move(args3)),
                                          Response(HTTPResponseCode::OK_200, nullptr));

        vector<unique_ptr<Expr>> post3_args;
        post3_args.push_back(make_unique<Var>("cart"));
        post3_args.push_back(make_unique<Var>("ItemFromCanteenX"));
        auto post3 = make_unique<FuncCall>("add_to_set", move(post3_args));

        apis.push_back(make_unique<API>(move(pre3), move(call3), Response(HTTPResponseCode::OK_200, move(post3))));

        // -------- Step 4: PlaceOrderOK(Order1) --------
        vector<unique_ptr<Expr>> pre4_args;
        pre4_args.push_back(make_unique<Var>("cart"));
        auto pre4 = make_unique<FuncCall>("not_empty", move(pre4_args));

        vector<unique_ptr<Expr>> args4;
        args4.push_back(make_unique<Var>("Order1"));
        auto call4 = make_unique<APIcall>(make_unique<FuncCall>("place_order", move(args4)),
                                          Response(HTTPResponseCode::CREATED_201, nullptr));

        vector<unique_ptr<Expr>> post4_args, post4_index;
        post4_index.push_back(make_unique<Var>("orders"));
        post4_index.push_back(make_unique<Var>("Order1"));
        post4_args.push_back(make_unique<FuncCall>("[]", move(post4_index)));
        post4_args.push_back(make_unique<Var>("cart"));
        auto post4 = make_unique<FuncCall>("=", move(post4_args));

        apis.push_back(make_unique<API>(move(pre4), move(call4), Response(HTTPResponseCode::CREATED_201, move(post4))));

        // -------- Symbol Table --------
        SymbolTable *sym2 = new SymbolTable();
        sym2->symtable.insert(Var("username"));
        sym2->symtable.insert(Var("password"));
        sym2->symtable.insert(Var("tokenVal"));
        sym2->symtable.insert(Var("ItemFromCanteenX"));
        sym2->symtable.insert(Var("MenuList"));
        sym2->symtable.insert(Var("Order1"));
        root.children.push_back(sym2);
    }
};*/

/*#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

using namespace std;

class Pesu_foods_example1 {
public:
    static void example(vector<unique_ptr<API>> &apis, SymbolTable &root) {
        // ================================
        // Step 1: RegisterUserOK
        // PRE: username not_in dom(U)
        // CALL: register_user(username, password) ==> 201
        // POST: U[username] = password
        // ================================
        {
            // PRE
            vector<unique_ptr<Expr>> pre1_args;
            pre1_args.push_back(make_unique<Var>("username"));
            vector<unique_ptr<Expr>> dom_args;
            dom_args.push_back(make_unique<Var>("U"));
            pre1_args.push_back(make_unique<FuncCall>("dom", move(dom_args)));
            auto pre1 = make_unique<FuncCall>("not_in", move(pre1_args));

            // CALL
            vector<unique_ptr<Expr>> args1;
            args1.push_back(make_unique<Var>("username"));
            args1.push_back(make_unique<Var>("password"));
            auto call1 = make_unique<APIcall>(
                make_unique<FuncCall>("register_user", move(args1)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            // POST: [] (U, username) = password
            vector<unique_ptr<Expr>> post1_args, index_args;
            index_args.push_back(make_unique<Var>("U"));
            index_args.push_back(make_unique<Var>("username"));
            post1_args.push_back(make_unique<FuncCall>("[]", move(index_args)));
            post1_args.push_back(make_unique<Var>("password"));
            auto post1 = make_unique<FuncCall>("=", move(post1_args));

            apis.push_back(make_unique<API>(move(pre1), move(call1),
                       Response(HTTPResponseCode::CREATED_201, move(post1))));

            // Per-API symbol table (inputs for Step 1)
            auto* c1 = new SymbolTable();
            c1->symtable.insert(Var("username"));
            c1->symtable.insert(Var("password"));
            root.children.push_back(c1);
        }

        // ================================
        // Step 2: AuthenticateOK
        // PRE: U[username] = password
        // CALL: authenticate(username, password) ==> 200
        // POST: T[tokenVal] = username   (FIXED)
        // ================================
        {
            // PRE
            vector<unique_ptr<Expr>> pre2_args, index2_args;
            index2_args.push_back(make_unique<Var>("U"));
            index2_args.push_back(make_unique<Var>("username"));
            pre2_args.push_back(make_unique<FuncCall>("[]", move(index2_args)));
            pre2_args.push_back(make_unique<Var>("password"));
            auto pre2 = make_unique<FuncCall>("=", move(pre2_args));

            // CALL
            vector<unique_ptr<Expr>> args2;
            args2.push_back(make_unique<Var>("username"));
            args2.push_back(make_unique<Var>("password"));
            auto call2 = make_unique<APIcall>(
                make_unique<FuncCall>("authenticate", move(args2)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: [] (T, tokenVal) = username   // << corrected map/key/order
            vector<unique_ptr<Expr>> post2_args, post2_index;
            post2_index.push_back(make_unique<Var>("T"));
            post2_index.push_back(make_unique<Var>("tokenVal"));
            post2_args.push_back(make_unique<FuncCall>("[]", move(post2_index)));
            post2_args.push_back(make_unique<Var>("username"));
            auto post2 = make_unique<FuncCall>("=", move(post2_args));

            apis.push_back(make_unique<API>(move(pre2), move(call2),
                       Response(HTTPResponseCode::OK_200, move(post2))));

            // Per-API symbol table (inputs for Step 2)
            auto* c2 = new SymbolTable();
            c2->symtable.insert(Var("username"));
            c2->symtable.insert(Var("password"));
            // tokenVal is produced by server normally; keep it out of inputs
            root.children.push_back(c2);
        }

        // ================================
        // Step 3: AddToCartOK(ItemFromCanteenX)
        // PRE: ItemFromCanteenX in MenuList
        // CALL: add_to_cart(ItemFromCanteenX) ==> 200
        // POST: add_to_set(cart, ItemFromCanteenX)   (kept as-is for TestGen)
        // ================================
        {
            // PRE
            vector<unique_ptr<Expr>> pre3_args;
            pre3_args.push_back(make_unique<Var>("ItemFromCanteenX"));
            pre3_args.push_back(make_unique<Var>("MenuList"));
            auto pre3 = make_unique<FuncCall>("in", move(pre3_args));

            // CALL
            vector<unique_ptr<Expr>> args3;
            args3.push_back(make_unique<Var>("ItemFromCanteenX"));
            auto call3 = make_unique<APIcall>(
                make_unique<FuncCall>("add_to_cart", move(args3)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST
            vector<unique_ptr<Expr>> post3_args;
            post3_args.push_back(make_unique<Var>("cart"));
            post3_args.push_back(make_unique<Var>("ItemFromCanteenX"));
            auto post3 = make_unique<FuncCall>("add_to_set", move(post3_args));

            apis.push_back(make_unique<API>(move(pre3), move(call3),
                       Response(HTTPResponseCode::OK_200, move(post3))));

            // Per-API symbol table (inputs for Step 3)
            auto* c3 = new SymbolTable();
            c3->symtable.insert(Var("ItemFromCanteenX"));
            // MenuList/cart are state containers; not inputs
            root.children.push_back(c3);
        }

        // ================================
        // Step 4: PlaceOrderOK(Order1)
        // PRE: not_empty(cart)
        // CALL: place_order(Order1) ==> 201
        // POST: orders[Order1] = cart
        // ================================
        {
            // PRE
            vector<unique_ptr<Expr>> pre4_args;
            pre4_args.push_back(make_unique<Var>("cart"));
            auto pre4 = make_unique<FuncCall>("not_empty", move(pre4_args));

            // CALL
            vector<unique_ptr<Expr>> args4;
            args4.push_back(make_unique<Var>("Order1"));
            auto call4 = make_unique<APIcall>(
                make_unique<FuncCall>("place_order", move(args4)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            // POST
            vector<unique_ptr<Expr>> post4_args, post4_index;
            post4_index.push_back(make_unique<Var>("orders"));
            post4_index.push_back(make_unique<Var>("Order1"));
            post4_args.push_back(make_unique<FuncCall>("[]", move(post4_index)));
            post4_args.push_back(make_unique<Var>("cart"));
            auto post4 = make_unique<FuncCall>("=", move(post4_args));

            apis.push_back(make_unique<API>(move(pre4), move(call4),
                       Response(HTTPResponseCode::CREATED_201, move(post4))));

            // Per-API symbol table (inputs for Step 4)
            auto* c4 = new SymbolTable();
            c4->symtable.insert(Var("Order1"));
            root.children.push_back(c4);
        }
    }
};*/
#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

using namespace std;

/**
 * Scenario 1: Login without signup
 * This should be UNSAT (infeasible) because we can't login 
 * with credentials that don't exist in U
 */
class Pesu_foods_example1 {
public:
    static void example(vector<unique_ptr<API>> &apis, SymbolTable &root) {
        // ================================
        // LoginOK (but user doesn't exist!)
        // PRE: U[email] = password  ← User MUST exist (but U is empty!)
        // CALL: login(email, password) ==> 200
        // POST: T[token] = email
        // ================================
        {
            // PRE: U[email] = password
            vector<unique_ptr<Expr>> pre_args, index_args;
            index_args.push_back(make_unique<Var>("U"));
            index_args.push_back(make_unique<Var>("email"));
            pre_args.push_back(make_unique<FuncCall>("[]", move(index_args)));
            pre_args.push_back(make_unique<Var>("password"));
            auto pre = make_unique<FuncCall>("=", move(pre_args));

            // CALL: login(email, password)
            vector<unique_ptr<Expr>> call_args;
            call_args.push_back(make_unique<Var>("email"));
            call_args.push_back(make_unique<Var>("password"));
            auto call = make_unique<APIcall>(
                make_unique<FuncCall>("login", move(call_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: T[token] = email
            vector<unique_ptr<Expr>> post_args, post_index;
            post_index.push_back(make_unique<Var>("T"));
            post_index.push_back(make_unique<Var>("token"));
            post_args.push_back(make_unique<FuncCall>("[]", move(post_index)));
            post_args.push_back(make_unique<Var>("email"));
            auto post = make_unique<FuncCall>("=", move(post_args));

            apis.push_back(make_unique<API>(
                move(pre), 
                move(call),
                Response(HTTPResponseCode::OK_200, move(post))
            ));

            // Input variables
            auto* c1 = new SymbolTable();
            c1->symtable.insert(Var("email"));
            c1->symtable.insert(Var("password"));
            root.children.push_back(c1);
        }
    }
};

