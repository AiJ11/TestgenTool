#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

/**
 * Restaurant Scenario 13: Delivery Agent Set Unavailable
 * =======================================================
 * 
 * Flow: LoginOK(agentEmail) -> UpdateAvailabilityOK(Unavailable) -> ViewAssignedOrdersOK
 * 
 * PREREQUISITE: Register a delivery_agent user first!
 */

class restaurant_example13 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {
        
        // ========== API 1: LoginOK(agentEmail) ==========
        {
            // PRE: registered(agentEmail) - agent exists in U
            std::vector<std::unique_ptr<Expr>> pre1_args;
            pre1_args.push_back(std::make_unique<Var>("agentEmail"));
            auto pre1 = std::make_unique<FuncCall>("registered", std::move(pre1_args));

            // CALL: login(agentEmail, password)
            std::vector<std::unique_ptr<Expr>> call1_args;
            call1_args.push_back(std::make_unique<Var>("agentEmail"));
            call1_args.push_back(std::make_unique<Var>("password"));
            auto call1 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("login", std::move(call1_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: authenticated(agentEmail)
            std::vector<std::unique_ptr<Expr>> post1_args;
            post1_args.push_back(std::make_unique<Var>("agentEmail"));
            auto post1 = std::make_unique<FuncCall>("authenticated", std::move(post1_args));

            apis.push_back(std::make_unique<API>(std::move(pre1), std::move(call1),
                Response(HTTPResponseCode::OK_200, std::move(post1))));

            // Symbol table for API 1
            auto *c1 = new SymbolTable();
            c1->symtable.insert(Var("agentEmail"));
            c1->symtable.insert(Var("password"));
            root.children.push_back(c1);
        }

        // ========== API 2: UpdateAvailabilityOK(Unavailable) ==========
        {
            // PRE: token_present(agentEmail)
            std::vector<std::unique_ptr<Expr>> token_args2;
            token_args2.push_back(std::make_unique<Var>("agentEmail"));
            auto pre2 = std::make_unique<FuncCall>("token_present", std::move(token_args2));

            // CALL: updateAvailability(availabilityStatus)
            std::vector<std::unique_ptr<Expr>> call2_args;
            call2_args.push_back(std::make_unique<Var>("availabilityStatus"));
            auto call2 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("updateAvailability", std::move(call2_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: availability_updated(agentEmail)
            std::vector<std::unique_ptr<Expr>> post2_args;
            post2_args.push_back(std::make_unique<Var>("agentEmail"));
            auto post2 = std::make_unique<FuncCall>("availability_updated", std::move(post2_args));

            apis.push_back(std::make_unique<API>(std::move(pre2), std::move(call2),
                Response(HTTPResponseCode::OK_200, std::move(post2))));

            // Symbol table for API 2
            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("agentEmail"));
            c2->symtable.insert(Var("availabilityStatus"));
            root.children.push_back(c2);
        }

        // ========== API 3: ViewAssignedOrdersOK ==========
        {
            // PRE: token_present(agentEmail)
            std::vector<std::unique_ptr<Expr>> token_args3;
            token_args3.push_back(std::make_unique<Var>("agentEmail"));
            auto pre3 = std::make_unique<FuncCall>("token_present", std::move(token_args3));

            // CALL: viewAssignedOrders()
            auto call3 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewAssignedOrders", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: result_is_order_list()
            auto post3 = std::make_unique<FuncCall>("result_is_order_list", std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(std::move(pre3), std::move(call3),
                Response(HTTPResponseCode::OK_200, std::move(post3))));

            // Symbol table for API 3
            auto *c3 = new SymbolTable();
            c3->symtable.insert(Var("agentEmail"));
            root.children.push_back(c3);
        }
    }
};
