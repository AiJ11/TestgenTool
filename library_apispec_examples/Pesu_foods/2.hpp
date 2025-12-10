#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

using namespace std;

/**
 * Scenario 2: Complete authentication flow
 * Signup → Login → GetAccount
 * 
 * FIXED v4: Only first API uses _old, subsequent APIs use current state
 */
class Pesu_foods_example2 {
public:
    static void example(vector<unique_ptr<API>> &apis, SymbolTable &root) {
        
        // ==========================================
        // Step 1: SignupOK
        // PRE:  email not_in dom(U_old)  ← Use U_old for FIRST API
        // CALL: signup(name, email, password) ==> 201
        // POST: U'[email] = password
        // ==========================================
        {
            // PRE: email not_in dom(U_old)
            vector<unique_ptr<Expr>> pre_args;
            pre_args.push_back(make_unique<Var>("email"));
            
            vector<unique_ptr<Expr>> dom_args;
            dom_args.push_back(make_unique<Var>("U_old"));
            pre_args.push_back(make_unique<FuncCall>("dom", move(dom_args)));
            
            auto pre = make_unique<FuncCall>("not_in", move(pre_args));
            
            // CALL: signup(name, email, password)
            vector<unique_ptr<Expr>> call_args;
            call_args.push_back(make_unique<Var>("name"));
            call_args.push_back(make_unique<Var>("email"));
            call_args.push_back(make_unique<Var>("password"));
            
            auto call = make_unique<APIcall>(
                make_unique<FuncCall>("signup", move(call_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );
            
            // POST: U'[email] = password
            vector<unique_ptr<Expr>> prime_u_args;
            prime_u_args.push_back(make_unique<Var>("U"));
            auto u_prime = make_unique<FuncCall>("'", move(prime_u_args));
            
            vector<unique_ptr<Expr>> post_args, index_args;
            index_args.push_back(move(u_prime));
            index_args.push_back(make_unique<Var>("email"));
            post_args.push_back(make_unique<FuncCall>("[]", move(index_args)));
            post_args.push_back(make_unique<Var>("password"));
            
            auto post = make_unique<FuncCall>("=", move(post_args));
            
            apis.push_back(make_unique<API>(
                move(pre), 
                move(call),
                Response(HTTPResponseCode::CREATED_201, move(post))
            ));
            
            // Input variables for Step 1
            auto* c1 = new SymbolTable();
            c1->symtable.insert(Var("name"));
            c1->symtable.insert(Var("email"));
            c1->symtable.insert(Var("password"));
            root.children.push_back(c1);
        }
        
        // ==========================================
        // Step 2: LoginOK
        // PRE:  U[email] = password  ← Use U (current state), NOT U_old!
        // CALL: login(email, password) ==> 200
        // POST: T'[token] = email
        // ==========================================
        {
            // PRE: U[email] = password  ← Changed back to U
            vector<unique_ptr<Expr>> pre_args, index_args;
            index_args.push_back(make_unique<Var>("U"));  // ← Use U, not U_old
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
            
            // POST: T'[token] = email
            vector<unique_ptr<Expr>> prime_t_args;
            prime_t_args.push_back(make_unique<Var>("T"));
            auto t_prime = make_unique<FuncCall>("'", move(prime_t_args));
            
            vector<unique_ptr<Expr>> post_args, post_index;
            post_index.push_back(move(t_prime));
            post_index.push_back(make_unique<Var>("token"));
            post_args.push_back(make_unique<FuncCall>("[]", move(post_index)));
            post_args.push_back(make_unique<Var>("email"));
            
            auto post = make_unique<FuncCall>("=", move(post_args));
            
            apis.push_back(make_unique<API>(
                move(pre), 
                move(call),
                Response(HTTPResponseCode::OK_200, move(post))
            ));
            
            // Input variables for Step 2
            auto* c2 = new SymbolTable();
            c2->symtable.insert(Var("email"));
            c2->symtable.insert(Var("password"));
            root.children.push_back(c2);
        }
        
        // ==========================================
        // Step 3: GetAccountOK
        // PRE:  token in dom(T)  ← Use T (current state), NOT T_old!
        // CALL: getAccount(token) ==> 200
        // POST: true
        // ==========================================
        {
            // PRE: token in dom(T)  ← Changed back to T
            vector<unique_ptr<Expr>> pre_args;
            pre_args.push_back(make_unique<Var>("token"));
            
            vector<unique_ptr<Expr>> dom_args;
            dom_args.push_back(make_unique<Var>("T"));  // ← Use T, not T_old
            pre_args.push_back(make_unique<FuncCall>("dom", move(dom_args)));
            
            auto pre = make_unique<FuncCall>("in", move(pre_args));
            
            // CALL: getAccount(token)
            vector<unique_ptr<Expr>> call_args;
            call_args.push_back(make_unique<Var>("token"));
            
            auto call = make_unique<APIcall>(
                make_unique<FuncCall>("getAccount", move(call_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );
            
            // POST: true (no state change)
            vector<unique_ptr<Expr>> true_args;
            auto post = make_unique<FuncCall>("true", move(true_args));
            
            apis.push_back(make_unique<API>(
                move(pre), 
                move(call),
                Response(HTTPResponseCode::OK_200, move(post))
            ));
            
            // Input variables for Step 3
            auto* c3 = new SymbolTable();
            c3->symtable.insert(Var("token"));
            root.children.push_back(c3);
        }
    }
};
