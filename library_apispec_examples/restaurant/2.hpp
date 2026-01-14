#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

// Test string 2:
// LoginOK(email), BrowseRestaurantsOK, ViewMenuOK(RestaurantX),
// AddToCartOK(ItemX), ViewCartOK, PlaceOrderOK(Order1), AddReviewOK(Order1)(create review)

class restaurant_example2 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {

        // =========================
        // 1. LoginOK(email)
        // PRE:  U[email] = password
        // CALL: login(email, password) => 200
        // POST: authenticated(email)
        // =========================
        {
            // PRE: U[email] = password   *** FIX ***
            std::vector<std::unique_ptr<Expr>> pre1_args;
            std::vector<std::unique_ptr<Expr>> idx1;

            idx1.push_back(std::make_unique<Var>("U"));
            idx1.push_back(std::make_unique<Var>("email"));
            pre1_args.push_back(std::make_unique<FuncCall>("[]", std::move(idx1)));
            pre1_args.push_back(std::make_unique<Var>("password"));

            auto pre1 = std::make_unique<FuncCall>("=", std::move(pre1_args));

            // CALL: login(email, password)
            std::vector<std::unique_ptr<Expr>> call1_args;
            call1_args.push_back(std::make_unique<Var>("email"));
            call1_args.push_back(std::make_unique<Var>("password"));
            auto call1 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("login", std::move(call1_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: authenticated(email)
            std::vector<std::unique_ptr<Expr>> post1_args;
            post1_args.push_back(std::make_unique<Var>("email"));
            auto post1 = std::make_unique<FuncCall>("authenticated", std::move(post1_args));

            apis.push_back(std::make_unique<API>(
                std::move(pre1),
                std::move(call1),
                Response(HTTPResponseCode::OK_200, std::move(post1))
            ));

            // Symbol table for API 1
            auto *c1 = new SymbolTable();
            c1->symtable.insert(Var("email"));
            c1->symtable.insert(Var("password"));
            root.children.push_back(c1);
        }

        // =========================
        // 2. BrowseRestaurantsOK
        // PRE:  token_present(email)
        // CALL: browseRestaurants() => 200
        // POST: result_is_restaurant_list()
        // =========================
        {
            std::vector<std::unique_ptr<Expr>> token_args1;
            token_args1.push_back(std::make_unique<Var>("token"));
            auto pre2 = std::make_unique<FuncCall>("token_present", std::move(token_args1));

            auto call2 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("browseRestaurants", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post2 = std::make_unique<FuncCall>("result_is_restaurant_list",
                                                    std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(
                std::move(pre2),
                std::move(call2),
                Response(HTTPResponseCode::OK_200, std::move(post2))
            ));

            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("email"));
            c2->symtable.insert(Var("token"));
            root.children.push_back(c2);
        }

        // =========================
        // 3. ViewMenuOK(RestaurantX)
        // PRE:  token_present(email)
        // CALL: viewMenu(RestaurantX) => 200
        // POST: result_is_menu()
        // =========================
        {
            std::vector<std::unique_ptr<Expr>> token_args2;
            token_args2.push_back(std::make_unique<Var>("token"));
            auto pre3 = std::make_unique<FuncCall>("token_present", std::move(token_args2));

            std::vector<std::unique_ptr<Expr>> call3_args;
            call3_args.push_back(std::make_unique<Var>("RestaurantX"));
            auto call3 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewMenu", std::move(call3_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post3 = std::make_unique<FuncCall>("result_is_menu",
                                                    std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(
                std::move(pre3),
                std::move(call3),
                Response(HTTPResponseCode::OK_200, std::move(post3))
            ));

            auto *c3 = new SymbolTable();
            c3->symtable.insert(Var("email"));
            c3->symtable.insert(Var("token"));
            c3->symtable.insert(Var("RestaurantX"));
            root.children.push_back(c3);
        }

        // =========================
        // 4. AddToCartOK(ItemX)
        // PRE:  token_present(email)
        // CALL: addToCart(ItemX) => 200
        // POST: cart_contains(email, ItemX)
        // =========================
        {
            std::vector<std::unique_ptr<Expr>> token_args3;
            token_args3.push_back(std::make_unique<Var>("token"));
            auto pre4 = std::make_unique<FuncCall>("token_present", std::move(token_args3));

            std::vector<std::unique_ptr<Expr>> call4_args;
            call4_args.push_back(std::make_unique<Var>("RestaurantX"));  // restaurantId
            call4_args.push_back(std::make_unique<Var>("ItemX"));        // menuItemId
            call4_args.push_back(std::make_unique<Var>("Quantity"));     // quantity
            auto call4 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("addToCart", std::move(call4_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post4_args;
            post4_args.push_back(std::make_unique<Var>("email"));
            post4_args.push_back(std::make_unique<Var>("ItemX"));
            auto post4 = std::make_unique<FuncCall>("cart_contains", std::move(post4_args));

            apis.push_back(std::make_unique<API>(
                std::move(pre4),
                std::move(call4),
                Response(HTTPResponseCode::OK_200, std::move(post4))
            ));

            auto *c4 = new SymbolTable();
            c4->symtable.insert(Var("email"));
            c4->symtable.insert(Var("token"));
            c4->symtable.insert(Var("RestaurantX"));
            c4->symtable.insert(Var("ItemX"));
            c4->symtable.insert(Var("Quantity"));
            root.children.push_back(c4);
        }

        // =========================
        // 5. ViewCartOK
        // PRE:  token_present(email)
        // CALL: viewCart() => 200
        // POST: result_is_cart()
        // =========================
        {
            std::vector<std::unique_ptr<Expr>> token_args4;
            token_args4.push_back(std::make_unique<Var>("token"));
            auto pre5 = std::make_unique<FuncCall>("token_present", std::move(token_args4));

            auto call5 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewCart", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post5 = std::make_unique<FuncCall>("result_is_cart",
                                                    std::vector<std::unique_ptr<Expr>>{});

            apis.push_back(std::make_unique<API>(
                std::move(pre5),
                std::move(call5),
                Response(HTTPResponseCode::OK_200, std::move(post5))
            ));

            auto *c5 = new SymbolTable();
            c5->symtable.insert(Var("email"));
            c5->symtable.insert(Var("token"));
            root.children.push_back(c5);
        }

        // =========================
        // 6. PlaceOrderOK(Order1)
        // PRE:  token_present(email)
        // CALL: placeOrder(Order1) => 201
        // POST: order_recorded(Order1, email)
        // =========================
        {
            std::vector<std::unique_ptr<Expr>> token_args5;
            token_args5.push_back(std::make_unique<Var>("token"));
            auto pre6 = std::make_unique<FuncCall>("token_present", std::move(token_args5));

            std::vector<std::unique_ptr<Expr>> call6_args;
            call6_args.push_back(std::make_unique<Var>("Order1"));
            auto call6 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("placeOrder", std::move(call6_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post6_args;
            post6_args.push_back(std::make_unique<Var>("Order1"));
            post6_args.push_back(std::make_unique<Var>("email"));
            auto post6 = std::make_unique<FuncCall>("order_recorded", std::move(post6_args));

            apis.push_back(std::make_unique<API>(
                std::move(pre6),
                std::move(call6),
                Response(HTTPResponseCode::CREATED_201, std::move(post6))
            ));

            auto *c6 = new SymbolTable();
            c6->symtable.insert(Var("email"));
            c6->symtable.insert(Var("token"));
            c6->symtable.insert(Var("Order1"));
            root.children.push_back(c6);
        }

        // =========================
        // 7. AddReviewOK(Order1)(creating review)
        // PRE:  token_present(email)
        // CALL: addReview(Order1, reviewDetails(rating, comment)) => 200
        // POST: review_added()
        // =========================
        {
    std::vector<std::unique_ptr<Expr>> token_args6;
    token_args6.push_back(std::make_unique<Var>("token"));
    auto pre7 = std::make_unique<FuncCall>("token_present", std::move(token_args6));

    // CALL: createReview(orderId, restaurantId, rating, comment)
    std::vector<std::unique_ptr<Expr>> call7_args;
    call7_args.push_back(std::make_unique<Var>("orderId"));
    call7_args.push_back(std::make_unique<Var>("restaurantId"));
    call7_args.push_back(std::make_unique<Var>("rating"));
    call7_args.push_back(std::make_unique<Var>("comment"));

    auto call7 = std::make_unique<APIcall>(
        std::make_unique<FuncCall>("createReview", std::move(call7_args)),
        Response(HTTPResponseCode::CREATED_201, nullptr)
    );

    // POST: review_added()
    auto post7 = std::make_unique<FuncCall>("review_added",
                                            std::vector<std::unique_ptr<Expr>>{});

    apis.push_back(std::make_unique<API>(
        std::move(pre7),
        std::move(call7),
        Response(HTTPResponseCode::CREATED_201, std::move(post7))
    ));

    // Symbol table for API 7
    auto *c7 = new SymbolTable();
    c7->symtable.insert(Var("email"));
    c7->symtable.insert(Var("token"));
    c7->symtable.insert(Var("orderId"));
    c7->symtable.insert(Var("restaurantId"));
    c7->symtable.insert(Var("rating"));
    c7->symtable.insert(Var("comment"));
    root.children.push_back(c7);
}
    }
};
