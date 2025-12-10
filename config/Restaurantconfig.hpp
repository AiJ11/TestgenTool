#pragma once
#include "../WebAppConfig.hpp"
#include "../ModularTestGen.hpp"

// Forward declarations for test scenarios
#include "../library_apispec_examples/restaurant/1.hpp"
#include "../library_apispec_examples/restaurant/2.hpp"
#include "../library_apispec_examples/restaurant/3.hpp"  
#include "../library_apispec_examples/restaurant/4.hpp"  
#include "../library_apispec_examples/restaurant/5.hpp"  
#include "../library_apispec_examples/restaurant/6.hpp" 
#include "../library_apispec_examples/restaurant/7.hpp" 
#include "../library_apispec_examples/restaurant/8.hpp"
#include "../library_apispec_examples/restaurant/9.hpp"
#include "../library_apispec_examples/restaurant/10.hpp"   

/**
 * Restaurant Delivery Application Configuration
 * Based on Swagger API Documentation
 * 
 * Server: http://localhost:5002 (actual deployment port)
 * API Docs: http://localhost:5000/api-docs (Swagger default port)
 * 
 * Backend Stack:
 * - MongoDB with Mongoose (Users, Restaurants, MenuItems, Carts, Orders, Reviews, Agents)
 * - JWT Authentication (stateless)
 * - Express.js REST API
 * 
 * Logical Global Variables (for TestGen specification layer):
 * - U: (email -> user) map
 * - T: (email -> token) map (logical auth state)
 * - R: (restaurantId -> restaurant) map
 * - M: (menuItemId -> menuItem) map
 * - C: (userId -> cart) map
 * - O: (orderId -> order) map
 * - Rev: (reviewId -> review) map
 * 
 * IMPORTANT: Backend has NO /test/* endpoints!
 * - No state reset capability
 * - No state inspection endpoints
 * - Consider adding custom test routes if needed
 */
class RestaurantAppConfig {
public:
    static WebAppConfig create() {
        return WebAppConfigBuilder("restaurant")
            .withDescription("Restaurant food delivery system (Swiggy/Zomato clone)")
            .withPort(5002)  // Actual deployment port: 5002
            
            // Logical globals for TestGen spec
            .addMapGlobal("U", "string", "string")  // email -> password
            .addMapGlobal("T", "string", "string")  // email -> token
            
            .initializeGlobal("U", "{}")
            .initializeGlobal("T", "{}")
            
            // ========================================
            // AUTH FUNCTIONS
            // ========================================
            
            .addFunction("registerUser", 
                {"string", "string", "string", "string", "string"},  // fullName, email, password, mobile, role
                HTTPResponseCode::CREATED_201,  // Confirmed: 201
                {}
            )
            
            .addFunction("login", 
                {"string", "string"},                                // email, password
                HTTPResponseCode::OK_200,                            // Confirmed: 200
                {"string"}                                           // returns token
            )
            
            // ========================================
            // USER FUNCTIONS
            // ========================================
            
            .addFunction("getUserProfile",
                {},                                                  // no parameters
                HTTPResponseCode::OK_200,
                {"string"}                                           // returns user JSON
            )
            
            .addFunction("updateUserProfile",
                {"string"},                                          // update JSON
                HTTPResponseCode::OK_200,
                {}
            )
            
            // ========================================
            // RESTAURANT FUNCTIONS
            // ========================================
            
            .addFunction("browseRestaurants", 
                {},                                                  // optional query params not in signature
                HTTPResponseCode::OK_200,
                {"string"}                                           // returns restaurant list JSON
            )
            
            .addFunction("getRestaurant",
                {"string"},                                          // restaurantId
                HTTPResponseCode::OK_200,
                {"string"}                                           // returns restaurant JSON
            )
            
            .addFunction("viewMenu", 
                {"string"},                                          // restaurantId
                HTTPResponseCode::OK_200,
                {"string"}                                           // returns menu items JSON
            )
            
            // ========================================
            // CART FUNCTIONS
            // ========================================
            
            .addFunction("viewCart", 
                {},                                                  // no parameters
                HTTPResponseCode::OK_200,
                {"string"}                                           // returns cart JSON
            )
            
            .addFunction("addToCart", 
                {"string", "string", "string"},                      // restaurantId, menuItemId, quantity
                HTTPResponseCode::OK_200,                            // Confirmed: 200
                {}
            )
            
            .addFunction("updateCartItem",
                {"string", "string"},                                // itemId, quantity
                HTTPResponseCode::OK_200,
                {}
            )
            
            .addFunction("removeFromCart",
                {"string"},                                          // itemId
                HTTPResponseCode::OK_200,
                {}
            )
            
            .addFunction("clearCart",
                {},                                                  // no parameters
                HTTPResponseCode::OK_200,
                {}
            )
            
            // ========================================
            // ORDER FUNCTIONS
            // ========================================
            
            .addFunction("placeOrder", 
                {},                                                  // optional: deliveryAddress, instructions
                HTTPResponseCode::CREATED_201,                       // Confirmed: 201
                {"string"}                                           // returns order JSON
            )
            
            .addFunction("viewOrders", 
                {},                                                  // no parameters
                HTTPResponseCode::OK_200,
                {"string"}                                           // returns orders array JSON
            )
            
            .addFunction("getOrder",
                {"string"},                                          // orderId
                HTTPResponseCode::OK_200,
                {"string"}                                           // returns order JSON
            )
            
            // ========================================
            // REVIEW FUNCTIONS
            // ========================================
            
            .addFunction("createReview", 
                {"string", "string", "string", "string"},            // orderId, restaurantId, rating, comment
                HTTPResponseCode::CREATED_201,                       // Confirmed: 201
                {}
            )
            
            .addFunction("getRestaurantReviews",
                {"string"},                                          // restaurantId
                HTTPResponseCode::OK_200,
                {"string"}                                           // returns reviews array JSON
            )
            
            // ========================================
            // TEST API STUBS (backend has no test endpoints)
            // ========================================
            .addTestAPI("U", "get_U", "set_U")
            .addTestAPI("T", "get_T", "set_T")
            
            // ========================================
            // TEST SCENARIOS
            // ========================================
            
            // Scenario 1: Full registration and order flow
            .addTestScenario(
                "restaurant_scenario_1",
                "Complete flow: Register -> Login -> Browse -> Menu -> Cart -> Order",
                {"registerUser", "login", "browseRestaurants", "viewMenu", 
                 "addToCart", "placeOrder", "viewOrders"},
                "restaurant_example1"
            )
            
            // Scenario 2: Login and order with review
            .addTestScenario(
                "restaurant_scenario_2",
                "Login -> Browse -> Menu -> Cart -> Order -> Review",
                {"login", "browseRestaurants", "viewMenu", "addToCart", 
                 "viewCart", "placeOrder", "createReview"},
                "restaurant_example2"
            )

            // Scenario 3: Register, browse, order, and review
            .addTestScenario(
                "restaurant_scenario_3",
                "Register -> Login -> Browse -> Menu -> Cart -> Order -> Review",
                {"registerUser", "login", "browseRestaurants", "viewMenu", 
                 "addToCart", "placeOrder", "viewOrders", "createReview"},
                "restaurant_example3"
            )
            
            // Scenario 4: Cart management - add, remove, add different item
            .addTestScenario(
                "restaurant_scenario_4",
                "Login -> Browse -> Menu -> Add -> Remove -> Add Different -> Order",
                {"login", "browseRestaurants", "viewMenu", "addToCart", 
                 "removeFromCart", "addToCart", "placeOrder"},
                "restaurant_example4"
            )
            
            // Scenario 5: Simple order from existing cart
            .addTestScenario(
                "restaurant_scenario_5",
                "Login -> View Cart -> Place Order -> View Orders",
                {"login", "viewCart", "placeOrder", "viewOrders"},
                "restaurant_example5"
            )

            // Scenario 6: Owner flow - create restaurant, add menu, manage orders
            .addTestScenario(
                "restaurant_scenario_6",
                "Login -> Create Restaurant -> Add Menu Item -> View Orders -> Assign Order -> Update Status",
                {"login", "createRestaurant", "addMenuItem", "viewOrders", 
                 "assignOrder", "updateOrderStatus"},
                "restaurant_example6"
            )
            
            // Scenario 7: Owner update restaurant
            .addTestScenario(
                "restaurant_scenario_7",
                "Login -> Update Restaurant -> View Orders",
                {"login", "updateRestaurant", "viewOrders"},
                "restaurant_example7"
            )
            
            // Scenario 8: Owner add multiple menu items
            .addTestScenario(
                "restaurant_scenario_8",
                "Login -> Create Restaurant -> Add Menu Item -> Add Menu Item -> View Orders",
                {"login", "createRestaurant", "addMenuItem", "addMenuItem", "viewOrders"},
                "restaurant_example8"
            )
            
            // Scenario 9: Owner assign order to agent
            .addTestScenario(
                "restaurant_scenario_9",
                "Login -> View Orders -> Assign Order",
                {"login", "viewOrders", "assignOrder"},
                "restaurant_example9"
            )
            
            // Scenario 10: Owner delete menu item
            .addTestScenario(
                "restaurant_scenario_10",
                "Login -> Delete Menu Item",
                {"login", "deleteMenuItem"},
                "restaurant_example10"
            )
            
            .build();
    }
};

// Register builders
REGISTER_API_BUILDER(restaurant_example1, restaurant_example1);
REGISTER_API_BUILDER(restaurant_example2, restaurant_example2);
REGISTER_API_BUILDER(restaurant_example3, restaurant_example3);  
REGISTER_API_BUILDER(restaurant_example4, restaurant_example4);  
REGISTER_API_BUILDER(restaurant_example5, restaurant_example5);  
REGISTER_API_BUILDER(restaurant_example6, restaurant_example6);
REGISTER_API_BUILDER(restaurant_example7, restaurant_example7);
REGISTER_API_BUILDER(restaurant_example8, restaurant_example8);
