#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

/**
 * Restaurant Scenario 16: Customer Places Order, Owner Assigns to Agent
 * ======================================================================
 * 
 * Flow: RegisterUserOK(email) -> LoginOK(email) -> BrowseRestaurantsOK -> 
 *       ViewMenuOK(RestaurantX) -> AddToCartOK(ItemX) -> PlaceOrderOK -> ViewOrdersOK ->
 *       OwnerLogin(ownerEmail) -> ViewOrdersOK -> AssignOrderOK(Order, Agent)
 * 
 * PREREQUISITE: Run Scenario 6 first to register the owner!
 */

class restaurant_example16 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {
        
        // ========== API 1: RegisterUserOK (Customer) ==========
        {
            // PRE: not_in(email, dom(U_old))
            std::vector<std::unique_ptr<Expr>> pre1_args;
            pre1_args.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> dom_args1;
            dom_args1.push_back(std::make_unique<Var>("U_old"));
            pre1_args.push_back(std::make_unique<FuncCall>("dom", std::move(dom_args1)));
            auto pre1 = std::make_unique<FuncCall>("not_in", std::move(pre1_args));

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

            apis.push_back(std::make_unique<API>(std::move(pre1), std::move(call1),
                Response(HTTPResponseCode::CREATED_201, std::move(post1))));

            auto *c1 = new SymbolTable();
            c1->symtable.insert(Var("fullName"));
            c1->symtable.insert(Var("email"));
            c1->symtable.insert(Var("password"));
            c1->symtable.insert(Var("mobile"));
            c1->symtable.insert(Var("role"));
            root.children.push_back(c1);
        }

        // ========== API 2: LoginOK(email) ==========
        {
            // PRE: registered(email)
            std::vector<std::unique_ptr<Expr>> pre2_args;
            pre2_args.push_back(std::make_unique<Var>("email"));
            auto pre2 = std::make_unique<FuncCall>("registered", std::move(pre2_args));

            // CALL: login(email, password)
            std::vector<std::unique_ptr<Expr>> call2_args;
            call2_args.push_back(std::make_unique<Var>("email"));
            call2_args.push_back(std::make_unique<Var>("password"));
            auto call2 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("login", std::move(call2_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: authenticated(email)
            std::vector<std::unique_ptr<Expr>> post2_args;
            post2_args.push_back(std::make_unique<Var>("email"));
            auto post2 = std::make_unique<FuncCall>("authenticated", std::move(post2_args));

            apis.push_back(std::make_unique<API>(std::move(pre2), std::move(call2),
                Response(HTTPResponseCode::OK_200, std::move(post2))));

            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("email"));
            c2->symtable.insert(Var("password"));
            root.children.push_back(c2);
        }

        // ========== API 3: BrowseRestaurantsOK ==========
        {
            // PRE: token_present(email)
            std::vector<std::unique_ptr<Expr>> token_args3;
            token_args3.push_back(std::make_unique<Var>("email"));
            auto pre3 = std::make_unique<FuncCall>("token_present", std::move(token_args3));

            // CALL: browseRestaurants()
            auto call3 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("browseRestaurants", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: result_is_restaurant_list()
            auto post3 = std::make_unique<FuncCall>("result_is_restaurant_list", std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(std::move(pre3), std::move(call3),
                Response(HTTPResponseCode::OK_200, std::move(post3))));

            auto *c3 = new SymbolTable();
            c3->symtable.insert(Var("email"));
            root.children.push_back(c3);
        }

        // ========== API 4: ViewMenuOK(RestaurantX) ==========
        {
            // PRE: token_present(email)
            std::vector<std::unique_ptr<Expr>> token_args4;
            token_args4.push_back(std::make_unique<Var>("email"));
            auto pre4 = std::make_unique<FuncCall>("token_present", std::move(token_args4));

            // CALL: viewMenu(RestaurantX)
            std::vector<std::unique_ptr<Expr>> call4_args;
            call4_args.push_back(std::make_unique<Var>("RestaurantX"));
            auto call4 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewMenu", std::move(call4_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: result_is_menu()
            auto post4 = std::make_unique<FuncCall>("result_is_menu", std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(std::move(pre4), std::move(call4),
                Response(HTTPResponseCode::OK_200, std::move(post4))));

            auto *c4 = new SymbolTable();
            c4->symtable.insert(Var("email"));
            c4->symtable.insert(Var("RestaurantX"));
            root.children.push_back(c4);
        }

        // ========== API 5: AddToCartOK(ItemX) ==========
        {
            // PRE: token_present(email)
            std::vector<std::unique_ptr<Expr>> token_args5;
            token_args5.push_back(std::make_unique<Var>("email"));
            auto pre5 = std::make_unique<FuncCall>("token_present", std::move(token_args5));

            // CALL: addToCart(RestaurantX, ItemX, Quantity)
            std::vector<std::unique_ptr<Expr>> call5_args;
            call5_args.push_back(std::make_unique<Var>("RestaurantX"));
            call5_args.push_back(std::make_unique<Var>("ItemX"));
            call5_args.push_back(std::make_unique<Var>("Quantity"));
            auto call5 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("addToCart", std::move(call5_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: cart_contains(email, ItemX)
            std::vector<std::unique_ptr<Expr>> post5_args;
            post5_args.push_back(std::make_unique<Var>("email"));
            post5_args.push_back(std::make_unique<Var>("ItemX"));
            auto post5 = std::make_unique<FuncCall>("cart_contains", std::move(post5_args));

            apis.push_back(std::make_unique<API>(std::move(pre5), std::move(call5),
                Response(HTTPResponseCode::OK_200, std::move(post5))));

            auto *c5 = new SymbolTable();
            c5->symtable.insert(Var("email"));
            c5->symtable.insert(Var("RestaurantX"));
            c5->symtable.insert(Var("ItemX"));
            c5->symtable.insert(Var("Quantity"));
            root.children.push_back(c5);
        }

        // ========== API 6: PlaceOrderOK ==========
        {
            // PRE: token_present(email)
            std::vector<std::unique_ptr<Expr>> token_args6;
            token_args6.push_back(std::make_unique<Var>("email"));
            auto pre6 = std::make_unique<FuncCall>("token_present", std::move(token_args6));

            // CALL: placeOrder() - no parameters
            auto call6 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("placeOrder", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            // POST: order_recorded(email)
            std::vector<std::unique_ptr<Expr>> post6_args;
            post6_args.push_back(std::make_unique<Var>("email"));
            auto post6 = std::make_unique<FuncCall>("order_recorded", std::move(post6_args));

            apis.push_back(std::make_unique<API>(std::move(pre6), std::move(call6),
                Response(HTTPResponseCode::CREATED_201, std::move(post6))));

            auto *c6 = new SymbolTable();
            c6->symtable.insert(Var("email"));
            root.children.push_back(c6);
        }

        // ========== API 7: ViewOrdersOK (Customer) ==========
        {
            // PRE: token_present(email)
            std::vector<std::unique_ptr<Expr>> token_args7;
            token_args7.push_back(std::make_unique<Var>("email"));
            auto pre7 = std::make_unique<FuncCall>("token_present", std::move(token_args7));

            // CALL: viewOrders()
            auto call7 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewOrders", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: result_is_order_list()
            auto post7 = std::make_unique<FuncCall>("result_is_order_list", std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(std::move(pre7), std::move(call7),
                Response(HTTPResponseCode::OK_200, std::move(post7))));

            auto *c7 = new SymbolTable();
            c7->symtable.insert(Var("email"));
            root.children.push_back(c7);
        }

        // ========== API 8: OwnerLoginOK(ownerEmail) ==========
        {
            // PRE: registered(ownerEmail)
            std::vector<std::unique_ptr<Expr>> pre8_args;
            pre8_args.push_back(std::make_unique<Var>("ownerEmail"));
            auto pre8 = std::make_unique<FuncCall>("registered", std::move(pre8_args));

            // CALL: login(ownerEmail, password)
            std::vector<std::unique_ptr<Expr>> call8_args;
            call8_args.push_back(std::make_unique<Var>("ownerEmail"));
            call8_args.push_back(std::make_unique<Var>("password"));
            auto call8 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("login", std::move(call8_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: authenticated(ownerEmail)
            std::vector<std::unique_ptr<Expr>> post8_args;
            post8_args.push_back(std::make_unique<Var>("ownerEmail"));
            auto post8 = std::make_unique<FuncCall>("authenticated", std::move(post8_args));

            apis.push_back(std::make_unique<API>(std::move(pre8), std::move(call8),
                Response(HTTPResponseCode::OK_200, std::move(post8))));

            auto *c8 = new SymbolTable();
            c8->symtable.insert(Var("ownerEmail"));
            c8->symtable.insert(Var("password"));
            root.children.push_back(c8);
        }

        // ========== API 9: ViewOrdersOK (Owner) ==========
        {
            // PRE: token_present(ownerEmail)
            std::vector<std::unique_ptr<Expr>> token_args9;
            token_args9.push_back(std::make_unique<Var>("ownerEmail"));
            auto pre9 = std::make_unique<FuncCall>("token_present", std::move(token_args9));

            // CALL: viewOrders()
            auto call9 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewOrders", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: result_is_order_list()
            auto post9 = std::make_unique<FuncCall>("result_is_order_list", std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(std::move(pre9), std::move(call9),
                Response(HTTPResponseCode::OK_200, std::move(post9))));

            auto *c9 = new SymbolTable();
            c9->symtable.insert(Var("ownerEmail"));
            root.children.push_back(c9);
        }

        // ========== API 10: AssignOrderOK(Order, Agent) ==========
        {
            // PRE: token_present(ownerEmail)
            std::vector<std::unique_ptr<Expr>> token_args10;
            token_args10.push_back(std::make_unique<Var>("ownerEmail"));
            auto pre10 = std::make_unique<FuncCall>("token_present", std::move(token_args10));

            // CALL: assignOrder(orderId, agentId)
            std::vector<std::unique_ptr<Expr>> call10_args;
            call10_args.push_back(std::make_unique<Var>("orderId"));
            call10_args.push_back(std::make_unique<Var>("agentId"));
            auto call10 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("assignOrder", std::move(call10_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: order_assigned(orderId, agentId)
            std::vector<std::unique_ptr<Expr>> post10_args;
            post10_args.push_back(std::make_unique<Var>("orderId"));
            post10_args.push_back(std::make_unique<Var>("agentId"));
            auto post10 = std::make_unique<FuncCall>("order_assigned", std::move(post10_args));

            apis.push_back(std::make_unique<API>(std::move(pre10), std::move(call10),
                Response(HTTPResponseCode::OK_200, std::move(post10))));

            auto *c10 = new SymbolTable();
            c10->symtable.insert(Var("ownerEmail"));
            c10->symtable.insert(Var("orderId"));
            c10->symtable.insert(Var("agentId"));
            root.children.push_back(c10);
        }
    }
};
