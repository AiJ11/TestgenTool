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
#include "see/see.hh" 

// Import webapp-specific specs
#include "specs/RestaurantSpec.hpp"

using namespace std;

// ============================================
// TEST EXECUTION MODES
// ============================================

enum class TestMode {
    ORIGINAL,           // Just genATC (no rewrite)
    REWRITE_ONLY,      // genATC + RewriteGlobalsVisitor (no backend)
    FULL_PIPELINE      // Complete: genATC + Rewrite + SEE + Backend
};

// ============================================
// TEST EXECUTOR
// ============================================

class TestExecutor {
private:
    TestMode mode;
    string backendUrl;
    
public:
    TestExecutor(TestMode m, const string& url = "http://localhost:5002") 
        : mode(m), backendUrl(url) {}
    
    void runTest(
        const string& testName,
        unique_ptr<Spec> spec,
        const vector<string>& testSequence
    ) {
        cout << "\n========================================" << endl;
        cout << "TEST: " << testName << endl;
        cout << "MODE: " << getModeString() << endl;
        cout << "DEPTH: " << testSequence.size() << " API calls" << endl;
        cout << "========================================\n" << endl;
        
        try {
            switch (mode) {
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
            cout << "\n✓ " << testName << " COMPLETE!\n" << endl;
        } catch (const exception& e) {
            cout << "\n✗ " << testName << " FAILED: " << e.what() << "\n" << endl;
        }
    }
    
private:
    string getModeString() {
        switch (mode) {
            case TestMode::ORIGINAL: return "Original (No Rewrite)";
            case TestMode::REWRITE_ONLY: return "Rewrite Only (No Backend)";
            case TestMode::FULL_PIPELINE: return "Full Pipeline (With Backend)";
        }
        return "Unknown";
    }
    
    void runOriginal(unique_ptr<Spec> spec, const vector<string>& ts) {
        Program atc = genATC(*spec, ts);
        PrintVisitor printer;
        printer.visitProgram(atc);
    }
    
    void runRewriteOnly(unique_ptr<Spec> spec, const vector<string>& ts) {
        auto factory = make_unique<RestaurantFunctionFactory>(backendUrl);
        Tester tester(factory.get());
        
        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);
    }
    
    void runFullPipeline(unique_ptr<Spec> spec, const vector<string>& ts) {
        SymbolTable* symbolTable = new SymbolTable(nullptr);  
        
        auto factory = make_unique<RestaurantFunctionFactory>(backendUrl);
        Tester tester(factory.get());
        
        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);
        
        vector<Expr*> inputVars;
        ValueEnvironment* valueEnv = new ValueEnvironment(nullptr);
        
        unique_ptr<Program> ctc = tester.generateCTC(
            std::move(testApiATC),
            inputVars,
            valueEnv
        );
        
        if (ctc) {
            cout << "\n[FINAL CTC]" << endl;
            PrintVisitor printer;
            printer.visitProgram(*ctc);
        }
        
        delete symbolTable;
        delete valueEnv;
    }
};

// ============================================
// 25 COMPREHENSIVE RESTAURANT TESTS
// ============================================

namespace RestaurantTests {
    
    // ========================================
    // DEPTH 1: Single API Call Tests
    // ========================================
    
    void test01_registerOnly(TestExecutor& executor) {
        executor.runTest(
            "Test 01: Register Customer Only (Depth=1, SAT)",
            makeRestaurantSpec(),
            {"registerCustomerOk"}
        );
    }
    
    void test02_loginFailure(TestExecutor& executor) {
        executor.runTest(
            "Test 02: Login Without Registration (Depth=1)",
            makeRestaurantSpec(),
            {"loginErr"}  // Should fail: user not in U
        );
    }
    
    void test03_browseOnly(TestExecutor& executor) {
        executor.runTest(
            "Test 03: Browse Restaurants (Depth=1)",
            makeRestaurantSpec(),
            {"browseRestaurantsOk"}  // Public API, no auth needed
        );
    }

    // ========================================
    // DEPTH 2: Two API Call Tests
    // ========================================
    
    void test04_registerAndLogin(TestExecutor& executor) {
        executor.runTest(
            "Test 04: Register → Login (Depth=2)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginOk"} // should be SAT
        );
    }
    
    void test05_registerOwnerAndLogin(TestExecutor& executor) {
        executor.runTest(
            "Test 05: Register Owner → Login (Depth=2)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOk"} // should be SAT
        );
    }
    
    void test06_registerAgentAndLogin(TestExecutor& executor) {
        executor.runTest(
            "Test 06: Register Agent → Login (Depth=2)",
            makeRestaurantSpec(),
            {"registerAgentOk", "loginOk"} // should be SAT
        );
    }

    // ========================================
    // DEPTH 4: Three API Call Tests
    // ========================================
    
    void test07_loginBrowseView(TestExecutor& executor) {
        executor.runTest(
            "Test 07: Login → Browse → View Menu (Depth=4)",
            makeRestaurantSpec(),
            {"registerCustomerOk","loginOk", "browseRestaurantsOk", "viewMenuOk"} //should be SAT
        );
    }
    
    void test08_loginAndAddToCart(TestExecutor& executor) {
        executor.runTest(
            "Test 08: Login → Browse → Add to Cart (Depth=4)",
            makeRestaurantSpec(),
            {"registerCustomerOk","loginOk", "browseRestaurantsOk", "addToCartOk"}
        );
    }
    
    void test09_loginAndReview(TestExecutor& executor) {
        executor.runTest(
            "Test 09: Login → Browse → Leave Review (Depth=4)",
            makeRestaurantSpec(),
            {"registerCustomerOk","loginOk", "browseRestaurantsOk", "leaveReviewOk"}
        );
    }
    
    void test10_reviewWithoutLogin(TestExecutor& executor) {
        executor.runTest(
            "Test 10: Browse → Leave Review (No Auth) (Depth=2)",
            makeRestaurantSpec(),
            {"browseRestaurantsOk", "leaveReviewErr"}  // Should fail: no token
        );
    }

    // ========================================
    // DEPTH 4: Four API Call Tests
    // ========================================
    
    void test11_fullCustomerOrder(TestExecutor& executor) {
        executor.runTest(
            "Test 11: Register → Login → Add to Cart → Place Order (Depth=4)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginOk", "addToCartOk", "placeOrderOk"}// should fail no browse restaurants
        );
    }
    
    void test12_ownerCreateRestaurant(TestExecutor& executor) {
        executor.runTest(
            "Test 12: Register Owner → Login → Create Restaurant → Add Menu (Depth=4)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOk", "createRestaurantOk", "addMenuItemOk"}
        );
    }
    
    void test13_cartWithoutItems(TestExecutor& executor) {
        executor.runTest(
            "Test 13: Login → Place Order (Empty Cart) (Depth=2)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginOk", "placeOrderErr"}  // Should fail: cart empty
        );
    }

    // ========================================
    // DEPTH 5: Five API Call Tests
    // ========================================
    
    void test14_customerFullWorkflow(TestExecutor& executor) {
        executor.runTest(
            "Test 14: Register → Login → Browse → Add Cart → Order (Depth=5)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginOk", "browseRestaurantsOk", "addToCartOk", "placeOrderOk"}
        );
    }
    
    void test15_ownerFullSetup(TestExecutor& executor) {
        executor.runTest(
            "Test 15: Register Owner → Login → Create Restaurant → Add 2 Menu Items (Depth=5)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOk", "createRestaurantOk", "addMenuItemOk", "addMenuItemOk"}
        );
    }
    
    void test16_agentAssignOrder(TestExecutor& executor) {
        executor.runTest(
            "Test 16: Register Agent → Login → Customer Places Order → Agent Assigned → Update Status (Depth=5)",
            makeRestaurantSpec(),
            {"registerAgentOk", "loginOk", "placeOrderOk", "assignOrderOk", "updateOrderStatusAgentOk"}//delivery agent can't place order
        );
    }

    // ========================================
    // DEPTH 6: Six API Call Tests
    // ========================================
    
    void test17_ownerManageOrder(TestExecutor& executor) {
        executor.runTest(
            "Test 17: Owner: Register → Login → Create Restaurant → owner tries to place order (Depth=6)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOk", "createRestaurantOk", "placeOrderOk"} // should be UNSAT, owner can't place order
        );
    }
    
    void test18_multipleCartAdditions(TestExecutor& executor) {
        executor.runTest(
            "Test 18: Login → Browse → Add 3 Items to Cart → Place Order (Depth=6)",
            makeRestaurantSpec(),
            {"loginOk", "browseRestaurantsOk", "addToCartOk", "addToCartOk", "addToCartOk", "placeOrderOk"}// should return unsat as no registration
        );
    }
    
    void test19_wrongRoleAccess(TestExecutor& executor) {
        executor.runTest(
            "Test 19: Customer Tries Owner Action (Depth=3)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginOk", "createRestaurantErr"}  // Customer can't create restaurant
        );
    }

    // ========================================
    // DEPTH 7: Seven API Call Tests
    // ========================================
    
    void test20_fullLifecycle(TestExecutor& executor) {
        executor.runTest(
            "Test 20: Full Order Lifecycle (Depth=7)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginOk", "browseRestaurantsOk", "viewMenuOk", 
             "addToCartOk", "placeOrderOk", "leaveReviewOk"}
        );
    }
    
    void test21_ownerCompleteFlow(TestExecutor& executor) {
        executor.runTest(
            "Test 21: Owner Complete Restaurant Management (Depth=7)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOk", "createRestaurantOk", 
             "addMenuItemOk", "addMenuItemOk", "addMenuItemOk", "updateOrderStatusOwnerOk"}
        );
    } 

    // ========================================
    // More Tests
    // ========================================
    
    void test22_multiItems(TestExecutor& executor) {
        executor.runTest(
            "Test 22: (Depth=6, SAT)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOk", "createRestaurantOk", 
         "addMenuItemOk", "addMenuItemOk", "addMenuItemOk"}
        );
    }
    
    void test23_complexOrderManagement(TestExecutor& executor) {
        executor.runTest(
            "Test 23: Complex Order Management with Multiple Updates (Depth=9)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginOk", "browseRestaurantsOk", "viewMenuOk",
         "addToCartOk", "addToCartOk", "placeOrderOk", "leaveReviewOk", "leaveReviewOk"}
        );
    }
    
    void test24_invalidSequence(TestExecutor& executor) {
        executor.runTest(
            "Test 24: Invalid Sequence - Review Before Order (Depth=4)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginOk", "leaveReviewErr", "placeOrderOk"}
            // Should fail: can't review restaurant you haven't ordered from
        );
    }
    
    void test25_deepWorkflow(TestExecutor& executor) {
        executor.runTest(
            "Test 25: Deep Workflow  (Depth=10)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOk", "createRestaurantOk", 
         "addMenuItemOk", "addMenuItemOk", "addMenuItemOk",
         "addMenuItemOk", "addMenuItemOk", "updateOrderStatusOwnerOk", "updateOrderStatusOwnerOk"}
        );
    }

    void test26_registerCustomerDuplicate(TestExecutor& executor) {
    executor.runTest(
        "Test 26: Register Customer → Register Again (Duplicate Email) (Depth=2)",
        makeRestaurantSpec(),
        {"registerCustomerOk", "registerCustomerErr"}
    );
}
void test27_deepCustomerJourney(TestExecutor& executor) {
    executor.runTest(
        "Test 27: Complete Customer Lifecycle with Multiple Operations (Depth=15)",
        makeRestaurantSpec(),
        {"registerCustomerOk", "loginOk", "browseRestaurantsOk", "viewMenuOk",
         "addToCartOk", "addToCartOk", "addToCartOk", "placeOrderOk",
         "browseRestaurantsOk", "viewMenuOk", "addToCartOk", "addToCartOk",
         "placeOrderOk", "leaveReviewOk", "leaveReviewOk"}
    );
}
void test28_deepOwnerJourney(TestExecutor& executor) {
    executor.runTest(
        "Test 28: Complete Owner Restaurant Management (Depth=15)",
        makeRestaurantSpec(),
        {"registerOwnerOk", "loginOk", "createRestaurantOk", 
         "addMenuItemOk", "addMenuItemOk", "addMenuItemOk", "addMenuItemOk",
         "addMenuItemOk", "addMenuItemOk", "addMenuItemOk", "addMenuItemOk",
         "updateOrderStatusOwnerOk", "updateOrderStatusOwnerOk", 
         "updateOrderStatusOwnerOk", "updateOrderStatusOwnerOk"}
    );
}
}

// ============================================
// MAIN FUNCTION - TEST SELECTION
// ============================================

int main() {
    try {
        // ========================================
        // CONFIGURATION
        // ========================================
        
        // Choose test mode
        //TestMode mode = TestMode::REWRITE_ONLY;
        // TestMode mode = TestMode::ORIGINAL;
         TestMode mode = TestMode::FULL_PIPELINE;  // Needs backend running!
        
        // Backend URL (only used for FULL_PIPELINE mode)
        string backendUrl = "http://localhost:5002";
        
        TestExecutor executor(mode, backendUrl);
        
        // ========================================
        // RUN TESTS
        // ========================================
        
        cout << "\n╔════════════════════════════════════════╗" << endl;
        cout << "║  TESTGEN - RESTAURANT TEST SUITE      ║" << endl;
        cout << "║  Total Tests: 25                       ║" << endl;
        cout << "╚════════════════════════════════════════╝\n" << endl;
        
        // QUICK TEST SUBSET (uncomment to run specific tests)
        // RestaurantTests::test01_registerOnly(executor);
        // RestaurantTests::test04_registerAndLogin(executor);
        
        // RUN ALL 25 TESTS (comment out for selective testing)
        cout << "\n=== DEPTH 1 TESTS ===" << endl;
        //RestaurantTests::test01_registerOnly(executor);
        //RestaurantTests::test02_loginFailure(executor);
        //RestaurantTests::test03_browseOnly(executor);
        
        //cout << "\n=== DEPTH 2 TESTS ===" << endl;
        RestaurantTests::test04_registerAndLogin(executor);
        //RestaurantTests::test05_registerOwnerAndLogin(executor);
        //RestaurantTests::test06_registerAgentAndLogin(executor);
        
        //cout << "\n=== DEPTH 3 TESTS ===" << endl;
        //RestaurantTests::test07_loginBrowseView(executor);
        //RestaurantTests::test08_loginAndAddToCart(executor);
        //RestaurantTests::test09_loginAndReview(executor);
        //RestaurantTests::test10_reviewWithoutLogin(executor);
        
        //cout << "\n=== DEPTH 4 TESTS ===" << endl;
        //RestaurantTests::test11_fullCustomerOrder(executor);
        //RestaurantTests::test12_ownerCreateRestaurant(executor);
        //RestaurantTests::test13_cartWithoutItems(executor);
        
        //cout << "\n=== DEPTH 5 TESTS ===" << endl;
        //RestaurantTests::test14_customerFullWorkflow(executor);
        //RestaurantTests::test15_ownerFullSetup(executor);
        //RestaurantTests::test16_agentAssignOrder(executor);
        
        //cout << "\n=== DEPTH 6 TESTS ===" << endl;
        //RestaurantTests::test17_ownerManageOrder(executor);
        //RestaurantTests::test18_multipleCartAdditions(executor);
        //RestaurantTests::test19_wrongRoleAccess(executor);
        
        //cout << "\n=== DEPTH 7 TESTS ===" << endl;
        //RestaurantTests::test20_fullLifecycle(executor);
        //RestaurantTests::test21_ownerCompleteFlow(executor);
        
        //cout << "\n===More TESTS ===" << endl;
        //RestaurantTests::test22_multiItems(executor);
        //RestaurantTests::test23_complexOrderManagement(executor);
        //RestaurantTests::test24_invalidSequence(executor);
        //RestaurantTests::test25_deepWorkflow(executor);
        //RestaurantTests::test26_registerCustomerDuplicate(executor);
        //RestaurantTests::test27_deepCustomerJourney(executor);
        //RestaurantTests::test28_deepOwnerJourney(executor);
        
        
    } catch (const exception& e) {
        cerr << "\n❌ FATAL ERROR: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
