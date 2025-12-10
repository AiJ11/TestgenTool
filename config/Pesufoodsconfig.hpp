#pragma once
#include "../WebAppConfig.hpp"
#include "../ModularTestGen.hpp"
//#include "executors/PesuFoodsExecutor.hpp"

// Forward declarations
#include "../library_apispec_examples/Pesu_foods/1.hpp"
#include "../library_apispec_examples/Pesu_foods/2.hpp"
#include "../library_apispec_examples/Pesu_foods/3.hpp"

/**
 * PESU Foods Application Configuration
 * 
 * Global Variables:
 * - U: (string -> string) map  // Users: username -> password
 * - T: (string -> string) map  // Tokens: token -> username
 * - MenuList: set of menu items (implicit)
 * - MenuMap: (canteen -> menu) map (implicit)
 * - cart: set of cart items (implicit)
 * - orders: (orderId -> cart) map (implicit)
 */
class PesuFoodsAppConfig {
public:
    static WebAppConfig create() {
        return WebAppConfigBuilder("pesu_foods")
            .withDescription("PESU campus food ordering system")
            .withPort(5001) // Set port to 5001
            
            // Core globals
            .addMapGlobal("U", "string", "string")  // username -> password
            .addMapGlobal("T", "string", "string")  // token -> username
            
            // Initialize as empty
            .initializeGlobal("U", "{}")
            .initializeGlobal("T", "{}")
            
            // Functions
            /*.addFunction("register_user", {"string", "string"}, HTTPResponseCode::CREATED_201)
            .addFunction("authenticate", {"string", "string"}, HTTPResponseCode::OK_200, {"string"})
            .addFunction("get_menu", {"string"}, HTTPResponseCode::OK_200)
            .addFunction("add_to_cart", {"string"}, HTTPResponseCode::OK_200)
            .addFunction("remove_from_cart", {"string"}, HTTPResponseCode::OK_200)
            .addFunction("place_order", {"string"}, HTTPResponseCode::CREATED_201)*/

            // Signup: POST /signup
            .addFunction("signup", 
                {"string", "string", "string"},     // name, usermail, password
                HTTPResponseCode::CREATED_201,
                {}                                   // No return value
            )
            
            // Login: POST /login
            .addFunction("login", 
                {"string", "string"},               // usermail, password
                HTTPResponseCode::OK_200,
                {"string"}                          // returns JWT token
            )
            
            // Get Account: GET /account
            .addFunction("getAccount", 
                {"string"},                         // token
                HTTPResponseCode::OK_200,
                {"string"}                          // returns user JSON
            )
            
            // Test API (getters/setters for global state)
            .addTestAPI("U", "get_U", "set_U")
            .addTestAPI("T", "get_T", "set_T")
            
            // Test Scenarios
            /*.addTestScenario(
                "pesu_foods_scenario_1",
                "Register -> Authenticate -> Add to Cart -> Place Order",
                {"register_user", "authenticate", "add_to_cart", "place_order"},
                "Pesu_foods_example1"
            )
            .addTestScenario(
                "pesu_foods_scenario_2",
                "Register -> Authenticate -> Add to Cart -> Remove from Cart",
                {"register_user", "authenticate", "add_to_cart", "remove_from_cart"},
                "Pesu_foods_example2"
            )
            .addTestScenario(
                "pesu_foods_scenario_3",
                "Register -> Authenticate -> Get Menu -> Add to Cart -> Place Order",
                {"register_user", "authenticate", "get_menu", "add_to_cart", "place_order"},
                "Pesu_foods_example3"
            )*/
           // Scenario 1: Login without signup (UNSAT - infeasible)
            .addTestScenario(
                "pesu_foods_scenario_1",
                "Login without signup (should be UNSAT)",
                {"login"},
                "Pesu_foods_example1"
            )
            
            // Scenario 2: Signup → Login → GetAccount (full flow)
            .addTestScenario(
                "pesu_foods_scenario_2",
                "Complete auth flow: Signup → Login → GetAccount",
                {"signup", "login", "getAccount"},
                "Pesu_foods_example2"
            )

            .build();
    }
};

// Register all pesu_foods builders
REGISTER_API_BUILDER(Pesu_foods_example1, Pesu_foods_example1);
REGISTER_API_BUILDER(Pesu_foods_example2, Pesu_foods_example2);
REGISTER_API_BUILDER(Pesu_foods_example3, Pesu_foods_example3);
