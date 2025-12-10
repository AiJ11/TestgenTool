/**
 * restaurant_api.cpp
 * ==================
 * API Function implementations for Restaurant Backend
 * 
 * Location: config/restaurant/api.cpp
 * 
 * This file defines all APIFunction classes for the Restaurant backend
 * and provides the factory to create them.
 * 
 * IMPORTANT: All execute() methods transform short Z3 placeholder values
 * (like "A", "B", "C") into realistic values that pass backend validation.
 */

#include "../../APIFunction.hpp"
#include "../executors/RestaurantExecutor.hpp"
#include <iostream>
#include <string>
#include <memory>
#include <set>

// ========================================
// REALISTIC VALUE TRANSFORMER
// ========================================

/**
 * Transform Z3's simple symbolic values into realistic test values
 * that will pass backend validation.
 * 
 * This function is called during API execution to ensure values like
 * "A", "B", "C" are converted to proper emails, passwords, etc.
 */
static std::string transformValue(const std::string& value, const std::string& paramType) {
    // If value is already realistic (longer than 2 chars), return as-is
    if (value.length() > 2) {
        return value;
    }
    
    // Static counters for unique values
    static int itemCounter = 0;
    static int restaurantCounter = 0;
    
    // Transform based on parameter type
    
    // === OWNER PARAMETERS ===
    if (paramType == "ownerEmail") {
        return "owner@test.com";
    }
    if (paramType == "ownerRole") {
        return "restaurant_owner";
    }
    if (paramType == "ownerName") {
        return "TestOwner";
    }
    if (paramType == "ownerMobile") {
        return "8123456789";
    }
    
    // === CUSTOMER PARAMETERS ===
    if (paramType == "email" || paramType == "customerEmail") {
        return "customer@test.com";
    }
    if (paramType == "password") {
        return "Pass123!";
    }
    if (paramType == "fullName" || paramType == "name" || paramType == "customerName") {
        return "TestCustomer";
    }
    if (paramType == "mobile" || paramType == "customerMobile") {
        return "9123456789";
    }
    if (paramType == "role" || paramType == "customerRole") {
        return "customer";
    }
    
    // === RESTAURANT PARAMETERS ===
    if (paramType == "restaurantName") {
        return "TestRestaurant_" + std::to_string(restaurantCounter++);
    }
    if (paramType == "restaurantId") {
        // Use last created restaurant if available
        std::string lastId = RestaurantExecutor::getLastCreatedRestaurantId();
        return lastId.empty() ? "507f1f77bcf86cd799439011" : lastId;
    }
    
    // === MENU ITEM PARAMETERS ===
    if (paramType == "itemName" || paramType == "menuItemName") {
        return "TestItem_" + std::to_string(itemCounter++);
    }
    if (paramType == "menuItemId") {
        std::string lastId = RestaurantExecutor::getLastCreatedMenuItemId();
        return lastId.empty() ? "507f191e810c19729de860ea" : lastId;
    }
    
    // === ORDER PARAMETERS ===
    if (paramType == "orderStatus" || paramType == "status") {
        return "accepted";
    }
    if (paramType == "orderId") {
        std::string lastId = RestaurantExecutor::getLastCreatedOrderId();
        return lastId.empty() ? "507f191e810c19729de860eb" : lastId;
    }
    
    // === CART PARAMETERS ===
    if (paramType == "quantity") {
        return "2";
    }
    
    // === AGENT PARAMETERS ===
    if (paramType == "agentId") {
        return "507f191e810c19729de860ec";
    }
    
    // === REVIEW PARAMETERS ===
    if (paramType == "rating") {
        return "5";
    }
    if (paramType == "comment") {
        return "Great food and excellent service!";
    }
    
    // Default: return original value
    return value;
}

// ========================================
// HELPER FUNCTIONS
// ========================================

namespace {
    std::string extractValue(const Expr& expr) {
        if (auto* str = dynamic_cast<const String*>(&expr)) {
            return str->value;
        }
        if (auto* num = dynamic_cast<const Num*>(&expr)) {
            return std::to_string(num->value);
        }
        if (auto* var = dynamic_cast<const Var*>(&expr)) {
            return var->name;  // Return variable name as fallback
        }
        throw std::runtime_error("Cannot extract value from non-concrete Expr");
    }
    
    std::unique_ptr<Expr> cloneExpr(const Expr* expr) {
        if (!expr) return nullptr;
        if (auto* str = dynamic_cast<const String*>(expr)) {
            return std::make_unique<String>(str->value);
        }
        if (auto* num = dynamic_cast<const Num*>(expr)) {
            return std::make_unique<Num>(num->value);
        }
        if (auto* var = dynamic_cast<const Var*>(expr)) {
            return std::make_unique<Var>(var->name);
        }
        throw std::runtime_error("Unsupported Expr type for cloning");
    }
}
namespace Restaurant {

// ========================================
// AUTH API FUNCTIONS
// ========================================

/**
 * RegisterUser Function
 * POST /api/auth/register
 * Parameters: fullName, email, password, mobile, role
 */
class RegisterUserFunction : public APIFunction {
private:
    std::unique_ptr<Expr> fullNameExpr;
    std::unique_ptr<Expr> emailExpr;
    std::unique_ptr<Expr> passwordExpr;
    std::unique_ptr<Expr> mobileExpr;
    std::unique_ptr<Expr> roleExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 5) {
            fullNameExpr = cloneExpr(params[0].get());
            emailExpr = cloneExpr(params[1].get());
            passwordExpr = cloneExpr(params[2].get());
            mobileExpr = cloneExpr(params[3].get());
            roleExpr = cloneExpr(params[4].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string fullName = extractValue(*fullNameExpr);
        std::string email = extractValue(*emailExpr);
        std::string password = extractValue(*passwordExpr);
        std::string mobile = extractValue(*mobileExpr);
        std::string role = extractValue(*roleExpr);
        
        // Transform Z3 placeholder values to realistic values
        fullName = transformValue(fullName, "ownerName");
        email = transformValue(email, "ownerEmail");
        password = transformValue(password, "password");
        mobile = transformValue(mobile, "ownerMobile");
        role = transformValue(role, "ownerRole");
        
        std::cout << "[RegisterUserFunction] Executing: " << fullName 
                  << " (" << email << ", role: " << role << ")" << std::endl;
        
        auto response = RestaurantExecutor::registerUser(fullName, email, password, mobile, role);
        int code = (response.find("\"token\"") != std::string::npos) ? 201 : 400;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "registerUser"; }
};

/**
 * Login Function
 * POST /api/auth/login
 * Parameters: email, password
 */
class LoginFunction : public APIFunction {
private:
    std::unique_ptr<Expr> emailExpr;
    std::unique_ptr<Expr> passwordExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 2) {
            emailExpr = cloneExpr(params[0].get());
            passwordExpr = cloneExpr(params[1].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string email = extractValue(*emailExpr);
        std::string password = extractValue(*passwordExpr);
        
        // Transform Z3 placeholder values to realistic values
        email = transformValue(email, "ownerEmail");
        password = transformValue(password, "password");
        
        std::cout << "[LoginFunction] Executing: " << email << std::endl;
        
        auto response = RestaurantExecutor::login(email, password);
        int code = (response.find("\"token\"") != std::string::npos) ? 200 : 401;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "login"; }
};

// ========================================
// RESTAURANT API FUNCTIONS
// ========================================

/**
 * BrowseRestaurants Function
 * GET /api/restaurants
 * Parameters: none
 */
class BrowseRestaurantsFunction : public APIFunction {
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {}
    
    std::unique_ptr<Expr> execute() override {
        std::cout << "[BrowseRestaurantsFunction] Executing" << std::endl;
        auto response = RestaurantExecutor::browseRestaurants();
        return std::make_unique<Num>(200);
    }
    
    std::string getName() const override { return "browseRestaurants"; }
};

/**
 * GetRestaurant Function
 * GET /api/restaurants/:id
 * Parameters: restaurantId
 */
class GetRestaurantFunction : public APIFunction {
private:
    std::unique_ptr<Expr> restaurantIdExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) {
            restaurantIdExpr = cloneExpr(params[0].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string restaurantId = extractValue(*restaurantIdExpr);
        restaurantId = transformValue(restaurantId, "restaurantId");
        
        std::cout << "[GetRestaurantFunction] Executing: " << restaurantId << std::endl;
        auto response = RestaurantExecutor::getRestaurant(restaurantId);
        return std::make_unique<Num>(200);
    }
    
    std::string getName() const override { return "getRestaurant"; }
};

/**
 * CreateRestaurant Function
 * POST /api/restaurants
 * Parameters: restaurantName
 */
class CreateRestaurantFunction : public APIFunction {
private:
    std::unique_ptr<Expr> nameExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) {
            nameExpr = cloneExpr(params[0].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string name = nameExpr ? extractValue(*nameExpr) : "TestRestaurant";
        name = transformValue(name, "restaurantName");
        
        std::cout << "[CreateRestaurantFunction] Executing: " << name << std::endl;
        auto response = RestaurantExecutor::createRestaurant(name, "Indian", "Bangalore", "MG Road");
        
        int code = (response.find("\"_id\"") != std::string::npos || 
                    response.find("\"restaurant\"") != std::string::npos) ? 201 : 400;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "createRestaurant"; }
};

/**
 * UpdateRestaurant Function
 * PUT /api/restaurants/:id
 * Parameters: restaurantId
 */
class UpdateRestaurantFunction : public APIFunction {
private:
    std::unique_ptr<Expr> restaurantIdExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) {
            restaurantIdExpr = cloneExpr(params[0].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string restaurantId = extractValue(*restaurantIdExpr);
        restaurantId = transformValue(restaurantId, "restaurantId");
        
        std::cout << "[UpdateRestaurantFunction] Executing: " << restaurantId << std::endl;
        std::string updateData = R"({"isOpen": true})";
        auto response = RestaurantExecutor::updateRestaurant(restaurantId, updateData);
        
        int code = (response.find("error") == std::string::npos) ? 200 : 400;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "updateRestaurant"; }
};

/**
 * ViewMenu Function
 * GET /api/restaurants/:id/menu
 * Parameters: restaurantId
 */
class ViewMenuFunction : public APIFunction {
private:
    std::unique_ptr<Expr> restaurantIdExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) {
            restaurantIdExpr = cloneExpr(params[0].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string restaurantId = extractValue(*restaurantIdExpr);
        restaurantId = transformValue(restaurantId, "restaurantId");
        
        std::cout << "[ViewMenuFunction] Executing: " << restaurantId << std::endl;
        auto response = RestaurantExecutor::viewMenu(restaurantId);
        return std::make_unique<Num>(200);
    }
    
    std::string getName() const override { return "viewMenu"; }
};

// ========================================
// MENU API FUNCTIONS
// ========================================

/**
 * AddMenuItem Function
 * POST /api/menu
 * Parameters: menuItemName
 */
class AddMenuItemFunction : public APIFunction {
private:
    std::unique_ptr<Expr> itemNameExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) {
            itemNameExpr = cloneExpr(params[0].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string itemName = itemNameExpr ? extractValue(*itemNameExpr) : "TestItem";
        itemName = transformValue(itemName, "itemName");
        
        // Get the restaurant ID from last created restaurant
        std::string restaurantId = RestaurantExecutor::getLastCreatedRestaurantId();
        if (restaurantId.empty()) {
            restaurantId = "507f1f77bcf86cd799439011";
        }
        
        std::cout << "[AddMenuItemFunction] Executing: " << itemName 
                  << " for restaurant " << restaurantId << std::endl;
        auto response = RestaurantExecutor::addMenuItem(restaurantId, itemName, "150", "Main Course");
        
        int code = (response.find("error") == std::string::npos) ? 201 : 400;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "addMenuItem"; }
};

/**
 * DeleteMenuItem Function
 * DELETE /api/menu/:id
 * Parameters: menuItemId
 */
class DeleteMenuItemFunction : public APIFunction {
private:
    std::unique_ptr<Expr> menuItemIdExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) {
            menuItemIdExpr = cloneExpr(params[0].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string menuItemId = extractValue(*menuItemIdExpr);
        menuItemId = transformValue(menuItemId, "menuItemId");
        
        std::cout << "[DeleteMenuItemFunction] Executing: " << menuItemId << std::endl;
        auto response = RestaurantExecutor::deleteMenuItem(menuItemId);
        
        int code = (response.find("error") == std::string::npos) ? 200 : 400;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "deleteMenuItem"; }
};

// ========================================
// CART API FUNCTIONS
// ========================================

/**
 * ViewCart Function
 * GET /api/cart
 * Parameters: none
 */
class ViewCartFunction : public APIFunction {
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {}
    
    std::unique_ptr<Expr> execute() override {
        std::cout << "[ViewCartFunction] Executing" << std::endl;
        auto response = RestaurantExecutor::viewCart();
        return std::make_unique<Num>(200);
    }
    
    std::string getName() const override { return "viewCart"; }
};

/**
 * AddToCart Function
 * POST /api/cart
 * Parameters: menuItemId, quantity (or restaurantId, menuItemId, quantity)
 */
class AddToCartFunction : public APIFunction {
private:
    std::unique_ptr<Expr> param1Expr;
    std::unique_ptr<Expr> param2Expr;
    std::unique_ptr<Expr> param3Expr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) param1Expr = cloneExpr(params[0].get());
        if (params.size() >= 2) param2Expr = cloneExpr(params[1].get());
        if (params.size() >= 3) param3Expr = cloneExpr(params[2].get());
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string menuItemId;
        std::string quantity = "1";
        
        if (param3Expr) {
            // 3 params: restaurantId, menuItemId, quantity
            menuItemId = extractValue(*param2Expr);
            quantity = extractValue(*param3Expr);
        } else if (param2Expr) {
            // 2 params: menuItemId, quantity
            menuItemId = extractValue(*param1Expr);
            quantity = extractValue(*param2Expr);
        } else if (param1Expr) {
            // 1 param: menuItemId
            menuItemId = extractValue(*param1Expr);
        }
        
        menuItemId = transformValue(menuItemId, "menuItemId");
        quantity = transformValue(quantity, "quantity");
        
        std::cout << "[AddToCartFunction] Executing: item=" << menuItemId 
                  << ", qty=" << quantity << std::endl;
        auto response = RestaurantExecutor::addToCart(menuItemId, quantity);
        
        int code = (response.find("error") == std::string::npos) ? 200 : 400;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "addToCart"; }
};

/**
 * RemoveFromCart Function
 * DELETE /api/cart/:itemId
 * Parameters: itemId
 */
class RemoveFromCartFunction : public APIFunction {
private:
    std::unique_ptr<Expr> itemIdExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) {
            itemIdExpr = cloneExpr(params[0].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string itemId = extractValue(*itemIdExpr);
        itemId = transformValue(itemId, "menuItemId");
        
        std::cout << "[RemoveFromCartFunction] Executing: " << itemId << std::endl;
        auto response = RestaurantExecutor::removeFromCart(itemId);
        
        int code = (response.find("error") == std::string::npos) ? 200 : 400;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "removeFromCart"; }
};

/**
 * ClearCart Function
 * DELETE /api/cart/clear
 * Parameters: none
 */
class ClearCartFunction : public APIFunction {
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {}
    
    std::unique_ptr<Expr> execute() override {
        std::cout << "[ClearCartFunction] Executing" << std::endl;
        auto response = RestaurantExecutor::clearCart();
        return std::make_unique<Num>(200);
    }
    
    std::string getName() const override { return "clearCart"; }
};

// ========================================
// ORDER API FUNCTIONS
// ========================================

/**
 * PlaceOrder Function
 * POST /api/orders
 * Parameters: none (cart should already be populated)
 */
class PlaceOrderFunction : public APIFunction {
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {}
    
    std::unique_ptr<Expr> execute() override {
        std::cout << "[PlaceOrderFunction] Executing" << std::endl;
        auto response = RestaurantExecutor::placeOrder();
        
        int code = (response.find("\"_id\"") != std::string::npos || 
                    response.find("\"order\"") != std::string::npos) ? 201 : 400;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "placeOrder"; }
};

/**
 * ViewOrders Function
 * GET /api/orders
 * Parameters: none
 */
class ViewOrdersFunction : public APIFunction {
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {}
    
    std::unique_ptr<Expr> execute() override {
        std::cout << "[ViewOrdersFunction] Executing" << std::endl;
        auto response = RestaurantExecutor::viewOrders();
        return std::make_unique<Num>(200);
    }
    
    std::string getName() const override { return "viewOrders"; }
};

/**
 * GetOrder Function
 * GET /api/orders/:id
 * Parameters: orderId
 */
class GetOrderFunction : public APIFunction {
private:
    std::unique_ptr<Expr> orderIdExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) {
            orderIdExpr = cloneExpr(params[0].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string orderId = extractValue(*orderIdExpr);
        orderId = transformValue(orderId, "orderId");
        
        std::cout << "[GetOrderFunction] Executing: " << orderId << std::endl;
        auto response = RestaurantExecutor::getOrder(orderId);
        return std::make_unique<Num>(200);
    }
    
    std::string getName() const override { return "getOrder"; }
};

/**
 * AssignOrder Function
 * PUT /api/orders/:id/assign
 * Parameters: orderId, agentId
 */
class AssignOrderFunction : public APIFunction {
private:
    std::unique_ptr<Expr> orderIdExpr;
    std::unique_ptr<Expr> agentIdExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 2) {
            orderIdExpr = cloneExpr(params[0].get());
            agentIdExpr = cloneExpr(params[1].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string orderId = extractValue(*orderIdExpr);
        std::string agentId = extractValue(*agentIdExpr);
        
        orderId = transformValue(orderId, "orderId");
        agentId = transformValue(agentId, "agentId");
        
        std::cout << "[AssignOrderFunction] Executing: order=" << orderId 
                  << ", agent=" << agentId << std::endl;
        auto response = RestaurantExecutor::assignOrder(orderId, agentId);
        
        int code = (response.find("error") == std::string::npos) ? 200 : 400;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "assignOrder"; }
};

/**
 * UpdateOrderStatus Function
 * PUT /api/orders/:id/status
 * Parameters: orderId, status
 */
class UpdateOrderStatusFunction : public APIFunction {
private:
    std::unique_ptr<Expr> orderIdExpr;
    std::unique_ptr<Expr> statusExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 2) {
            orderIdExpr = cloneExpr(params[0].get());
            statusExpr = cloneExpr(params[1].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string orderId = extractValue(*orderIdExpr);
        std::string status = extractValue(*statusExpr);
        
        orderId = transformValue(orderId, "orderId");
        status = transformValue(status, "orderStatus");
        
        std::cout << "[UpdateOrderStatusFunction] Executing: order=" << orderId 
                  << ", status=" << status << std::endl;
        auto response = RestaurantExecutor::updateOrderStatus(orderId, status);
        
        int code = (response.find("error") == std::string::npos) ? 200 : 400;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "updateOrderStatus"; }
};

// ========================================
// REVIEW API FUNCTIONS
// ========================================

/**
 * CreateReview Function
 * POST /api/reviews
 * Parameters: orderId, rating, comment (flexible)
 */
class CreateReviewFunction : public APIFunction {
private:
    std::unique_ptr<Expr> orderIdExpr;
    std::unique_ptr<Expr> param2Expr;
    std::unique_ptr<Expr> param3Expr;
    std::unique_ptr<Expr> param4Expr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) orderIdExpr = cloneExpr(params[0].get());
        if (params.size() >= 2) param2Expr = cloneExpr(params[1].get());
        if (params.size() >= 3) param3Expr = cloneExpr(params[2].get());
        if (params.size() >= 4) param4Expr = cloneExpr(params[3].get());
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string orderId = orderIdExpr ? extractValue(*orderIdExpr) : "";
        std::string rating = "5";
        std::string comment = "Great food!";
        
        orderId = transformValue(orderId, "orderId");
        
        if (param4Expr) {
            // 4 params: orderId, restaurantId, rating, comment
            rating = extractValue(*param3Expr);
            comment = extractValue(*param4Expr);
        } else if (param3Expr) {
            // 3 params: orderId, rating, comment
            rating = extractValue(*param2Expr);
            comment = extractValue(*param3Expr);
        } else if (param2Expr) {
            // 2 params: orderId, rating
            rating = extractValue(*param2Expr);
        }
        
        rating = transformValue(rating, "rating");
        comment = transformValue(comment, "comment");
        
        std::cout << "[CreateReviewFunction] Executing: order=" << orderId 
                  << ", rating=" << rating << std::endl;
        auto response = RestaurantExecutor::createReview(orderId, rating, rating, comment);
        
        int code = (response.find("error") == std::string::npos) ? 201 : 400;
        return std::make_unique<Num>(code);
    }
    
    std::string getName() const override { return "createReview"; }
};

/**
 * GetRestaurantReviews Function
 * GET /api/reviews/restaurant/:restaurantId
 * Parameters: restaurantId
 */
class GetRestaurantReviewsFunction : public APIFunction {
private:
    std::unique_ptr<Expr> restaurantIdExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) {
            restaurantIdExpr = cloneExpr(params[0].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string restaurantId = extractValue(*restaurantIdExpr);
        restaurantId = transformValue(restaurantId, "restaurantId");
        
        std::cout << "[GetRestaurantReviewsFunction] Executing: " << restaurantId << std::endl;
        auto response = RestaurantExecutor::getRestaurantReviews(restaurantId);
        return std::make_unique<Num>(200);
    }
    
    std::string getName() const override { return "getRestaurantReviews"; }
};

// ========================================
// USER API FUNCTIONS
// ========================================

/**
 * GetUserProfile Function
 * GET /api/users/me
 * Parameters: none
 */
class GetUserProfileFunction : public APIFunction {
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {}
    
    std::unique_ptr<Expr> execute() override {
        std::cout << "[GetUserProfileFunction] Executing" << std::endl;
        auto response = RestaurantExecutor::getUserProfile();
        return std::make_unique<Num>(200);
    }
    
    std::string getName() const override { return "getUserProfile"; }
};

// ========================================
// FUNCTION FACTORY
// ========================================

class RestaurantFunctionFactory : public APIFunctionFactory {
public:
    std::unique_ptr<APIFunction> getFunction(
        const std::string& funcName,
        const std::vector<std::unique_ptr<Expr>>& args
    ) override {
        std::unique_ptr<APIFunction> func;
        
        // Auth functions
        if (funcName == "registerUser") {
            func = std::make_unique<RegisterUserFunction>();
        }
        else if (funcName == "login") {
            func = std::make_unique<LoginFunction>();
        }
        // Restaurant functions
        else if (funcName == "browseRestaurants") {
            func = std::make_unique<BrowseRestaurantsFunction>();
        }
        else if (funcName == "getRestaurant") {
            func = std::make_unique<GetRestaurantFunction>();
        }
        else if (funcName == "createRestaurant") {
            func = std::make_unique<CreateRestaurantFunction>();
        }
        else if (funcName == "updateRestaurant") {
            func = std::make_unique<UpdateRestaurantFunction>();
        }
        else if (funcName == "viewMenu") {
            func = std::make_unique<ViewMenuFunction>();
        }
        // Menu functions
        else if (funcName == "addMenuItem") {
            func = std::make_unique<AddMenuItemFunction>();
        }
        else if (funcName == "deleteMenuItem") {
            func = std::make_unique<DeleteMenuItemFunction>();
        }
        // Cart functions
        else if (funcName == "viewCart") {
            func = std::make_unique<ViewCartFunction>();
        }
        else if (funcName == "addToCart") {
            func = std::make_unique<AddToCartFunction>();
        }
        else if (funcName == "removeFromCart") {
            func = std::make_unique<RemoveFromCartFunction>();
        }
        else if (funcName == "clearCart") {
            func = std::make_unique<ClearCartFunction>();
        }
        // Order functions
        else if (funcName == "placeOrder") {
            func = std::make_unique<PlaceOrderFunction>();
        }
        else if (funcName == "viewOrders") {
            func = std::make_unique<ViewOrdersFunction>();
        }
        else if (funcName == "getOrder") {
            func = std::make_unique<GetOrderFunction>();
        }
        else if (funcName == "assignOrder") {
            func = std::make_unique<AssignOrderFunction>();
        }
        else if (funcName == "updateOrderStatus") {
            func = std::make_unique<UpdateOrderStatusFunction>();
        }
        // Review functions
        else if (funcName == "createReview") {
            func = std::make_unique<CreateReviewFunction>();
        }
        else if (funcName == "getRestaurantReviews") {
            func = std::make_unique<GetRestaurantReviewsFunction>();
        }
        // User functions
        else if (funcName == "getUserProfile") {
            func = std::make_unique<GetUserProfileFunction>();
        }
        
        if (func) {
            func->setParameters(args);
        }
        
        return func;
    }
    
    bool hasFunction(const std::string& funcName) const override {
        static const std::set<std::string> functions = {
            // Auth
            "registerUser", "login",
            // Restaurant
            "browseRestaurants", "getRestaurant", "createRestaurant", 
            "updateRestaurant", "viewMenu",
            // Menu
            "addMenuItem", "deleteMenuItem",
            // Cart
            "viewCart", "addToCart", "removeFromCart", "clearCart",
            // Order
            "placeOrder", "viewOrders", "getOrder", "assignOrder", "updateOrderStatus",
            // Review
            "createReview", "getRestaurantReviews",
            // User
            "getUserProfile"
        };
        return functions.count(funcName) > 0;
    }
};
}

// Factory getter function
APIFunctionFactory* getRestaurantFunctionFactory() {
   static Restaurant::RestaurantFunctionFactory factory;
   std::cout << "[Factory] Returning Restaurant factory instance at " << &factory << std::endl;
   return &factory;
}
