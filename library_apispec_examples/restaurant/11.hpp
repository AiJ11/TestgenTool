#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

/**
 * Restaurant Scenario 11: Delivery Agent Full Flow
 * =================================================
 * 
 * Flow: RegisterAgent -> LoginOK(agentEmail) -> UpdateAvailabilityOK(Available) 
 *       -> ViewAssignedOrdersOK -> UpdateOrderStatusOK(Order1, PickedUp) 
 *       -> UpdateOrderStatusOK(Order1, Delivered)
 * 
 * This scenario registers a delivery agent, then performs agent operations.
 * Run this BEFORE scenarios 12 and 13!
 */

class restaurant_example11 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {
        
        // ========== API 1: RegisterAgent ==========
        // Register a user with role = "delivery_agent"
        {
            // PRE: not_in(agentEmail, dom(U_old)) - agent not yet registered
            std::vector<std::unique_ptr<Expr>> pre1_args;
            pre1_args.push_back(std::make_unique<Var>("agentEmail"));
            std::vector<std::unique_ptr<Expr>> dom_args1;
            dom_args1.push_back(std::make_unique<Var>("U_old"));
            pre1_args.push_back(std::make_unique<FuncCall>("dom", std::move(dom_args1)));
            auto pre1 = std::make_unique<FuncCall>("not_in", std::move(pre1_args));

            // CALL: registerUser(agentName, agentEmail, password, agentMobile, agentRole)
            std::vector<std::unique_ptr<Expr>> call1_args;
            call1_args.push_back(std::make_unique<Var>("agentName"));
            call1_args.push_back(std::make_unique<Var>("agentEmail"));
            call1_args.push_back(std::make_unique<Var>("password"));
            call1_args.push_back(std::make_unique<Var>("agentMobile"));
            call1_args.push_back(std::make_unique<Var>("agentRole"));
            auto call1 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("registerUser", std::move(call1_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            // POST: registered(agentEmail)
            std::vector<std::unique_ptr<Expr>> post1_args;
            post1_args.push_back(std::make_unique<Var>("agentEmail"));
            auto post1 = std::make_unique<FuncCall>("registered", std::move(post1_args));

            apis.push_back(std::make_unique<API>(std::move(pre1), std::move(call1),
                Response(HTTPResponseCode::CREATED_201, std::move(post1))));

            // Symbol table for API 1
            auto *c1 = new SymbolTable();
            c1->symtable.insert(Var("agentName"));
            c1->symtable.insert(Var("agentEmail"));
            c1->symtable.insert(Var("password"));
            c1->symtable.insert(Var("agentMobile"));
            c1->symtable.insert(Var("agentRole"));
            root.children.push_back(c1);
        }

        // ========== API 2: LoginOK(agentEmail) ==========
        {
            // PRE: registered(agentEmail) - agent exists in U
            std::vector<std::unique_ptr<Expr>> pre2_args;
            pre2_args.push_back(std::make_unique<Var>("agentEmail"));
            auto pre2 = std::make_unique<FuncCall>("registered", std::move(pre2_args));

            // CALL: login(agentEmail, password)
            std::vector<std::unique_ptr<Expr>> call2_args;
            call2_args.push_back(std::make_unique<Var>("agentEmail"));
            call2_args.push_back(std::make_unique<Var>("password"));
            auto call2 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("login", std::move(call2_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: authenticated(agentEmail)
            std::vector<std::unique_ptr<Expr>> post2_args;
            post2_args.push_back(std::make_unique<Var>("agentEmail"));
            auto post2 = std::make_unique<FuncCall>("authenticated", std::move(post2_args));

            apis.push_back(std::make_unique<API>(std::move(pre2), std::move(call2),
                Response(HTTPResponseCode::OK_200, std::move(post2))));

            // Symbol table for API 2
            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("agentEmail"));
            c2->symtable.insert(Var("password"));
            root.children.push_back(c2);
        }

        // ========== API 3: UpdateAvailabilityOK(Available) ==========
        {
            // PRE: token_present(agentEmail)
            std::vector<std::unique_ptr<Expr>> token_args3;
            token_args3.push_back(std::make_unique<Var>("agentEmail"));
            auto pre3 = std::make_unique<FuncCall>("token_present", std::move(token_args3));

            // CALL: updateAvailability(availabilityStatus)
            std::vector<std::unique_ptr<Expr>> call3_args;
            call3_args.push_back(std::make_unique<Var>("availabilityStatus"));
            auto call3 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("updateAvailability", std::move(call3_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: availability_updated(agentEmail)
            std::vector<std::unique_ptr<Expr>> post3_args;
            post3_args.push_back(std::make_unique<Var>("agentEmail"));
            auto post3 = std::make_unique<FuncCall>("availability_updated", std::move(post3_args));

            apis.push_back(std::make_unique<API>(std::move(pre3), std::move(call3),
                Response(HTTPResponseCode::OK_200, std::move(post3))));

            // Symbol table for API 3
            auto *c3 = new SymbolTable();
            c3->symtable.insert(Var("agentEmail"));
            c3->symtable.insert(Var("availabilityStatus"));
            root.children.push_back(c3);
        }

        // ========== API 4: ViewAssignedOrdersOK ==========
        {
            // PRE: token_present(agentEmail)
            std::vector<std::unique_ptr<Expr>> token_args4;
            token_args4.push_back(std::make_unique<Var>("agentEmail"));
            auto pre4 = std::make_unique<FuncCall>("token_present", std::move(token_args4));

            // CALL: viewAssignedOrders()
            auto call4 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewAssignedOrders", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: result_is_order_list()
            auto post4 = std::make_unique<FuncCall>("result_is_order_list", std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(std::move(pre4), std::move(call4),
                Response(HTTPResponseCode::OK_200, std::move(post4))));

            // Symbol table for API 4
            auto *c4 = new SymbolTable();
            c4->symtable.insert(Var("agentEmail"));
            root.children.push_back(c4);
        }

        // ========== API 5: UpdateOrderStatusOK(Order1, PickedUp) ==========
        {
            // PRE: token_present(agentEmail)
            std::vector<std::unique_ptr<Expr>> token_args5;
            token_args5.push_back(std::make_unique<Var>("agentEmail"));
            auto pre5 = std::make_unique<FuncCall>("token_present", std::move(token_args5));

            // CALL: updateOrderStatus(orderId, orderStatus)
            std::vector<std::unique_ptr<Expr>> call5_args;
            call5_args.push_back(std::make_unique<Var>("orderId"));
            call5_args.push_back(std::make_unique<Var>("pickedUpStatus"));
            auto call5 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("updateOrderStatus", std::move(call5_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: order_status_updated(orderId, pickedUpStatus)
            std::vector<std::unique_ptr<Expr>> post5_args;
            post5_args.push_back(std::make_unique<Var>("orderId"));
            post5_args.push_back(std::make_unique<Var>("pickedUpStatus"));
            auto post5 = std::make_unique<FuncCall>("order_status_updated", std::move(post5_args));

            apis.push_back(std::make_unique<API>(std::move(pre5), std::move(call5),
                Response(HTTPResponseCode::OK_200, std::move(post5))));

            // Symbol table for API 5
            auto *c5 = new SymbolTable();
            c5->symtable.insert(Var("agentEmail"));
            c5->symtable.insert(Var("orderId"));
            c5->symtable.insert(Var("pickedUpStatus"));
            root.children.push_back(c5);
        }

        // ========== API 6: UpdateOrderStatusOK(Order1, Delivered) ==========
        {
            // PRE: token_present(agentEmail)
            std::vector<std::unique_ptr<Expr>> token_args6;
            token_args6.push_back(std::make_unique<Var>("agentEmail"));
            auto pre6 = std::make_unique<FuncCall>("token_present", std::move(token_args6));

            // CALL: updateOrderStatus(orderId, orderStatus)
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

            // Symbol table for API 6
            auto *c6 = new SymbolTable();
            c6->symtable.insert(Var("agentEmail"));
            c6->symtable.insert(Var("orderId"));
            c6->symtable.insert(Var("deliveredStatus"));
            root.children.push_back(c6);
        }
    }
};
