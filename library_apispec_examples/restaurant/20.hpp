#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

// INFEASIBLE Test: Double Registration expecting both to succeed
// This SHOULD return UNSAT because you can't register same email twice with 201

class restaurant_example20 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {

        // =========================
        // 1. RegisterUserOK - First registration succeeds
        // PRE:  not_registered(email)
        // CALL: registerUser(fullName, email, password, mobile, role) => 201
        // POST: registered(email)
        // =========================
        {
            // PRE: not_registered(email)
            std::vector<std::unique_ptr<Expr>> pre1_args;
            pre1_args.push_back(std::make_unique<Var>("email"));
            auto pre1 = std::make_unique<FuncCall>("not_registered", std::move(pre1_args));

            // CALL: registerUser(fullName, email, password, mobile, role)
            std::vector<std::unique_ptr<Expr>> call1_args;
            call1_args.push_back(std::make_unique<Var>("fullName"));
            call1_args.push_back(std::make_unique<Var>("email"));
            call1_args.push_back(std::make_unique<Var>("password"));
            call1_args.push_back(std::make_unique<Var>("mobile"));
            call1_args.push_back(std::make_unique<Var>("role"));

            auto call1 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("registerUser", std::move(call1_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            // POST: registered(email)
            std::vector<std::unique_ptr<Expr>> post1_args;
            post1_args.push_back(std::make_unique<Var>("email"));
            auto post1 = std::make_unique<FuncCall>("registered", std::move(post1_args));

            apis.push_back(std::make_unique<API>(
                std::move(pre1),
                std::move(call1),
                Response(HTTPResponseCode::CREATED_201, std::move(post1))
            ));

            // Symbol table for API 1
            auto *c1 = new SymbolTable();
            c1->symtable.insert(Var("fullName"));
            c1->symtable.insert(Var("email"));
            c1->symtable.insert(Var("password"));
            c1->symtable.insert(Var("mobile"));
            c1->symtable.insert(Var("role"));
            root.children.push_back(c1);
        }

        // =========================
        // 2. RegisterUserOK - Second registration ALSO expects 201 (INFEASIBLE!)
        // PRE:  not_registered(email)  <-- IMPOSSIBLE after API 1!
        // CALL: registerUser(fullName, email, password, mobile, role) => 201
        // POST: registered(email)
        // =========================
        {
            // PRE: not_registered(email) - BUT user IS registered after API 1!
            std::vector<std::unique_ptr<Expr>> pre2_args;
            pre2_args.push_back(std::make_unique<Var>("email"));
            auto pre2 = std::make_unique<FuncCall>("not_registered", std::move(pre2_args));

            // CALL: registerUser - same email, expecting 201 again (IMPOSSIBLE!)
            std::vector<std::unique_ptr<Expr>> call2_args;
            call2_args.push_back(std::make_unique<Var>("fullName"));
            call2_args.push_back(std::make_unique<Var>("email"));
            call2_args.push_back(std::make_unique<Var>("password"));
            call2_args.push_back(std::make_unique<Var>("mobile"));
            call2_args.push_back(std::make_unique<Var>("role"));

            auto call2 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("registerUser", std::move(call2_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)  // Expects 201 - WRONG!
            );

            // POST: registered(email)
            std::vector<std::unique_ptr<Expr>> post2_args;
            post2_args.push_back(std::make_unique<Var>("email"));
            auto post2 = std::make_unique<FuncCall>("registered", std::move(post2_args));

            apis.push_back(std::make_unique<API>(
                std::move(pre2),
                std::move(call2),
                Response(HTTPResponseCode::CREATED_201, std::move(post2))
            ));

            // Symbol table for API 2
            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("fullName"));
            c2->symtable.insert(Var("email"));
            c2->symtable.insert(Var("password"));
            c2->symtable.insert(Var("mobile"));
            c2->symtable.insert(Var("role"));
            root.children.push_back(c2);
        }
    }
};
