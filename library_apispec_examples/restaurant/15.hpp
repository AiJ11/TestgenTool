#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

/**
 * Restaurant Scenario 15: Agent Login and View Assigned Orders
 * =============================================================
 * 
 * Flow: LoginOK(agentEmail) -> ViewAssignedOrdersOK
 * 
 * PREREQUISITE: Run Scenario 11 first to register the agent!
 */

class restaurant_example15 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {
        
        // ========== API 1: LoginOK(agentEmail) ==========
        {
            // PRE: registered(agentEmail)
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

            auto *c1 = new SymbolTable();
            c1->symtable.insert(Var("agentEmail"));
            c1->symtable.insert(Var("password"));
            root.children.push_back(c1);
        }

        // ========== API 2: ViewAssignedOrdersOK ==========
        {
            // PRE: token_present(agentEmail)
            std::vector<std::unique_ptr<Expr>> token_args2;
            token_args2.push_back(std::make_unique<Var>("agentEmail"));
            auto pre2 = std::make_unique<FuncCall>("token_present", std::move(token_args2));

            // CALL: viewAssignedOrders()
            auto call2 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewAssignedOrders", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: result_is_order_list()
            auto post2 = std::make_unique<FuncCall>("result_is_order_list", std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(std::move(pre2), std::move(call2),
                Response(HTTPResponseCode::OK_200, std::move(post2))));

            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("agentEmail"));
            root.children.push_back(c2);
        }
    }
};
