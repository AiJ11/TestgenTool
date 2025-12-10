#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

/**
 * Restaurant Scenario 7: Owner Update Restaurant
 * ==============================================
 * 
 * Flow: Login -> CreateRestaurant -> UpdateRestaurant -> ViewOrders
 * 
 * This scenario tests updating restaurant details:
 * 1. Register a new owner
 * 2. Login as owner
 * 3. Create a restaurant
 * 4. Update the restaurant details
 * 5. View orders
 * 
 * IMPORTANT: Uses ownerEmail, ownerRole variables to trigger correct seed values
 * 
 * Logical State:
 * - U: (email -> user) map
 * - T: (email -> token) map
 * - R: (restaurantId -> restaurant) map
 */

class restaurant_example7 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {
        
        // ========== API 1: RegisterOwner ==========
        {
            std::vector<std::unique_ptr<Expr>> pre1_args;
            pre1_args.push_back(std::make_unique<Var>("ownerEmail"));
            auto pre1 = std::make_unique<FuncCall>("not_registered", std::move(pre1_args));

            std::vector<std::unique_ptr<Expr>> call1_args;
            call1_args.push_back(std::make_unique<Var>("ownerName"));      // → "Owner_12345"
            call1_args.push_back(std::make_unique<Var>("ownerEmail"));     // → "owner12345_0@test.com"
            call1_args.push_back(std::make_unique<Var>("password"));       // → "Pass123!"
            call1_args.push_back(std::make_unique<Var>("ownerMobile"));    // → "8100000000"
            call1_args.push_back(std::make_unique<Var>("ownerRole"));      // → "restaurant_owner"
            auto call1 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("registerUser", std::move(call1_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post1_args;
            post1_args.push_back(std::make_unique<Var>("ownerEmail"));
            auto post1 = std::make_unique<FuncCall>("registered", std::move(post1_args));

            apis.push_back(std::make_unique<API>(std::move(pre1), std::move(call1),
                Response(HTTPResponseCode::CREATED_201, std::move(post1))));

            auto *c1 = new SymbolTable();
            c1->symtable.insert(Var("ownerName"));
            c1->symtable.insert(Var("ownerEmail"));
            c1->symtable.insert(Var("password"));
            c1->symtable.insert(Var("ownerMobile"));
            c1->symtable.insert(Var("ownerRole"));
            root.children.push_back(c1);
        }

        // ========== API 2: LoginOK ==========
        {
            std::vector<std::unique_ptr<Expr>> pre2_args;
            pre2_args.push_back(std::make_unique<Var>("ownerEmail"));
            auto pre2 = std::make_unique<FuncCall>("registered", std::move(pre2_args));

            std::vector<std::unique_ptr<Expr>> call2_args;
            call2_args.push_back(std::make_unique<Var>("ownerEmail"));
            call2_args.push_back(std::make_unique<Var>("password"));
            auto call2 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("login", std::move(call2_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post2_args;
            post2_args.push_back(std::make_unique<Var>("ownerEmail"));
            auto post2 = std::make_unique<FuncCall>("authenticated", std::move(post2_args));

            apis.push_back(std::make_unique<API>(std::move(pre2), std::move(call2),
                Response(HTTPResponseCode::OK_200, std::move(post2))));

            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("ownerEmail"));
            c2->symtable.insert(Var("password"));
            root.children.push_back(c2);
        }

        // ========== API 3: CreateRestaurantOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args3;
            token_args3.push_back(std::make_unique<Var>("ownerEmail"));
            std::vector<std::unique_ptr<Expr>> pre3_args;
            pre3_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args3)));
            auto pre3 = std::make_unique<FuncCall>("and", std::move(pre3_args));

            std::vector<std::unique_ptr<Expr>> call3_args;
            call3_args.push_back(std::make_unique<Var>("restaurantName"));  // → "TestRestaurant_12345"
            auto call3 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("createRestaurant", std::move(call3_args)),
                Response(HTTPResponseCode::CREATED_201, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post3_args;
            post3_args.push_back(std::make_unique<Var>("restaurantName"));
            auto post3 = std::make_unique<FuncCall>("restaurant_created", std::move(post3_args));

            apis.push_back(std::make_unique<API>(std::move(pre3), std::move(call3),
                Response(HTTPResponseCode::CREATED_201, std::move(post3))));

            auto *c3 = new SymbolTable();
            c3->symtable.insert(Var("ownerEmail"));
            c3->symtable.insert(Var("restaurantName"));
            root.children.push_back(c3);
        }

        // ========== API 4: UpdateRestaurantOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args4;
            token_args4.push_back(std::make_unique<Var>("ownerEmail"));
            std::vector<std::unique_ptr<Expr>> pre4_args;
            pre4_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args4)));
            auto pre4 = std::make_unique<FuncCall>("and", std::move(pre4_args));

            std::vector<std::unique_ptr<Expr>> call4_args;
            call4_args.push_back(std::make_unique<Var>("restaurantId"));  // → "507f1f77bcf86cd799439011" (or from context)
            auto call4 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("updateRestaurant", std::move(call4_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            std::vector<std::unique_ptr<Expr>> post4_args;
            post4_args.push_back(std::make_unique<Var>("restaurantId"));
            auto post4 = std::make_unique<FuncCall>("restaurant_updated", std::move(post4_args));

            apis.push_back(std::make_unique<API>(std::move(pre4), std::move(call4),
                Response(HTTPResponseCode::OK_200, std::move(post4))));

            auto *c4 = new SymbolTable();
            c4->symtable.insert(Var("ownerEmail"));
            c4->symtable.insert(Var("restaurantId"));
            root.children.push_back(c4);
        }

        // ========== API 5: ViewOrdersOK ==========
        {
            std::vector<std::unique_ptr<Expr>> token_args5;
            token_args5.push_back(std::make_unique<Var>("ownerEmail"));
            std::vector<std::unique_ptr<Expr>> pre5_args;
            pre5_args.push_back(std::make_unique<FuncCall>("token_present", std::move(token_args5)));
            auto pre5 = std::make_unique<FuncCall>("and", std::move(pre5_args));

            auto call5 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("viewOrders", std::vector<std::unique_ptr<Expr>>{}),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            auto post5 = std::make_unique<FuncCall>("result_is_order_list", std::vector<std::unique_ptr<Expr>>{});
            
            apis.push_back(std::make_unique<API>(std::move(pre5), std::move(call5),
                Response(HTTPResponseCode::OK_200, std::move(post5))));

            auto *c5 = new SymbolTable();
            c5->symtable.insert(Var("ownerEmail"));
            root.children.push_back(c5);
        }
    }
};
