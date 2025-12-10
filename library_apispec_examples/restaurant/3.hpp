#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

// Test string 3:
// RegisterUserOK(email), LoginOK(email), BrowseRestaurantsOK, ViewMenuOK(RestaurantY),
// AddToCartOK(ItemY), PlaceOrderOK(Order2), ViewOrdersOK, CreateReviewOK(Order2)

class restaurant_example3 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {
        
        // ========== API 1: RegisterUserOK ==========
        {
            std::vector<std::unique_ptr<Expr>> pre1_args;
            pre1_args.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> dom_args1;
            dom_args1.push_back(std::make_unique<Var>("U"));
            pre1_args.push_back(std::make_unique<FuncCall>("dom", std::move(dom_args1)));
            auto pre1 = std::make_unique<FuncCall>("not_in", std::move(pre1_args));

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

            std::vector<std::unique_ptr<Expr>> post1_args1;
            post1_args1.push_back(std::make_unique<Var>("U"));
            auto u_prime = std::make_unique<FuncCall>("'", std::move(post1_args1));
            std::vector<std::unique_ptr<Expr>> index1;
            index1.push_back(std::move(u_prime));
            index1.push_back(std::make_unique<Var>("email"));

            std::vector<std::unique_ptr<Expr>> post1_args;
            post1_args.push_back(std::make_unique<FuncCall>("[]", std::move(index1)));
            post1_args.push_back(std::make_unique<Var>("password"));
            auto post1 = std::make_unique<FuncCall>("=", std::move(post1_args));

            apis.push_back(std::make_unique<API>(std::move(pre1), std::move(call1),
                Response(HTTPResponseCode::CREATED_201, std::move(post1))));

            // Symbol table for API 1
            auto *c1 = new SymbolTable();
            c1->symtable.insert(Var("fullName"));
            c1->symtable.insert(Var("email"));
            c1->symtable.insert(Var("password"));
            c1->symtable.insert(Var("mobile"));
            c1->symtable.insert(Var("role"));
            root.children.push_back(c1);
        }

        // ========== API 2: LoginOK ==========
{
    //PRECONDITION: U[email] = password
    std::vector<std::unique_ptr<Expr>> idx_args;
    idx_args.push_back(std::make_unique<Var>("U"));
    idx_args.push_back(std::make_unique<Var>("email"));
    
    std::vector<std::unique_ptr<Expr>> pre2_args;
    pre2_args.push_back(std::make_unique<FuncCall>("[]", std::move(idx_args)));
    pre2_args.push_back(std::make_unique<Var>("password"));
    auto pre2 = std::make_unique<FuncCall>("=", std::move(pre2_args));

    std::vector<std::unique_ptr<Expr>> call2_args;
    call2_args.push_back(std::make_unique<Var>("email"));
    call2_args.push_back(std::make_unique<Var>("password"));
    auto call2 = std::make_unique<APIcall>(
        std::make_unique<FuncCall>("login", std::move(call2_args)),
        Response(HTTPResponseCode::OK_200, nullptr)
    );

    std::vector<std::unique_ptr<Expr>> post2_args;
    post2_args.push_back(std::make_unique<Var>("email"));
    auto post2 = std::make_unique<FuncCall>("authenticated", std::move(post2_args));

    apis.push_back(std::make_unique<API>(std::move(pre2), std::move(call2),
        Response(HTTPResponseCode::OK_200, std::move(post2))));

    // Symbol table for API 2
    auto *c2 = new SymbolTable();
    c2->symtable.insert(Var("email"));
    c2->symtable.insert(Var("password"));
    root.children.push_back(c2);
    
}
        // ========== API 3: BrowseRestaurantsOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args1;
            token_args1.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre3_args;
            pre3_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args1)));
            auto pre3 = std::make_unique<FuncCall>("and", std::move(pre3_args));

            auto call3 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("browseRestaurants", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post3 = std::make_unique<FuncCall>("result_is_restaurant_list", std::vector<std::unique_ptr<Expr>>{});
            
            apis.push_back(std::make_unique<API>(std::move(pre3), std::move(call3),
                Response(HTTPResponseCode::OK_200, std::move(post3))));

            // Symbol table for API 3
            auto *c3 = new SymbolTable();
            c3->symtable.insert(Var("email"));
            root.children.push_back(c3);
        }

        // ========== API 4: ViewMenuOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args2;
            token_args2.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre4_args;
            pre4_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args2)));
            auto pre4 = std::make_unique<FuncCall>("and", std::move(pre4_args));

            std::vector<std::unique_ptr<Expr>> call4_args;
            call4_args.push_back(std::make_unique<Var>("RestaurantY"));
            auto call4 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewMenu", std::move(call4_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post4 = std::make_unique<FuncCall>("result_is_menu", std::vector<std::unique_ptr<Expr>>{});
            
            apis.push_back(std::make_unique<API>(std::move(pre4), std::move(call4),
                Response(HTTPResponseCode::OK_200, std::move(post4))));

            // Symbol table for API 4
            auto *c4 = new SymbolTable();
            c4->symtable.insert(Var("email"));
            c4->symtable.insert(Var("RestaurantY"));
            root.children.push_back(c4);
        }

        // ========== API 5: AddToCartOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args3;
            token_args3.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre5_args;
            pre5_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args3)));
            auto pre5 = std::make_unique<FuncCall>("and", std::move(pre5_args));

            std::vector<std::unique_ptr<Expr>> call5_args;
            call5_args.push_back(std::make_unique<Var>("RestaurantY"));
            call5_args.push_back(std::make_unique<Var>("ItemY"));
            call5_args.push_back(std::make_unique<Var>("Quantity"));
            auto call5 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("addToCart", std::move(call5_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post5_args;
            post5_args.push_back(std::make_unique<Var>("email"));
            post5_args.push_back(std::make_unique<Var>("ItemY"));
            auto post5 = std::make_unique<FuncCall>("cart_contains", std::move(post5_args));

            apis.push_back(std::make_unique<API>(std::move(pre5), std::move(call5),
                Response(HTTPResponseCode::OK_200, std::move(post5))));

            // Symbol table for API 5
            auto *c5 = new SymbolTable();
            c5->symtable.insert(Var("email"));
            c5->symtable.insert(Var("RestaurantY"));
            c5->symtable.insert(Var("ItemY"));
            c5->symtable.insert(Var("Quantity"));
            root.children.push_back(c5);
        }

        // ========== API 6: PlaceOrderOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args4;
            token_args4.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre6_args;
            pre6_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args4)));
            auto pre6 = std::make_unique<FuncCall>("and", std::move(pre6_args));

            std::vector<std::unique_ptr<Expr>> call6_args;
            // Empty - placeOrder takes 0 parameters
            auto call6 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("placeOrder", std::move(call6_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post6_args;
            post6_args.push_back(std::make_unique<Var>("Order2"));
            post6_args.push_back(std::make_unique<Var>("email"));
            auto post6 = std::make_unique<FuncCall>("order_recorded", std::move(post6_args));

            apis.push_back(std::make_unique<API>(std::move(pre6), std::move(call6),
                Response(HTTPResponseCode::CREATED_201, std::move(post6))));

            // Symbol table for API 6
            auto *c6 = new SymbolTable();
            c6->symtable.insert(Var("email"));
            // Order2 is NOT in symbol table - it's symbolic in postcondition
            root.children.push_back(c6);
        }

        // ========== API 7: ViewOrdersOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args5;
            token_args5.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre7_args;
            pre7_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args5)));
            auto pre7 = std::make_unique<FuncCall>("and", std::move(pre7_args));

            auto call7 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewOrders", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post7 = std::make_unique<FuncCall>("result_is_order_list", std::vector<std::unique_ptr<Expr>>{});
            
            apis.push_back(std::make_unique<API>(std::move(pre7), std::move(call7),
                Response(HTTPResponseCode::OK_200, std::move(post7))));

            // Symbol table for API 7
            auto *c7 = new SymbolTable();
            c7->symtable.insert(Var("email"));
            root.children.push_back(c7);
        }

        // ========== API 8: CreateReviewOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args6;
            token_args6.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre8_args;
            pre8_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args6)));
            std::vector<std::unique_ptr<Expr>> pre8_args2;
            pre8_args2.push_back(std::make_unique<Var>("Order2"));
            auto pre8_order_check = std::make_unique<FuncCall>("order_exists", std::move(pre8_args2));
            pre8_args.push_back(std::move(pre8_order_check));
            auto pre8 = std::make_unique<FuncCall>("and", std::move(pre8_args));

            std::vector<std::unique_ptr<Expr>> call8_args;
            call8_args.push_back(std::make_unique<Var>("orderId"));
            call8_args.push_back(std::make_unique<Var>("restaurantId"));
            call8_args.push_back(std::make_unique<Var>("rating"));
            call8_args.push_back(std::make_unique<Var>("comment"));
            auto call8 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("createReview", std::move(call8_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post8_args;
            post8_args.push_back(std::make_unique<Var>("Order2"));
            post8_args.push_back(std::make_unique<Var>("comment"));
            auto post8 = std::make_unique<FuncCall>("review_added", std::move(post8_args));

            apis.push_back(std::make_unique<API>(std::move(pre8), std::move(call8),
                Response(HTTPResponseCode::CREATED_201, std::move(post8))));

            // Symbol table for API 8
            auto *c8 = new SymbolTable();
            c8->symtable.insert(Var("email"));
            c8->symtable.insert(Var("orderId"));
            c8->symtable.insert(Var("restaurantId"));
            c8->symtable.insert(Var("rating"));
            c8->symtable.insert(Var("comment"));
            root.children.push_back(c8);
        }
    }
};
