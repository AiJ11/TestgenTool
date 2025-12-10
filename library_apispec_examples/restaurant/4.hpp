#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

// Test string 4:
// LoginOK(email), BrowseRestaurantsOK, ViewMenuOK(RestaurantX), AddToCartOK(ItemX), 
// RemoveCartItemOK(ItemX), AddToCartOK(ItemZ), PlaceOrderOK(Order3)

class restaurant_example4 {
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

        // ========== API 2: BrowseRestaurantsOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args1;
            token_args1.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre2_args;
            pre2_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args1)));
            auto pre2 = std::make_unique<FuncCall>("and", std::move(pre2_args));

            auto call2 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("browseRestaurants", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post2 = std::make_unique<FuncCall>("result_is_restaurant_list", std::vector<std::unique_ptr<Expr>>{});
            
            apis.push_back(std::make_unique<API>(std::move(pre2), std::move(call2),
                Response(HTTPResponseCode::OK_200, std::move(post2))));

            // Symbol table for API 2
            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("email"));
            root.children.push_back(c2);
        }

        // ========== API 3: ViewMenuOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args2;
            token_args2.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre3_args;
            pre3_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args2)));
            auto pre3 = std::make_unique<FuncCall>("and", std::move(pre3_args));

            std::vector<std::unique_ptr<Expr>> call3_args;
            call3_args.push_back(std::make_unique<Var>("RestaurantX"));
            auto call3 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewMenu", std::move(call3_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post3 = std::make_unique<FuncCall>("result_is_menu", std::vector<std::unique_ptr<Expr>>{});
            
            apis.push_back(std::make_unique<API>(std::move(pre3), std::move(call3),
                Response(HTTPResponseCode::OK_200, std::move(post3))));

            // Symbol table for API 3
            auto *c3 = new SymbolTable();
            c3->symtable.insert(Var("email"));
            c3->symtable.insert(Var("RestaurantX"));
            root.children.push_back(c3);
        }

        // ========== API 4: AddToCartOK(ItemX) ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args3;
            token_args3.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre4_args;
            pre4_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args3)));
            auto pre4 = std::make_unique<FuncCall>("and", std::move(pre4_args));

            //  3 parameters
            std::vector<std::unique_ptr<Expr>> call4_args;
            call4_args.push_back(std::make_unique<Var>("RestaurantX"));
            call4_args.push_back(std::make_unique<Var>("ItemX"));
            call4_args.push_back(std::make_unique<Var>("QuantityX"));
            auto call4 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("addToCart", std::move(call4_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post4_args;
            post4_args.push_back(std::make_unique<Var>("email"));
            post4_args.push_back(std::make_unique<Var>("ItemX"));
            auto post4 = std::make_unique<FuncCall>("cart_contains", std::move(post4_args));

            apis.push_back(std::make_unique<API>(std::move(pre4), std::move(call4),
                Response(HTTPResponseCode::OK_200, std::move(post4))));

            // Symbol table for API 4
            auto *c4 = new SymbolTable();
            c4->symtable.insert(Var("email"));
            c4->symtable.insert(Var("RestaurantX"));
            c4->symtable.insert(Var("ItemX"));
            c4->symtable.insert(Var("QuantityX"));
            root.children.push_back(c4);
        }

        // ========== API 5: RemoveFromCartOK(ItemX) ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args5;
            token_args5.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre5_args;
            pre5_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args5)));
            auto pre5 = std::make_unique<FuncCall>("and", std::move(pre5_args));

            //  1 parameter (menuItemId)
            std::vector<std::unique_ptr<Expr>> call5_args;
            call5_args.push_back(std::make_unique<Var>("ItemX"));
            auto call5 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("removeFromCart", std::move(call5_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post5_args;
            post5_args.push_back(std::make_unique<Var>("email"));
            post5_args.push_back(std::make_unique<Var>("ItemX"));
            auto post5 = std::make_unique<FuncCall>("cart_not_contains", std::move(post5_args));

            apis.push_back(std::make_unique<API>(std::move(pre5), std::move(call5),
                Response(HTTPResponseCode::OK_200, std::move(post5))));

            // Symbol table for API 5
            auto *c5 = new SymbolTable();
            c5->symtable.insert(Var("email"));
            c5->symtable.insert(Var("ItemX"));
            root.children.push_back(c5);
        }

        // ========== API 6: AddToCartOK(ItemZ) ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args6;
            token_args6.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre6_args;
            pre6_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args6)));
            auto pre6 = std::make_unique<FuncCall>("and", std::move(pre6_args));

            //FIXED: 3 parameters
            std::vector<std::unique_ptr<Expr>> call6_args;
            call6_args.push_back(std::make_unique<Var>("RestaurantX"));
            call6_args.push_back(std::make_unique<Var>("ItemZ"));
            call6_args.push_back(std::make_unique<Var>("QuantityZ"));
            auto call6 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("addToCart", std::move(call6_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post6_args;
            post6_args.push_back(std::make_unique<Var>("email"));
            post6_args.push_back(std::make_unique<Var>("ItemZ"));
            auto post6 = std::make_unique<FuncCall>("cart_contains", std::move(post6_args));

            apis.push_back(std::make_unique<API>(std::move(pre6), std::move(call6),
                Response(HTTPResponseCode::OK_200, std::move(post6))));

            // Symbol table for API 6
            auto *c6 = new SymbolTable();
            c6->symtable.insert(Var("email"));
            c6->symtable.insert(Var("RestaurantX"));
            c6->symtable.insert(Var("ItemZ"));
            c6->symtable.insert(Var("QuantityZ"));
            root.children.push_back(c6);
        }

        // ========== API 7: PlaceOrderOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args7;
            token_args7.push_back(std::make_unique<Var>("email"));
            std::vector<std::unique_ptr<Expr>> pre7_args;
            pre7_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args7)));
            auto pre7 = std::make_unique<FuncCall>("and", std::move(pre7_args));

            //FIXED: 0 parameters
            std::vector<std::unique_ptr<Expr>> call7_args;
            // Empty - no parameters!
            auto call7 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("placeOrder", std::move(call7_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post7_args;
            post7_args.push_back(std::make_unique<Var>("Order3"));
            post7_args.push_back(std::make_unique<Var>("email"));
            auto post7 = std::make_unique<FuncCall>("order_recorded", std::move(post7_args));

            apis.push_back(std::make_unique<API>(std::move(pre7), std::move(call7),
                Response(HTTPResponseCode::CREATED_201, std::move(post7))));

            // Symbol table for API 7
            auto *c7 = new SymbolTable();
            c7->symtable.insert(Var("email"));
            // Order3 is NOT in symbol table - it's symbolic in postcondition
            root.children.push_back(c7);
        }
    }
};
