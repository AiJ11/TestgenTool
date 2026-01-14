#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include "../../ast.hpp"
#include "../../algo.hpp"

/**
 * Restaurant Scenario 10: Owner Delete Menu Item
 * ===============================================
 * 
 * Flow: LoginOK(ownerEmail) -> DeleteMenuItemOK(ItemZ)
 * 
 * PREREQUISITE: Run Scenario 6 or 8 first to register owner and add menu items!
 */

class restaurant_example10 {
public:
    static void example(std::vector<std::unique_ptr<API>> &apis, SymbolTable &root) {
        
        // ========== API 1: LoginOK(ownerEmail) ==========
        {
            // PRE: registered(ownerEmail) - owner exists in U (not O!)
            std::vector<std::unique_ptr<Expr>> pre1_args;
            pre1_args.push_back(std::make_unique<Var>("ownerEmail"));
            auto pre1 = std::make_unique<FuncCall>("registered", std::move(pre1_args));

            // CALL: login(ownerEmail, password)
            std::vector<std::unique_ptr<Expr>> call1_args;
            call1_args.push_back(std::make_unique<Var>("ownerEmail"));
            call1_args.push_back(std::make_unique<Var>("password"));
            auto call1 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("login", std::move(call1_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: authenticated(ownerEmail)
            std::vector<std::unique_ptr<Expr>> post1_args;
            post1_args.push_back(std::make_unique<Var>("ownerEmail"));
            auto post1 = std::make_unique<FuncCall>("authenticated", std::move(post1_args));

            apis.push_back(std::make_unique<API>(std::move(pre1), std::move(call1),
                Response(HTTPResponseCode::OK_200, std::move(post1))));

            // Symbol table for API 1
            auto *c1 = new SymbolTable();
            c1->symtable.insert(Var("ownerEmail"));
            c1->symtable.insert(Var("password"));
            root.children.push_back(c1);
        }

        // ========== API 2: DeleteMenuItemOK(ItemZ) ==========
        {
            // PRE: token_present(ownerEmail) - FIXED: Need auth check
            std::vector<std::unique_ptr<Expr>> token_args2;
            token_args2.push_back(std::make_unique<Var>("ownerEmail"));
            auto pre2 = std::make_unique<FuncCall>("token_present", std::move(token_args2));

            // CALL: deleteMenuItem(itemId)
            std::vector<std::unique_ptr<Expr>> call2_args;
            call2_args.push_back(std::make_unique<Var>("itemId"));
            auto call2 = std::make_unique<APIcall>(
                std::make_unique<FuncCall>("deleteMenuItem", std::move(call2_args)),
                Response(HTTPResponseCode::OK_200, nullptr)
            );

            // POST: menu_item_deleted(itemId)
            std::vector<std::unique_ptr<Expr>> post2_args;
            post2_args.push_back(std::make_unique<Var>("itemId"));
            auto post2 = std::make_unique<FuncCall>("menu_item_deleted", std::move(post2_args));

            apis.push_back(std::make_unique<API>(std::move(pre2), std::move(call2),
                Response(HTTPResponseCode::OK_200, std::move(post2))));

            // Symbol table for API 2
            auto *c2 = new SymbolTable();
            c2->symtable.insert(Var("ownerEmail"));
            c2->symtable.insert(Var("itemId"));
            root.children.push_back(c2);
        }
    }
};
