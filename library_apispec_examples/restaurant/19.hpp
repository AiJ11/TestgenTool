#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

/**
 * Restaurant Scenario 19: Full Order Flow - Customer to Agent Delivery
 * =====================================================================
 * 
 * Flow: LoginOK(customerEmail) -> PlaceOrderOK -> OwnerLoginOK(ownerEmail) ->
 *       AssignOrderOK(Order, Agent) -> AgentLoginOK(agentEmail) -> UpdateOrderStatusOK(Order, Delivered)
 * 
 * PREREQUISITE: Run Scenario 1 (customer), Scenario 6 (owner), Scenario 11 (agent) first!
 * Also need cart items - run full customer flow or have items in cart.
 */

class restaurant_example19 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {
        
        // ========== API 1: LoginOK(customerEmail) ==========
        {
            // PRE: registered(customerEmail)
            std::vector<std::unique_ptr<Expr>> pre1_args;
            pre1_args.push_back(std::make_unique<Var>("customerEmail"));
            auto pre1 = std::make_unique<FuncCall>("registered", std::move(pre1_args));

            // CALL: login(customerEmail, password)
            std::vector<std::unique_ptr<Expr>> call1_args;
            call1_args.push_back(std::make_unique<Var>("customerEmail"));
            call1_args.push_back(std::make_unique<Var>("password"));
            auto call1 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("login", std::move(call1_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: authenticated(customerEmail)
            std::vector<std::unique_ptr<Expr>> post1_args;
            post1_args.push_back(std::make_unique<Var>("customerEmail"));
            auto post1 = std::make_unique<FuncCall>("authenticated", std::move(post1_args));

            apis.push_back(std::make_unique<API>(std::move(pre1), std::move(call1),
                Response(HTTPResponseCode::OK_200, std::move(post1))));

            auto *c1 = new SymbolTable();
            c1->symtable.insert(Var("customerEmail"));
            c1->symtable.insert(Var("password"));
            root.children.push_back(c1);
        }

        // ========== API 2: PlaceOrderOK ==========
        {
            // PRE: token_present(customerEmail)
            std::vector<std::unique_ptr<Expr>> token_args2;
            token_args2.push_back(std::make_unique<Var>("customerEmail"));
            auto pre2 = std::make_unique<FuncCall>("token_present", std::move(token_args2));

            // CALL: placeOrder() - no parameters
            auto call2 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("placeOrder", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            // POST: order_recorded(customerEmail)
            std::vector<std::unique_ptr<Expr>> post2_args;
            post2_args.push_back(std::make_unique<Var>("customerEmail"));
            auto post2 = std::make_unique<FuncCall>("order_recorded", std::move(post2_args));

            apis.push_back(std::make_unique<API>(std::move(pre2), std::move(call2),
                Response(HTTPResponseCode::CREATED_201, std::move(post2))));

            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("customerEmail"));
            root.children.push_back(c2);
        }

        // ========== API 3: OwnerLoginOK(ownerEmail) ==========
        {
            // PRE: registered(ownerEmail)
            std::vector<std::unique_ptr<Expr>> pre3_args;
            pre3_args.push_back(std::make_unique<Var>("ownerEmail"));
            auto pre3 = std::make_unique<FuncCall>("registered", std::move(pre3_args));

            // CALL: login(ownerEmail, password)
            std::vector<std::unique_ptr<Expr>> call3_args;
            call3_args.push_back(std::make_unique<Var>("ownerEmail"));
            call3_args.push_back(std::make_unique<Var>("password"));
            auto call3 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("login", std::move(call3_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: authenticated(ownerEmail)
            std::vector<std::unique_ptr<Expr>> post3_args;
            post3_args.push_back(std::make_unique<Var>("ownerEmail"));
            auto post3 = std::make_unique<FuncCall>("authenticated", std::move(post3_args));

            apis.push_back(std::make_unique<API>(std::move(pre3), std::move(call3),
                Response(HTTPResponseCode::OK_200, std::move(post3))));

            auto *c3 = new SymbolTable();
            c3->symtable.insert(Var("ownerEmail"));
            c3->symtable.insert(Var("password"));
            root.children.push_back(c3);
        }

        // ========== API 4: AssignOrderOK(Order, Agent) ==========
        {
            // PRE: token_present(ownerEmail)
            std::vector<std::unique_ptr<Expr>> token_args4;
            token_args4.push_back(std::make_unique<Var>("ownerEmail"));
            auto pre4 = std::make_unique<FuncCall>("token_present", std::move(token_args4));

            // CALL: assignOrder(orderId, agentId)
            std::vector<std::unique_ptr<Expr>> call4_args;
            call4_args.push_back(std::make_unique<Var>("orderId"));
            call4_args.push_back(std::make_unique<Var>("agentId"));
            auto call4 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("assignOrder", std::move(call4_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: order_assigned(orderId, agentId)
            std::vector<std::unique_ptr<Expr>> post4_args;
            post4_args.push_back(std::make_unique<Var>("orderId"));
            post4_args.push_back(std::make_unique<Var>("agentId"));
            auto post4 = std::make_unique<FuncCall>("order_assigned", std::move(post4_args));

            apis.push_back(std::make_unique<API>(std::move(pre4), std::move(call4),
                Response(HTTPResponseCode::OK_200, std::move(post4))));

            auto *c4 = new SymbolTable();
            c4->symtable.insert(Var("ownerEmail"));
            c4->symtable.insert(Var("orderId"));
            c4->symtable.insert(Var("agentId"));
            root.children.push_back(c4);
        }

        // ========== API 5: AgentLoginOK(agentEmail) ==========
        {
            // PRE: registered(agentEmail)
            std::vector<std::unique_ptr<Expr>> pre5_args;
            pre5_args.push_back(std::make_unique<Var>("agentEmail"));
            auto pre5 = std::make_unique<FuncCall>("registered", std::move(pre5_args));

            // CALL: login(agentEmail, password)
            std::vector<std::unique_ptr<Expr>> call5_args;
            call5_args.push_back(std::make_unique<Var>("agentEmail"));
            call5_args.push_back(std::make_unique<Var>("password"));
            auto call5 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("login", std::move(call5_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: authenticated(agentEmail)
            std::vector<std::unique_ptr<Expr>> post5_args;
            post5_args.push_back(std::make_unique<Var>("agentEmail"));
            auto post5 = std::make_unique<FuncCall>("authenticated", std::move(post5_args));

            apis.push_back(std::make_unique<API>(std::move(pre5), std::move(call5),
                Response(HTTPResponseCode::OK_200, std::move(post5))));

            auto *c5 = new SymbolTable();
            c5->symtable.insert(Var("agentEmail"));
            c5->symtable.insert(Var("password"));
            root.children.push_back(c5);
        }

        // ========== API 6: UpdateOrderStatusOK(Order, Delivered) ==========
        {
            // PRE: token_present(agentEmail)
            std::vector<std::unique_ptr<Expr>> token_args6;
            token_args6.push_back(std::make_unique<Var>("agentEmail"));
            auto pre6 = std::make_unique<FuncCall>("token_present", std::move(token_args6));

            // CALL: updateOrderStatus(orderId, deliveredStatus)
            std::vector<std::unique_ptr<Expr>> call6_args;
            call6_args.push_back(std::make_unique<Var>("orderId"));
            call6_args.push_back(std::make_unique<Var>("deliveredStatus"));
            auto call6 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("updateOrderStatus", std::move(call6_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: order_status_updated(orderId, deliveredStatus)
            std::vector<std::unique_ptr<Expr>> post6_args;
            post6_args.push_back(std::make_unique<Var>("orderId"));
            post6_args.push_back(std::make_unique<Var>("deliveredStatus"));
            auto post6 = std::make_unique<FuncCall>("order_status_updated", std::move(post6_args));

            apis.push_back(std::make_unique<API>(std::move(pre6), std::move(call6),
                Response(HTTPResponseCode::OK_200, std::move(post6))));

            auto *c6 = new SymbolTable();
            c6->symtable.insert(Var("agentEmail"));
            c6->symtable.insert(Var("orderId"));
            c6->symtable.insert(Var("deliveredStatus"));
            root.children.push_back(c6);
        }
    }
};
