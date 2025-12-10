#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

// Test string 1:
// RegisterUserOK(fullName, email, password, mobile, role), LoginOK(email), BrowseRestaurantsOK,
// ViewMenuOK(RestaurantX), AddToCartOK(RestaurantX, ItemX, Qty), PlaceOrderOK(Order1), ViewOrdersOK

class restaurant_example1 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {

        // =========================
        // 1. RegisterUserOK(fullName, email, password, mobile, role)
        // PRE:  email not_in dom(U_old)
        // CALL: registerUser(fullName, email, password, mobile, role) => 201
        // POST: U'[email] = password
        // =========================
        {
            // PRE: email not_in dom(U_old)
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

            // POST: U'[email] = password
            std::vector<std::unique_ptr<Expr>> post1_prime_args;
            post1_prime_args.push_back(std::make_unique<Var>("U"));
            auto u_prime = std::make_unique<FuncCall>("'", std::move(post1_prime_args));

            std::vector<std::unique_ptr<Expr>> index1;
            index1.push_back(std::move(u_prime));
            index1.push_back(std::make_unique<Var>("email"));

            std::vector<std::unique_ptr<Expr>> post1_args;
            post1_args.push_back(std::make_unique<FuncCall>("[]", std::move(index1)));
            post1_args.push_back(std::make_unique<Var>("password"));
            auto post1 = std::make_unique<FuncCall>("=", std::move(post1_args));

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
        // 2. LoginOK / Authenticate
        // PRE:  U[email] = password
        // CALL: login(email, password) => 200
        // POST: authenticated(email)
        // =========================
        {
            // PRE: U[email] = password
            std::vector<std::unique_ptr<Expr>> pre2_args;
            std::vector<std::unique_ptr<Expr>> idx2;

            idx2.push_back(std::make_unique<Var>("U"));
            idx2.push_back(std::make_unique<Var>("email"));
            pre2_args.push_back(std::make_unique<FuncCall>("[]", std::move(idx2)));
            pre2_args.push_back(std::make_unique<Var>("password"));

            auto pre2 = std::make_unique<FuncCall>("=", std::move(pre2_args));

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

            apis.push_back(std::make_unique<API>(
                std::move(pre2),
                std::move(call2),
                Response(HTTPResponseCode::OK_200, std::move(post2))
            ));

            // Symbol table for API 2
            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("email"));
            c2->symtable.insert(Var("password"));
            root.children.push_back(c2);
        }

        // =========================
        // 3. BrowseRestaurantsOK
        // PRE:  token_present(email)
        // CALL: browseRestaurants() => 200
        // POST: result_is_restaurant_list()
        // =========================
        {
            // PRE: token_present(email)
            std::vector<std::unique_ptr<Expr>> token_args1;
            token_args1.push_back(std::make_unique<Var>("email"));
            auto pre3 = std::make_unique<FuncCall>("token_present", std::move(token_args1));

            auto call3 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("browseRestaurants", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post3 = std::make_unique<FuncCall>("result_is_restaurant_list",
                                                    std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(
                std::move(pre3),
                std::move(call3),
                Response(HTTPResponseCode::OK_200, std::move(post3))
            ));

            // Symbol table for API 3
            auto *c3 = new SymbolTable();
            c3->symtable.insert(Var("email"));
            root.children.push_back(c3);
        }

        // =========================
        // 4. ViewMenuOK(RestaurantX)
        // PRE:  token_present(email)
        // CALL: viewMenu(RestaurantX) => 200
        // POST: result_is_menu()
        // =========================
        {
            // PRE: token_present(email)
            std::vector<std::unique_ptr<Expr>> token_args2;
            token_args2.push_back(std::make_unique<Var>("email"));
            auto pre4 = std::make_unique<FuncCall>("token_present", std::move(token_args2));

            // CALL: viewMenu(RestaurantX)
            std::vector<std::unique_ptr<Expr>> call4_args;
            call4_args.push_back(std::make_unique<Var>("RestaurantX"));
            auto call4 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewMenu", std::move(call4_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post4 = std::make_unique<FuncCall>("result_is_menu",
                                                    std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(
                std::move(pre4),
                std::move(call4),
                Response(HTTPResponseCode::OK_200, std::move(post4))
            ));

            // Symbol table for API 4
            auto *c4 = new SymbolTable();
            c4->symtable.insert(Var("email"));
            c4->symtable.insert(Var("RestaurantX"));
            root.children.push_back(c4);
        }

        // =========================
        // 5. AddToCartOK(RestaurantX, ItemX, Quantity)
        // PRE:  token_present(email)
        // CALL: addToCart(RestaurantX, ItemX, Quantity) => 200
        // POST: cart_contains(email, ItemX)
        // =========================
        {
            // PRE: token_present(email)
            std::vector<std::unique_ptr<Expr>> token_args3;
            token_args3.push_back(std::make_unique<Var>("email"));
            auto pre5 = std::make_unique<FuncCall>("token_present", std::move(token_args3));

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

            apis.push_back(std::make_unique<API>(
                std::move(pre5),
                std::move(call5),
                Response(HTTPResponseCode::OK_200, std::move(post5))
            ));

            // Symbol table for API 5
            auto *c5 = new SymbolTable();
            c5->symtable.insert(Var("email"));
            c5->symtable.insert(Var("RestaurantX"));
            c5->symtable.insert(Var("ItemX"));
            c5->symtable.insert(Var("Quantity"));
            root.children.push_back(c5);
        }

        // =========================
        // 6. PlaceOrderOK(Order1)
        // PRE:  token_present(email)
        // CALL: placeOrder() => 201
        // POST: order_recorded(email)
        // =========================
        {
            // PRE: token_present(email)
            std::vector<std::unique_ptr<Expr>> token_args4;
            token_args4.push_back(std::make_unique<Var>("email"));
            auto pre6 = std::make_unique<FuncCall>("token_present", std::move(token_args4));

            // CALL: placeOrder() - no parameters, uses cart
            std::vector<std::unique_ptr<Expr>> call6_args;
            auto call6 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("placeOrder", std::move(call6_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            // POST: order_recorded(email)
            std::vector<std::unique_ptr<Expr>> post6_args;
            post6_args.push_back(std::make_unique<Var>("email"));
            auto post6 = std::make_unique<FuncCall>("order_recorded", std::move(post6_args));

            apis.push_back(std::make_unique<API>(
                std::move(pre6),
                std::move(call6),
                Response(HTTPResponseCode::CREATED_201, std::move(post6))
            ));

            // Symbol table for API 6
            auto *c6 = new SymbolTable();
            c6->symtable.insert(Var("email"));
            root.children.push_back(c6);
        }

        // =========================
        // 7. ViewOrdersOK
        // PRE:  token_present(email)
        // CALL: viewOrders() => 200
        // POST: result_is_order_list()
        // =========================
        {
            // PRE: token_present(email)
            std::vector<std::unique_ptr<Expr>> token_args5;
            token_args5.push_back(std::make_unique<Var>("email"));
            auto pre7 = std::make_unique<FuncCall>("token_present", std::move(token_args5));

            auto call7 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewOrders", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post7 = std::make_unique<FuncCall>("result_is_order_list",
                                                    std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(
                std::move(pre7),
                std::move(call7),
                Response(HTTPResponseCode::OK_200, std::move(post7))
            ));

            // Symbol table for API 7
            auto *c7 = new SymbolTable();
            c7->symtable.insert(Var("email"));
            root.children.push_back(c7);
        }
    }
};
