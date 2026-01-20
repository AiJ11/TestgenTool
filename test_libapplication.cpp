#include <iostream>
#include <memory>
#include <map>
#include <functional>
#include "ast.hh"
#include "printvisitor.hh"
#include "algo.hpp"
#include "tester/tester.hh"
#include "env.hh"
#include "see/restaurantfunctionfactory.hh"
#include "see/ecommercefunctionfactory.hh"
#include "see/see.hh"

// Import webapp-specific specs
#include "specs/RestaurantSpec.hpp"
#include "specs/EcommerceSpec.hpp"


using namespace std;

// ============================================
// TEST EXECUTION MODES
// ============================================

enum class TestMode
{
    ORIGINAL,     // Just genATC (no rewrite)
    REWRITE_ONLY, // genATC + RewriteGlobalsVisitor (no backend)
    FULL_PIPELINE // Complete: genATC + Rewrite + SEE + Backend
};

// ============================================
// TEST EXECUTOR
// ============================================

class TestExecutor
{
private:
    TestMode mode;
    string backendUrl;

public:
    TestExecutor(TestMode m, const string &url = "http://localhost:5002")
        : mode(m), backendUrl(url) {}

    void runTest(
        const string &testName,
        unique_ptr<Spec> spec,
        const vector<string> &testSequence)
    {
        cout << "\n========================================" << endl;
        cout << "TEST: " << testName << endl;
        cout << "MODE: " << getModeString() << endl;
        cout << "DEPTH: " << testSequence.size() << " API calls" << endl;
        cout << "========================================\n"
             << endl;

        try
        {
            switch (mode)
            {
            case TestMode::ORIGINAL:
                runOriginal(std::move(spec), testSequence);
                break;
            case TestMode::REWRITE_ONLY:
                runRewriteOnly(std::move(spec), testSequence);
                break;
            case TestMode::FULL_PIPELINE:
                runFullPipeline(std::move(spec), testSequence);
                break;
            }
            cout << "\n✓ " << testName << " COMPLETE!\n"
                 << endl;
        }
        catch (const runtime_error &e)
        {
            string errorMsg = e.what();
            // Check if this is an UNSAT error
            if (errorMsg.find("UNSAT") != string::npos)
            {
                cout << "\n⊘ " << testName << " UNSAT: Preconditions not satisfiable\n"
                     << endl;
            }
            else
            {
                cout << "\n✗ " << testName << " FAILED: " << errorMsg << "\n"
                     << endl;
            }
        }
        catch (const exception &e)
        {
            cout << "\n✗ " << testName << " FAILED: " << e.what() << "\n"
                 << endl;
        }
    }

private:
    string getModeString()
    {
        switch (mode)
        {
        case TestMode::ORIGINAL:
            return "Original (No Rewrite)";
        case TestMode::REWRITE_ONLY:
            return "Rewrite Only (No Backend)";
        case TestMode::FULL_PIPELINE:
            return "Full Pipeline (With Backend)";
        }
        return "Unknown";
    }

    void runOriginal(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        Program atc = genATC(*spec, ts);
        PrintVisitor printer;
        printer.visitProgram(atc);
    }

    void runRewriteOnly(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        auto factory = make_unique<RestaurantFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);
    }

    void runFullPipeline(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        SymbolTable *symbolTable = new SymbolTable(nullptr);

        auto factory = make_unique<RestaurantFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);

        vector<Expr *> inputVars;
        ValueEnvironment *valueEnv = new ValueEnvironment(nullptr);

        unique_ptr<Program> ctc = tester.generateCTC(
            std::move(testApiATC),
            inputVars,
            valueEnv);

        if (!ctc)
        {
            cout << "\n[RESULT] UNSAT - Test preconditions cannot be satisfied" << endl;
            delete symbolTable;
            delete valueEnv;
            throw runtime_error("UNSAT: Preconditions not satisfiable");
        }

        cout << "\n[FINAL CTC]" << endl;
        PrintVisitor printer;
        printer.visitProgram(*ctc);

        delete symbolTable;
        delete valueEnv;
    }
};

// ============================================
// ECOMMERCE TEST EXECUTOR
// ============================================

class EcommerceTestExecutor
{
private:
    TestMode mode;
    string backendUrl;

public:
    EcommerceTestExecutor(TestMode m, const string &url = "http://localhost:3000")
        : mode(m), backendUrl(url) {}

    void runTest(
        const string &testName,
        unique_ptr<Spec> spec,
        const vector<string> &testSequence)
    {
        cout << "\n========================================" << endl;
        cout << "TEST: " << testName << endl;
        cout << "MODE: " << getModeString() << endl;
        cout << "DEPTH: " << testSequence.size() << " API calls" << endl;
        cout << "========================================\n"
             << endl;

        try
        {
            switch (mode)
            {
            case TestMode::ORIGINAL:
                runOriginal(std::move(spec), testSequence);
                break;
            case TestMode::REWRITE_ONLY:
                runRewriteOnly(std::move(spec), testSequence);
                break;
            case TestMode::FULL_PIPELINE:
                runFullPipeline(std::move(spec), testSequence);
                break;
            }
            cout << "\n✓ " << testName << " COMPLETE!\n"
                 << endl;
        }
        catch (const runtime_error &e)
        {
            string errorMsg = e.what();
            if (errorMsg.find("UNSAT") != string::npos)
            {
                cout << "\n⊘ " << testName << " UNSAT: Preconditions not satisfiable\n"
                     << endl;
            }
            else
            {
                cout << "\n✗ " << testName << " FAILED: " << errorMsg << "\n"
                     << endl;
            }
        }
        catch (const exception &e)
        {
            cout << "\n✗ " << testName << " FAILED: " << e.what() << "\n"
                 << endl;
        }
    }

private:
    string getModeString()
    {
        switch (mode)
        {
        case TestMode::ORIGINAL:
            return "Original (No Rewrite)";
        case TestMode::REWRITE_ONLY:
            return "Rewrite Only (No Backend)";
        case TestMode::FULL_PIPELINE:
            return "Full Pipeline (With Backend)";
        }
        return "Unknown";
    }

    void runOriginal(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        Program atc = genATC(*spec, ts);
        PrintVisitor printer;
        printer.visitProgram(atc);
    }

    void runRewriteOnly(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        auto factory = make_unique<Ecommerce::EcommerceFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);
    }

    void runFullPipeline(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        SymbolTable *symbolTable = new SymbolTable(nullptr);

        auto factory = make_unique<Ecommerce::EcommerceFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);

        vector<Expr *> inputVars;
        ValueEnvironment *valueEnv = new ValueEnvironment(nullptr);

        unique_ptr<Program> ctc = tester.generateCTC(
            std::move(testApiATC),
            inputVars,
            valueEnv);

        if (!ctc)
        {
            cout << "\n[RESULT] UNSAT - Test preconditions cannot be satisfied" << endl;
            delete symbolTable;
            delete valueEnv;
            throw runtime_error("UNSAT: Preconditions not satisfiable");
        }

        cout << "\n[FINAL CTC]" << endl;
        PrintVisitor printer;
        printer.visitProgram(*ctc);

        delete symbolTable;
        delete valueEnv;
    }
};

// ============================================
// 25 COMPREHENSIVE RESTAURANT TESTS
// ============================================

namespace RestaurantTests
{

    // ========================================
    // DEPTH 1: Single API Call Tests
    // ========================================

    void test01_registerLogin(TestExecutor &executor)
    {
        executor.runTest(
            "Test 01: Register Customer-->Login (Depth=2)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginCustomerOk"});
    }

    void test02_loginFailure(TestExecutor &executor)
    {
        executor.runTest(
            "Test 02: Login Without Registration (Depth=1)",
            makeRestaurantSpec(), {"loginCustomerErr"}
            // {"loginCustomerOk"} // Should fail: user not in U
        );
    }

    void test03_browseOnly(TestExecutor &executor)
    {
        executor.runTest(
            "Test 03: Browse Restaurants (Depth=1)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginWrongPasswordErr"}
            // {"browseRestaurantsOk"} // Public API, no auth needed
        );
    }

    // ========================================
    // DEPTH 2: Two API Call Tests
    // ========================================

    void test04_Login(TestExecutor &executor)
    {
        executor.runTest(
            "Test 04: Login (Depth=1)",
            makeRestaurantSpec(),
            {"loginCustomerOk"}
            // {"loginOk"} // should be UNSAT
        );
    }

    void test05_registerOwnerAndLogin(TestExecutor &executor)
    {
        executor.runTest(
            "Test 05: Register Owner → Login (Depth=2)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginCustomerOk", "addToCartOk"}//unsat
            // {"registerOwnerOk", "loginOk"} // should be SAT
        );
    }

    void test06_registerAgentAndLogin(TestExecutor &executor)
    {
        executor.runTest(
            "Test 06: Register Agent → Login (Depth=2)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "placeOrderOk"}
            // {"registerAgentOk", "loginOk"} // should be SAT
        );
    }

    // ========================================
    // DEPTH 4: Three API Call Tests
    // ========================================

    void test07_loginBrowseView(TestExecutor &executor)
    {
        executor.runTest(
            "Test 07: Registerowner -> Loginowner ->create restaurant-> Browse-> view menu(Depth=5)",
            makeRestaurantSpec(),
            {"registerOwnerOk",     // 1. Owner registers
             "loginOwnerOk",        // 2. Owner logs in
             "createRestaurantOk",  // 3. Owner creates restaurant
             "registerCustomerOk",  // 4. Customer registers
             "loginCustomerOk",     // 5. Customer logs in
             "browseRestaurantsOk", // 6. Customer browses
             "viewMenuOk"}          // should be SAT
        );
    }

    void test08_loginAndAddToCart(TestExecutor &executor)
    {
        executor.runTest(
            "Test 08: Login → Browse → Add to Cart (Depth=4)",
            makeRestaurantSpec(),
            {"registerOwnerOk",     // 1. Owner registers
             "loginOwnerOk",        // 2. Owner logs in
             "createRestaurantOk",  // 3. Owner creates restaurant
             "addMenuItemOk",       // 4. Owner adds menu item  <-- ADD THIS
             "registerCustomerOk",  // 5. Customer registers
             "loginCustomerOk",     // 6. Customer logs in
             "browseRestaurantsOk", // 7. Customer browses
             "viewMenuOk",          // 8. Customer views menu
             "addToCartOk"}         // 9. Customer adds to cart
        );
    }

    void test09_loginAndReview(TestExecutor &executor)
    {
        executor.runTest(
            "Test 09: Login → Browse → Leave Review (Depth=4)",
            makeRestaurantSpec(),
            {"registerOwnerOk",     // 1. Owner registers
             "loginOwnerOk",        // 2. Owner logs in
             "createRestaurantOk",  // 3. Owner creates restaurant
             "addMenuItemOk",       // 4. Owner adds menu item  <-- ADD THIS
             "registerCustomerOk",  // 5. Customer registers
             "loginCustomerOk",     // 6. Customer logs in
             "browseRestaurantsOk", // 7. Customer browses
             "viewMenuOk",          // 8. Customer views menu
             "addToCartOk",
             "placeOrderOk"});
    }

    void test10_reviewWithoutLogin(TestExecutor &executor)
    {
        executor.runTest(
            "Test 10: Browse → Leave Review (No Auth) (Depth=2)",
            makeRestaurantSpec(),
            {"registerOwnerOk",          // 1. Owner registers
             "loginOwnerOk",             // 2. Owner logs in
             "createRestaurantOk",       // 3. Owner creates restaurant
             "addMenuItemOk",            // 4. Owner adds menu item
             "registerCustomerOk",       // 5. Customer registers
             "loginCustomerOk",          // 6. Customer logs in
             "browseRestaurantsOk",      // 7. Customer browses
             "viewMenuOk",               // 8. Customer views menu
             "addToCartOk",              // 9. Customer adds to cart
             "placeOrderOk",             // 10. Customer places order (status: placed)
             "registerAgentOk",          // 11. Delivery agent registers
             "loginAgentOk",             // 12. Delivery agent logs in
             "updateOrderStatusOwnerOk", // 13. Owner accepts order (status: accepted)
             "updateOrderStatusOwnerOk", // 14. Owner marks preparing (status: preparing)
             "updateOrderStatusOwnerOk", // 15. Owner marks ready (status: ready)
             "assignOrderOk",            // 16. Owner assigns delivery agent
             "updateOrderStatusAgentOk", // 17. Agent picks up (status: picked_up)
             "updateOrderStatusAgentOk", // 18. Agent delivers (status: delivered)
             "leaveReviewOk"});
    }

    // ========================================
    // DEPTH 4: Four API Call Tests
    // ========================================

    void test11_fullCustomerOrder(TestExecutor &executor)
    {
        executor.runTest(
            "Test 11: Register → Login → Add to Cart → Place Order (Depth=4)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk"}
            // {"registerCustomerOk", "loginOk", "addToCartOk", "placeOrderOk"} // should fail no browse restaurants
        );
    }

    void test12_ownerCreateRestaurant(TestExecutor &executor)
    {
        executor.runTest(
            "Test 12: Register Owner → Login → Create Restaurant → Add Menu (Depth=4)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk",
             "addMenuItemOk", "addMenuItemOk", "addMenuItemOk"}
            // {"registerOwnerOk", "loginOk", "createRestaurantOk", "addMenuItemOk"}
        );
    }

    void test13_cartWithoutItems(TestExecutor &executor)
    {
        executor.runTest(
            "Test 13: Login → Place Order (Empty Cart) (Depth=2)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartOk"}
            // {"registerCustomerOk", "loginOk", "placeOrderErr"} // Should fail: cart empty
        );
    }

    // ========================================
    // DEPTH 5: Five API Call Tests
    // ========================================

    void test14_customerFullWorkflow(TestExecutor &executor)
    {
        executor.runTest(
            "Test 14: Register → Login → Browse → Add Cart → Order (Depth=5)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartOk", "placeOrderOk"}
            // {"registerCustomerOk", "loginOk", "browseRestaurantsOk", "addToCartOk", "placeOrderOk"}
        );
    }

    void test15_ownerFullSetup(TestExecutor &executor)
    {
        executor.runTest(
            "Test 15: Register Owner → Login → Create Restaurant → Add 2 Menu Items (Depth=5)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartOk", "placeOrderOk",
             "updateOrderStatusOwnerOk", // accepted
             "updateOrderStatusOwnerOk", // preparing
             "updateOrderStatusOwnerOk"}
            // {"registerOwnerOk", "loginOk", "createRestaurantOk", "addMenuItemOk", "addMenuItemOk"}
        );
    }

    void test16_agentAssignOrder(TestExecutor &executor)
    {
        executor.runTest(
            "Test 16: Register Agent → Login → Customer Places Order → Agent Assigned → Update Status (Depth=5)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "registerCustomerOk"}
            //{"registerAgentOk", "loginOk", "placeOrderOk", "assignOrderOk", "updateOrderStatusAgentOk"} // delivery agent can't place order
        );
    }

    // ========================================
    // DEPTH 6: Six API Call Tests
    // ========================================

    void test17_ownerManageOrder(TestExecutor &executor)
    {
        executor.runTest(
            "Test 17: Owner: Register → Login → Create Restaurant → owner tries to place order (Depth=6)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartOk", "placeOrderOk",
             "registerAgentOk", "loginAgentOk",
             "updateOrderStatusAgentOk"}
            //{"registerOwnerOk", "loginOk", "createRestaurantOk", "placeOrderOk"} // should be UNSAT, owner can't place order
        );
    }

    void test18_multipleCartAdditions(TestExecutor &executor)
    {
        executor.runTest(
            "Test 18: Login → Browse → Add 3 Items to Cart → Place Order (Depth=6)",
            makeRestaurantSpec(),
            {"loginCustomerOk", "browseRestaurantsOk", "addToCartOk", "addToCartOk", "addToCartOk", "placeOrderOk"} // should return unsat as no registration
        );
    }

    void test19_wrongRoleAccess(TestExecutor &executor)
    {
        executor.runTest(
            "Test 19: Customer Tries Owner Action (Depth=3)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginCustomerOk", "createRestaurantOk"} // Customer can't create restaurant
        );
    }

    // ========================================
    // DEPTH 7: Seven API Call Tests
    // ========================================

    void test20_fullLifecycle(TestExecutor &executor)
    {
        executor.runTest(
            "Test 20: Full Order Lifecycle (Depth=7)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartOk", "placeOrderOk", "leaveReviewOk"});
    }

    void test21_ownerCompleteFlow(TestExecutor &executor)
    {
        executor.runTest(
            "Test 21: Owner Complete Restaurant Management (Depth=7)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk",
             "addMenuItemOk", "addMenuItemOk", "addMenuItemOk"});
    }

    // ========================================
    // More Tests
    // ========================================

    // void test22_multiItems(TestExecutor &executor)
    // {
    //     executor.runTest(
    //         "Test 22: (Depth=6, SAT)",
    //         makeRestaurantSpec(),
    //         {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk",
    //          "addMenuItemOk", "addMenuItemOk", "addMenuItemOk"});
    // }

    void test23_complexOrderManagement(TestExecutor &executor)
    {
        executor.runTest(
            "Test 23: Complex Order Management with Multiple Updates (Depth=9)",
            makeRestaurantSpec(),
            {"registerOwnerOk",          // 1. Owner registers
             "loginOwnerOk",             // 2. Owner logs in
             "createRestaurantOk",       // 3. Owner creates restaurant
             "addMenuItemOk",            // 4. Owner adds menu item
             "registerCustomerOk",       // 5. Customer registers
             "loginCustomerOk",          // 6. Customer logs in
             "browseRestaurantsOk",      // 7. Customer browses
             "viewMenuOk",               // 8. Customer views menu
             "addToCartOk",              // 9. Customer adds to cart
             "placeOrderOk"});
    }

    void test24_invalidSequence(TestExecutor &executor)
    {
        executor.runTest(
            "Test 24: Invalid Sequence - Review Before Order (Depth=4)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginCustomerOk", "leaveReviewOk"}
            // Should fail: can't review restaurant you haven't ordered from
        );
    }

    void test25_deepWorkflow(TestExecutor &executor)
    {
        executor.runTest(
            "Test 25: Deep Workflow  (Depth=10)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk",
             "addMenuItemOk", "addMenuItemOk", "addMenuItemOk",
             "addMenuItemOk", "addMenuItemOk", "updateOrderStatusOwnerOk", "updateOrderStatusOwnerOk"});
    }

    void test26_registerCustomerDuplicate(TestExecutor &executor)
    {
        executor.runTest(
            "Test 26: Register Customer → Register Again (Duplicate Email) (Depth=2)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "registerCustomerOk"}); // edge case should return unsat
    }
    // void test27_deepCustomerJourney(TestExecutor &executor)
    // {
    //     executor.runTest(
    //         "Test 27: Complete Customer Lifecycle with Multiple Operations (Depth=15)",
    //         makeRestaurantSpec(),
    //         {"registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
    //          "addToCartOk", "addToCartOk", "addToCartOk", "placeOrderOk",
    //          "browseRestaurantsOk", "viewMenuOk", "addToCartOk", "addToCartOk",
    //          "placeOrderOk", "leaveReviewOk", "leaveReviewOk"});
    // }
    // void test28_deepOwnerJourney(TestExecutor &executor)
    // {
    //     executor.runTest(
    //         "Test 28: Complete Owner Restaurant Management (Depth=15)",
    //         makeRestaurantSpec(),
    //         {"registerOwnerOk", "loginOk", "createRestaurantOk",
    //          "addMenuItemOk", "addMenuItemOk", "addMenuItemOk", "addMenuItemOk",
    //          "addMenuItemOk", "addMenuItemOk", "addMenuItemOk", "addMenuItemOk",
    //          "updateOrderStatusOwnerOk", "updateOrderStatusOwnerOk",
    //          "updateOrderStatusOwnerOk", "updateOrderStatusOwnerOk"});
    // }
}

// ============================================
// E-COMMERCE COMPREHENSIVE TESTS (30 tests: 21 SAT, 9 UNSAT)
// ============================================

namespace EcommerceTests
{
    // ╔════════════════════════════════════════════════════════════╗
    // ║  SAT TESTS (21 tests = 70%)                                ║
    // ╚════════════════════════════════════════════════════════════╝

    // SAT-01: Register Buyer (Depth=1)
    void test01_registerBuyer(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 01: Register Buyer (Depth=1)",
            makeEcommerceSpec(),
            {"registerBuyerOk"});
    }

    // SAT-02: Register Seller (Depth=1)
    void test02_registerSeller(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 02: Register Seller (Depth=1)",
            makeEcommerceSpec(),
            {"registerSellerOk"});
    }

    // SAT-03: Browse Products - Public API (Depth=1)
    void test03_browseProducts(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 03: Browse All Products - Public (Depth=1)",
            makeEcommerceSpec(),
            {"getAllProductsOk"});
    }

    // SAT-04: Register Buyer → Login (Depth=2)
    void test04_buyerRegisterLogin(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 04: Register Buyer → Login (Depth=2)",
            makeEcommerceSpec(),
            {"registerBuyerOk", "loginBuyerOk"});
    }

    // SAT-05: Register Seller → Login (Depth=2)
    void test05_sellerRegisterLogin(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 05: Register Seller → Login (Depth=2)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk"});
    }

    // SAT-06: Seller Creates Product (Depth=3)
    void test06_sellerCreateProduct(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 06: Seller Creates Product (Depth=3)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk"});
    }

    // SAT-07: Seller Creates Multiple Products (Depth=5)
    void test07_sellerCreateMultipleProducts(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 07: Seller Creates 3 Products (Depth=5)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk",
             "createProductOk", "createProductOk", "createProductOk"});
    }

    // SAT-08: Seller Creates → Updates Product (Depth=4)
    void test08_sellerUpdateProduct(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 08: Seller Creates → Updates Product (Depth=4)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk", "updateProductOk"});
    }

    // SAT-09: Seller Creates → Deletes Product (Depth=4)
    void test09_sellerDeleteProduct(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 09: Seller Creates → Deletes Product (Depth=4)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk", "deleteProductOk"});
    }

    // SAT-10: Seller Views Own Inventory (Depth=4)
    void test10_sellerViewInventory(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 10: Seller Creates → Views Inventory (Depth=4)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk", "getSellerProductsOk"});
    }

    // SAT-11: Seller creates product → Buyer browses (Depth=6)
    void test11_multiUserBrowse(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 11: Seller Setup → Buyer Browses (Depth=6)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk", "getAllProductsOk"});
    }

    // SAT-12: Seller creates product → Buyer adds to cart (Depth=7)
    void test12_multiUserAddToCart(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 12: Seller Setup → Buyer Adds to Cart (Depth=7)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk"});
    }

    // SAT-13: Seller creates product → Buyer views cart (Depth=8)
    void test13_multiUserViewCart(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 13: Seller Setup → Buyer Adds & Views Cart (Depth=8)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "getCartOk"});
    }

    // SAT-14: Seller creates product → Buyer creates order (Depth=8)
    void test14_multiUserCreateOrder(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 14: Seller Setup → Buyer Creates Order (Depth=8)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk"});
    }

    // SAT-15: Full order flow → Buyer views orders (Depth=9)
    void test15_multiUserViewOrders(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 15: Full Order Flow → Buyer Views Orders (Depth=9)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk", "getBuyerOrdersOk"});
    }

    // SAT-16: Full order flow → Seller views orders (Depth=9)
    void test16_sellerViewsOrders(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 16: Full Order → Seller Views Orders (Depth=9)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk", "getSellerOrdersOk"});
    }

    // SAT-17: Full flow → Buyer creates review (Depth=9)
    void test17_multiUserCreateReview(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 17: Full Order → Buyer Creates Review (Depth=9)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk", "createReviewOk"});
    }

    // SAT-18: Complete E-Commerce Journey (Depth=12)
    void test18_completeEcommerceFlow(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 18: Complete E-Commerce Flow (Depth=12)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "getAllProductsOk", "addToCartOk", "addToCartOk",
             "createOrderOk", "getBuyerOrdersOk", "createReviewOk"});
    }

    // SAT-19: Multiple Orders by Same Buyer (Depth=12)
    void test19_multipleOrders(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 19: Buyer Places Multiple Orders (Depth=12)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk",
             "createProductOk", "createProductOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk",
             "addToCartOk", "createOrderOk",
             "getBuyerOrdersOk"});
    }

    // SAT-20: Seller Full Product Management (Depth=10)
    void test20_sellerFullManagement(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 20: Seller Full Product Management (Depth=10)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk",
             "createProductOk", "createProductOk", "createProductOk",
             "updateProductOk", "updateProductOk",
             "deleteProductOk",
             "getSellerProductsOk", "getSellerOrdersOk"});
    }

    // SAT-21: Deep E-Commerce Workflow (Depth=15)
    void test21_deepWorkflow(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 21: Deep E-Commerce Workflow (Depth=15)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk",
             "createProductOk", "createProductOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "getAllProductsOk", "addToCartOk", "addToCartOk",
             "createOrderOk", "createReviewOk",
             "addToCartOk", "createOrderOk", "createReviewOk"});
    }

    // ╔════════════════════════════════════════════════════════════╗
    // ║  UNSAT TESTS (9 tests = 30%)                               ║
    // ╚════════════════════════════════════════════════════════════╝

    // UNSAT-01: Login without registration (Depth=1)
    void test22_loginWithoutRegister(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 22: Buyer Login Without Registration (Depth=1)",
            makeEcommerceSpec(),
            {"loginBuyerOk"});
    }

    // UNSAT-02: Seller login without registration (Depth=1)
    void test23_sellerLoginWithoutRegister(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 23: Seller Login Without Registration (Depth=1)",
            makeEcommerceSpec(),
            {"loginSellerOk"});
    }

    // UNSAT-03: Duplicate registration (Depth=2)
    void test24_duplicateRegistration(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 24: Duplicate Buyer Registration (Depth=2)",
            makeEcommerceSpec(),
            {"registerBuyerOk", "registerBuyerOk"});
    }

    // UNSAT-04: Buyer tries to create product (Depth=3)
    void test25_buyerCannotCreateProduct(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 25: Buyer Cannot Create Product (Depth=3)",
            makeEcommerceSpec(),
            {"registerBuyerOk", "loginBuyerOk", "createProductOk"});
    }

    // UNSAT-05: Seller tries to add to cart (Depth=3)
    void test26_sellerCannotAddToCart(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 26: Seller Cannot Add to Cart (Depth=3)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "addToCartOk"});
    }

    // UNSAT-06: Seller tries to create order (Depth=4)
    void test27_sellerCannotCreateOrder(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 27: Seller Cannot Create Order (Depth=4)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk", "createOrderOk"});
    }

    // UNSAT-07: Buyer adds to cart without product existing (Depth=3)
    void test28_addToCartNoProduct(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 28: Add to Cart - No Product Exists (Depth=3)",
            makeEcommerceSpec(),
            {"registerBuyerOk", "loginBuyerOk", "addToCartOk"});
    }

    // UNSAT-08: Create order without items in cart (Depth=6)
    void test29_createOrderEmptyCart(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 29: Create Order - Empty Cart (Depth=6)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "createOrderOk"});
    }

    // UNSAT-09: Create review without placing order (Depth=7)
    void test30_reviewWithoutOrder(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 30: Create Review - No Order Placed (Depth=7)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createReviewOk"});
    }
}

// ============================================
// MAIN FUNCTION - TEST SELECTION
// ============================================

int main()
{
    try
    {
        // ========================================
        // CONFIGURATION
        // ========================================

        // Choose test mode
        // TestMode mode = TestMode::REWRITE_ONLY;
        // TestMode mode = TestMode::ORIGINAL;
        TestMode mode = TestMode::FULL_PIPELINE; // Needs backend running!

        // Backend URL (only used for FULL_PIPELINE mode)
        // string backendUrl = "http://localhost:5002"; // for restaurant

        // TestExecutor executor(mode, backendUrl); // for restaurant

        // ========================================
        // RUN TESTS
        // ========================================

        // cout << "\n╔════════════════════════════════════════╗" << endl; // for restaurant
        // cout << "║  TESTGEN - RESTAURANT TEST SUITE      ║" << endl; // for restaurant
        // cout << "║  Total Tests: 25                       ║" << endl; // for restaurant
        // cout << "╚════════════════════════════════════════╝\n" // for restaurant
        //     << endl; // for restaurant

        // QUICK TEST SUBSET (uncomment to run specific tests)
        // RestaurantTests::test01_registerOnly(executor);
        // RestaurantTests::test04_registerAndLogin(executor);

        // RUN ALL 25 TESTS (comment out for selective testing)
        // cout << "\n=== DEPTH TESTS ===" << endl; // for restaurant
        // RestaurantTests::test01_registerLogin(executor);
        // RestaurantTests::test02_loginFailure(executor);
        // RestaurantTests::test03_browseOnly(executor);

        //RestaurantTests::test04_Login(executor);
        // RestaurantTests::test05_registerOwnerAndLogin(executor);
        // RestaurantTests::test06_registerAgentAndLogin(executor);

        // RestaurantTests::test07_loginBrowseView(executor);
        // RestaurantTests::test08_loginAndAddToCart(executor);
        // RestaurantTests::test09_loginAndReview(executor);
        // RestaurantTests::test10_reviewWithoutLogin(executor);

        // RestaurantTests::test11_fullCustomerOrder(executor);
        // RestaurantTests::test12_ownerCreateRestaurant(executor);
        // RestaurantTests::test13_cartWithoutItems(executor);

        // cout << "\n=== DEPTH 5 TESTS ===" << endl;
        // RestaurantTests::test14_customerFullWorkflow(executor);
        // RestaurantTests::test15_ownerFullSetup(executor);
        // RestaurantTests::test16_agentAssignOrder(executor);

        // cout << "\n=== DEPTH 6 TESTS ===" << endl;
        // RestaurantTests::test17_ownerManageOrder(executor);
        // RestaurantTests::test18_multipleCartAdditions(executor);
        // RestaurantTests::test19_wrongRoleAccess(executor);

        // cout << "\n=== DEPTH 7 TESTS ===" << endl;
        // RestaurantTests::test20_fullLifecycle(executor);
        // RestaurantTests::test21_ownerCompleteFlow(executor);

        // cout << "\n===More TESTS ===" << endl;
        // RestaurantTests::test22_multiItems(executor);
        // RestaurantTests::test23_complexOrderManagement(executor);
        // RestaurantTests::test24_invalidSequence(executor);
        // RestaurantTests::test25_deepWorkflow(executor);
        // RestaurantTests::test26_registerCustomerDuplicate(executor);
        // RestaurantTests::test27_deepCustomerJourney(executor);
        // RestaurantTests::test28_deepOwnerJourney(executor);

        // ECOMMERCE TEST CASES
        // ========================================
        // E-COMMERCE TESTS
        // ========================================

        cout << "\n╔════════════════════════════════════════╗" << endl;
        cout << "║  TESTGEN - E-COMMERCE TEST SUITE       ║" << endl;
        cout << "║  Total Tests: 30 (21 SAT, 9 UNSAT)     ║" << endl;
        cout << "╚════════════════════════════════════════╝\n"
             << endl;

        // E-commerce backend URL
        string ecommerceBackendUrl = "http://localhost:3000";
        EcommerceTestExecutor ecommerceExecutor(mode, ecommerceBackendUrl);

        // === SAT TESTS ===
        // EcommerceTests::test01_registerBuyer(ecommerceExecutor);
        // EcommerceTests::test02_registerSeller(ecommerceExecutor);
        // EcommerceTests::test03_browseProducts(ecommerceExecutor);
        // EcommerceTests::test04_buyerRegisterLogin(ecommerceExecutor);
        // EcommerceTests::test05_sellerRegisterLogin(ecommerceExecutor);
        // EcommerceTests::test06_sellerCreateProduct(ecommerceExecutor);
        // EcommerceTests::test07_sellerCreateMultipleProducts(ecommerceExecutor);
        // EcommerceTests::test08_sellerUpdateProduct(ecommerceExecutor);
        // EcommerceTests::test09_sellerDeleteProduct(ecommerceExecutor);
        // EcommerceTests::test10_sellerViewInventory(ecommerceExecutor);
        // EcommerceTests::test11_multiUserBrowse(ecommerceExecutor);
        // EcommerceTests::test12_multiUserAddToCart(ecommerceExecutor);
        // EcommerceTests::test13_multiUserViewCart(ecommerceExecutor);
        // EcommerceTests::test14_multiUserCreateOrder(ecommerceExecutor);
        // EcommerceTests::test15_multiUserViewOrders(ecommerceExecutor);
        // EcommerceTests::test16_sellerViewsOrders(ecommerceExecutor);
        // EcommerceTests::test17_multiUserCreateReview(ecommerceExecutor);
        // EcommerceTests::test18_completeEcommerceFlow(ecommerceExecutor);
        // EcommerceTests::test19_multipleOrders(ecommerceExecutor);
        // EcommerceTests::test20_sellerFullManagement(ecommerceExecutor);
        // EcommerceTests::test21_deepWorkflow(ecommerceExecutor);

        // === UNSAT TESTS ===
        // EcommerceTests::test22_loginWithoutRegister(ecommerceExecutor);
        // EcommerceTests::test23_sellerLoginWithoutRegister(ecommerceExecutor);
        // EcommerceTests::test24_duplicateRegistration(ecommerceExecutor);
        // EcommerceTests::test25_buyerCannotCreateProduct(ecommerceExecutor);
        // EcommerceTests::test26_sellerCannotAddToCart(ecommerceExecutor);
        // EcommerceTests::test27_sellerCannotCreateOrder(ecommerceExecutor);
        // EcommerceTests::test28_addToCartNoProduct(ecommerceExecutor);
        // EcommerceTests::test29_createOrderEmptyCart(ecommerceExecutor);
        // EcommerceTests::test30_reviewWithoutOrder(ecommerceExecutor);

    }
    catch (const exception &e)
    {
        cerr << "\n❌ FATAL ERROR: " << e.what() << endl;
        return 1;
    }

    return 0;
}
