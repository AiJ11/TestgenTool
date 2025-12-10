#pragma once
#include "../WebAppConfig.hpp"
#include "../ModularTestGen.hpp"

// Include test builders (tourism_example1, 2, 3, 4)
#include "../library_apispec_examples/tourism/1.hpp"
#include "../library_apispec_examples/tourism/2.hpp"
#include "../library_apispec_examples/tourism/3.hpp"
#include "../library_apispec_examples/tourism/4.hpp"

/**
 * Tourism Web Application Configuration
 * 
 * Globals:
 *  - U : (string → string) map     // userID → password
 *  - T : (string → string) map     // token → userID
 *  - tours : set / map (handled inside test builders)
 *  - cart, wishlist, orders (handled inside test builders)
 */
class TourismAppConfig {
public:
    static WebAppConfig create() {
        return WebAppConfigBuilder("tourism")
            .withDescription("Tourism booking and travel management application")

            // ----- GLOBAL VARIABLES -----
            // Core required globals
            .addMapGlobal("U", "string", "string")       // username -> password
            .addMapGlobal("T", "string", "string")       // token -> username

            // Initialize globals as empty structures
            .initializeGlobal("U", "{}")
            .initializeGlobal("T", "{}")

            // ----- FUNCTION SIGNATURES -----
            .addFunction("register_user", {"string", "string"}, HTTPResponseCode::CREATED_201)
            .addFunction("login", {"string", "string"}, HTTPResponseCode::OK_200, {"string"})
            .addFunction("browseTours", {"string"}, HTTPResponseCode::OK_200)
            .addFunction("selectTour", {"string", "string"}, HTTPResponseCode::OK_200)
            .addFunction("addToCart", {"string", "string"}, HTTPResponseCode::OK_200)
            .addFunction("placeOrder", {"string"}, HTTPResponseCode::CREATED_201)
            .addFunction("addToWishlist", {"string", "string"}, HTTPResponseCode::OK_200)
            .addFunction("removeFromWishlist", {"string", "string"}, HTTPResponseCode::OK_200)
            .addFunction("applyCoupon", {"string", "string"}, HTTPResponseCode::OK_200)

            // ----- TEST APIS (optional, for debugging global state) -----
            .addTestAPI("U", "get_U", "set_U")
            .addTestAPI("T", "get_T", "set_T")

            // ----- TEST SCENARIOS -----

            .addTestScenario(
                "tourism_scenario_1",
                "Login -> Browse -> Select Tour -> Add to Cart -> Place Order",
                {"login", "browseTours", "selectTour", "addToCart", "placeOrder"},
                "tourism_example1"
            )

            .addTestScenario(
                "tourism_scenario_2",
                "Register -> Login -> Add to Wishlist -> Remove from Wishlist",
                {"register_user", "login", "addToWishlist", "removeFromWishlist"},
                "tourism_example2"
            )

            .addTestScenario(
                "tourism_scenario_3",
                "Login -> Browse Tours -> Apply Coupon -> Place Order",
                {"login", "browseTours", "applyCoupon", "placeOrder"},
                "tourism_example3"
            )

            .addTestScenario(
                "tourism_scenario_4",
                "Register -> Login -> Select Tour -> Apply Coupon -> Add to Cart -> Place Order",
                {"register_user", "login", "selectTour", "applyCoupon", "addToCart", "placeOrder"},
                "tourism_example4"
            )

            .build();
    }
};

// Register all tourism example builders
REGISTER_API_BUILDER(tourism_example1, tourism_example1);
REGISTER_API_BUILDER(tourism_example2, tourism_example2);
REGISTER_API_BUILDER(tourism_example3, tourism_example3);
REGISTER_API_BUILDER(tourism_example4, tourism_example4);
