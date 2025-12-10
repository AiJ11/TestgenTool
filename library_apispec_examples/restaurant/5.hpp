#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

// Test string 5:
// LoginOK(email), ViewCartOK, PlaceOrderOK(Order4), ViewOrdersOK

class restaurant_example5 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {
        
        // ========== API 1: LoginOK ==========
        {
            std::vector<std::unique_ptr<Expr>> pre1_args;
            pre1_args.push_back(std::make_unique<Var>("email"));
            auto pre1 = std::make_unique<FuncCall>("in", std::move(pre1_args));

            std::vector<std::unique_ptr<Expr>> call1_args;
            call1_args.push_back(std::make_unique<Var>("email"));
            call1_args.push_back(std::make_unique<Var>("password"));
            auto call1 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("login", std::move(call1_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post1_args;
            post1_args.push_back(std::make_unique<Var>("email"));
            auto post1 = std::make_unique<FuncCall>("authenticated", std::move(post1_args));

            apis.push_back(std::make_unique<API>(std::move(pre1), std::move(call1),
                Response(HTTPResponseCode::OK_200, std::move(post1))));

            // Symbol table for API 1
            auto *c1 = new SymbolTable();
            c1->symtable.insert(Var("email"));
            c1->symtable.insert(Var("password"));
            root.children.push_back(c1);
        }

        // ========== API 2: ViewCartOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args2;
            token_args2.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre2_args;
            pre2_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args2)));
            auto pre2 = std::make_unique<FuncCall>("and", std::move(pre2_args));

            auto call2 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewCart", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post2 = std::make_unique<FuncCall>("result_is_cart", std::vector<std::unique_ptr<Expr>>{});
            
            apis.push_back(std::make_unique<API>(std::move(pre2), std::move(call2),
                Response(HTTPResponseCode::OK_200, std::move(post2))));

            // Symbol table for API 2
            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("email"));
            root.children.push_back(c2);
        }

        // ========== API 3: PlaceOrderOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args3;
            token_args3.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre3_args;
            pre3_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args3)));
            auto pre3 = std::make_unique<FuncCall>("and", std::move(pre3_args));

            // ✅ FIXED: 0 parameters
            std::vector<std::unique_ptr<Expr>> call3_args;
            // Empty - no parameters!
            auto call3 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("placeOrder", std::move(call3_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post3_args;
            post3_args.push_back(std::make_unique<Var>("Order4"));
            post3_args.push_back(std::make_unique<Var>("email"));
            auto post3 = std::make_unique<FuncCall>("order_recorded", std::move(post3_args));

            apis.push_back(std::make_unique<API>(std::move(pre3), std::move(call3),
                Response(HTTPResponseCode::CREATED_201, std::move(post3))));

            // Symbol table for API 3
            auto *c3 = new SymbolTable();
            c3->symtable.insert(Var("email"));
            // Order4 is NOT in symbol table - it's symbolic in postcondition
            root.children.push_back(c3);
        }

        // ========== API 4: ViewOrdersOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args4;
            token_args4.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre4_args;
            pre4_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args4)));
            auto pre4 = std::make_unique<FuncCall>("and", std::move(pre4_args));

            auto call4 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewOrders", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post4 = std::make_unique<FuncCall>("result_is_order_list", std::vector<std::unique_ptr<Expr>>{});
            
            apis.push_back(std::make_unique<API>(std::move(pre4), std::move(call4),
                Response(HTTPResponseCode::OK_200, std::move(post4))));

            // Symbol table for API 4
            auto *c4 = new SymbolTable();
            c4->symtable.insert(Var("email"));
            root.children.push_back(c4);
        }
    }
};
