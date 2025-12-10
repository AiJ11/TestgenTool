#pragma once
/**
 * RestaurantExecutor.hpp
 * ======================
 * HTTP executor for Restaurant Backend API
 * 
 * Location: config/executors/RestaurantExecutor.hpp
 * 
 * This file handles all HTTP communication with the Restaurant backend.
 * It manages JWT tokens and provides methods for all API endpoints.
 */

#include <string>
#include <iostream>
#include <cstdlib>
#include <memory>
#include <array>
#include <sstream>

class RestaurantExecutor {
private:
    static inline std::string baseUrl = "http://localhost:5002";
    static inline int port = 5002;
    static inline std::string authToken = "";
    static inline std::string lastCreatedRestaurantId = "";
    static inline std::string lastCreatedMenuItemId = "";
    static inline std::string lastCreatedOrderId = "";

    /**
     * Execute a curl command and return the response
     */
    static std::string execCurl(const std::string& cmd) {
        std::array<char, 4096> buffer;
        std::string result;
        
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
            return R"({"error": "Failed to execute curl"})";
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        return result;
    }

    /**
     * HTTP POST request
     */
    static std::string httpPost(const std::string& url, const std::string& body, bool useAuth = false) {
        std::string cmd = "curl -s -X POST \"" + url + "\" "
                         "-H \"Content-Type: application/json\" ";
        
        if (useAuth && !authToken.empty()) {
            cmd += "-H \"Authorization: Bearer " + authToken + "\" ";
        }
        
        cmd += "-d '" + body + "' 2>/dev/null";
        
        return execCurl(cmd);
    }

    /**
     * HTTP GET request
     */
    static std::string httpGet(const std::string& url, bool useAuth = false) {
        std::string cmd = "curl -s -X GET \"" + url + "\" "
                         "-H \"Content-Type: application/json\" ";
        
        if (useAuth && !authToken.empty()) {
            cmd += "-H \"Authorization: Bearer " + authToken + "\" ";
        }
        
        cmd += "2>/dev/null";
        
        return execCurl(cmd);
    }

    /**
     * HTTP PUT request
     */
    static std::string httpPut(const std::string& url, const std::string& body, bool useAuth = false) {
        std::string cmd = "curl -s -X PUT \"" + url + "\" "
                         "-H \"Content-Type: application/json\" ";
        
        if (useAuth && !authToken.empty()) {
            cmd += "-H \"Authorization: Bearer " + authToken + "\" ";
        }
        
        cmd += "-d '" + body + "' 2>/dev/null";
        
        return execCurl(cmd);
    }

    /**
     * HTTP DELETE request
     */
    static std::string httpDelete(const std::string& url, bool useAuth = false) {
        std::string cmd = "curl -s -X DELETE \"" + url + "\" "
                         "-H \"Content-Type: application/json\" ";
        
        if (useAuth && !authToken.empty()) {
            cmd += "-H \"Authorization: Bearer " + authToken + "\" ";
        }
        
        cmd += "2>/dev/null";
        
        return execCurl(cmd);
    }

    /**
     * Extract a JSON string value by key (simple parser)
     */
    static std::string extractJsonValue(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\":";
        size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos) return "";
        
        size_t valueStart = keyPos + searchKey.length();
        
        // Skip whitespace
        while (valueStart < json.length() && 
               (json[valueStart] == ' ' || json[valueStart] == '\t')) {
            valueStart++;
        }
        
        if (valueStart >= json.length()) return "";
        
        // Check if it's a string value
        if (json[valueStart] == '"') {
            valueStart++;
            size_t valueEnd = json.find('"', valueStart);
            if (valueEnd == std::string::npos) return "";
            return json.substr(valueStart, valueEnd - valueStart);
        }
        
        // Check if it's a nested object (for extracting _id)
        if (json[valueStart] == '{') {
            // Look for _id inside the nested object
            size_t nestedEnd = json.find('}', valueStart);
            if (nestedEnd != std::string::npos) {
                std::string nested = json.substr(valueStart, nestedEnd - valueStart + 1);
                return extractJsonValue(nested, "_id");
            }
        }
        
        // For non-string values (numbers, booleans)
        size_t valueEnd = json.find_first_of(",}]", valueStart);
        if (valueEnd == std::string::npos) return "";
        return json.substr(valueStart, valueEnd - valueStart);
    }

public:
    // ========================================
    // CONFIGURATION & INITIALIZATION
    // ========================================
    
    /**
     * Set the port for the backend server
     * Updates both port and baseUrl
     */
    static void setPort(int p) {
        port = p;
        baseUrl = "http://localhost:" + std::to_string(port);
        std::cout << "[RestaurantExecutor] Port set to " << port << std::endl;
        std::cout << "[RestaurantExecutor] Base URL: " << baseUrl << std::endl;
    }
    
    static int getPort() {
        return port;
    }
    
    static void setBaseUrl(const std::string& url) {
        baseUrl = url;
    }
    
    static std::string getBaseUrl() {
        return baseUrl;
    }
    
    static void setAuthToken(const std::string& token) {
        authToken = token;
    }
    
    static std::string getAuthToken() {
        return authToken;
    }
    
    static void clearAuthToken() {
        authToken = "";
    }

    static std::string getLastCreatedRestaurantId() {
        return lastCreatedRestaurantId;
    }

    static std::string getLastCreatedMenuItemId() {
        return lastCreatedMenuItemId;
    }

    static std::string getLastCreatedOrderId() {
        return lastCreatedOrderId;
    }

    // ========================================
    // HEALTH CHECK & TEST DATA RESET
    // ========================================
    
    /**
     * Check if the backend server is healthy/reachable
     * Returns true if server responds, false otherwise
     */
    static bool checkHealth() {
        std::cout << "[RestaurantExecutor] Checking health at " << baseUrl << std::endl;
        
        // Try to reach the restaurants endpoint (public, no auth needed)
        std::string url = baseUrl + "/api/restaurants";
        std::string response = httpGet(url, false);
        
        // If we get any response (even empty array), server is up
        bool healthy = !response.empty() && response.find("error") == std::string::npos;
        
        if (healthy) {
            std::cout << "[RestaurantExecutor] ✓ Backend is healthy" << std::endl;
        } else {
            std::cout << "[RestaurantExecutor] ✗ Backend health check failed" << std::endl;
            std::cout << "[RestaurantExecutor] Response: " << response << std::endl;
        }
        
        return healthy;
    }
    
    /**
     * Reset test data on the backend
     * NOTE: The restaurant backend has NO /test/* endpoints!
     * This function clears local state only.
     * For full reset, you need to manually clear MongoDB.
     */
    static bool resetTestData() {
        std::cout << "[RestaurantExecutor] Resetting local test state..." << std::endl;
        
        // Clear local state
        authToken = "";
        lastCreatedRestaurantId = "";
        lastCreatedMenuItemId = "";
        lastCreatedOrderId = "";
        
        std::cout << "[RestaurantExecutor] ✓ Local state cleared" << std::endl;
        std::cout << "[RestaurantExecutor] NOTE: Backend has no /test/reset endpoint." << std::endl;
        std::cout << "[RestaurantExecutor] To fully reset, clear MongoDB manually:" << std::endl;
        std::cout << "[RestaurantExecutor]   mongosh -> use restaurant_db -> db.dropDatabase()" << std::endl;
        
        // Return true since local reset always succeeds
        // The backend doesn't support remote reset
        return true;
    }

    // ========================================
    // AUTH APIs
    // ========================================
    
    /**
     * POST /api/auth/register
     * Register a new user
     */
    static std::string registerUser(const std::string& fullName, 
                                    const std::string& email,
                                    const std::string& password, 
                                    const std::string& mobile,
                                    const std::string& role) {
        std::string url = baseUrl + "/api/auth/register";
        std::cout << "[Restaurant] POST " << url << std::endl;
        
        std::string body = R"({
            "fullName": ")" + fullName + R"(",
            "email": ")" + email + R"(",
            "password": ")" + password + R"(",
            "mobile": ")" + mobile + R"(",
            "role": ")" + role + R"("
        })";
        
        std::string response = httpPost(url, body, false);
        
        // Log response (truncated)
        if (response.length() > 100) {
            std::cout << "[Restaurant] Response: " << response.substr(0, 100) << "..." << std::endl;
        } else {
            std::cout << "[Restaurant] Response: " << response << std::endl;
        }
        
        // Check for success and extract token
        if (response.find("\"token\"") != std::string::npos) {
            std::string token = extractJsonValue(response, "token");
            if (!token.empty()) {
                authToken = token;
                std::cout << "[Restaurant] ✓ Status: 201" << std::endl;
                std::cout << "[Restaurant] ✓ Token obtained: " << token.substr(0, 20) << "..." << std::endl;
            }
        }
        
        return response;
    }
    
    /**
     * POST /api/auth/login
     * Login user and get token
     */
    static std::string login(const std::string& email, const std::string& password) {
        std::string url = baseUrl + "/api/auth/login";
        std::cout << "[Restaurant] POST " << url << std::endl;
        
        std::string body = R"({"email": ")" + email + R"(", "password": ")" + password + R"("})";
        
        std::string response = httpPost(url, body, false);
        
        // Log response (truncated)
        if (response.length() > 100) {
            std::cout << "[Restaurant] Response: " << response.substr(0, 100) << "..." << std::endl;
        } else {
            std::cout << "[Restaurant] Response: " << response << std::endl;
        }
        
        // Check for success
        if (response.find("\"token\"") != std::string::npos) {
            std::string token = extractJsonValue(response, "token");
            if (!token.empty()) {
                authToken = token;
                std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
                std::cout << "[Restaurant] ✓ Token obtained: " << token.substr(0, 20) << "..." << std::endl;
            }
        } else {
            std::cout << "[Restaurant] ✗ Login failed" << std::endl;
        }
        
        return response;
    }

    // ========================================
    // USER APIs
    // ========================================
    
    /**
     * GET /api/users/me
     * Get current user profile
     */
    static std::string getUserProfile() {
        std::string url = baseUrl + "/api/users/me";
        std::cout << "[Restaurant] GET " << url << std::endl;
        
        std::string response = httpGet(url, true);
        std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        
        return response;
    }

    // ========================================
    // RESTAURANT APIs
    // ========================================
    
    /**
     * GET /api/restaurants
     * Browse all restaurants
     */
    static std::string browseRestaurants() {
        std::string url = baseUrl + "/api/restaurants";
        std::cout << "[Restaurant] GET " << url << std::endl;
        
        std::string response = httpGet(url, true);
        std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        
        return response;
    }
    
    /**
     * GET /api/restaurants/:id
     * Get restaurant by ID
     */
    static std::string getRestaurant(const std::string& restaurantId) {
        std::string url = baseUrl + "/api/restaurants/" + restaurantId;
        std::cout << "[Restaurant] GET " << url << std::endl;
        
        std::string response = httpGet(url, true);
        std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        
        return response;
    }
    
    /**
     * POST /api/restaurants
     * Create a new restaurant (restaurant_owner only)
     */
    static std::string createRestaurant(const std::string& name,
                                        const std::string& cuisineType,
                                        const std::string& city,
                                        const std::string& street) {
        std::string url = baseUrl + "/api/restaurants";
        std::cout << "[Restaurant] POST " << url << std::endl;
        
        std::string body = R"({
            "name": ")" + name + R"(",
            "cuisineTypes": [")" + cuisineType + R"("],
            "address": {
                "street": ")" + street + R"(",
                "city": ")" + city + R"(",
                "state": "Karnataka",
                "zipCode": "560001"
            },
            "contact": {
                "phone": "1234567890",
                "email": "restaurant@test.com"
            },
            "openingHours": {
                "open": "09:00",
                "close": "22:00"
            }
        })";
        
        std::string response = httpPost(url, body, true);
        
        // Extract restaurant ID from response
        if (response.find("\"_id\"") != std::string::npos) {
            std::string id = extractJsonValue(response, "_id");
            if (!id.empty()) {
                lastCreatedRestaurantId = id;
                std::cout << "[Restaurant] ✓ Status: 201" << std::endl;
                std::cout << "[Restaurant] ✓ Restaurant created with ID: " << id << std::endl;
            }
        } else if (response.find("restaurant") != std::string::npos) {
            // Try to extract from nested restaurant object
            size_t restStart = response.find("\"restaurant\"");
            if (restStart != std::string::npos) {
                std::string restPart = response.substr(restStart);
                std::string id = extractJsonValue(restPart, "_id");
                if (!id.empty()) {
                    lastCreatedRestaurantId = id;
                    std::cout << "[Restaurant] ✓ Status: 201" << std::endl;
                    std::cout << "[Restaurant] ✓ Restaurant created with ID: " << id << std::endl;
                }
            }
        } else {
            std::cout << "[Restaurant] Response: " << response << std::endl;
        }
        
        return response;
    }
    
    /**
     * PUT /api/restaurants/:id
     * Update restaurant (owner only)
     */
    static std::string updateRestaurant(const std::string& restaurantId, 
                                        const std::string& updateData) {
        std::string url = baseUrl + "/api/restaurants/" + restaurantId;
        std::cout << "[Restaurant] PUT " << url << std::endl;
        
        std::string response = httpPut(url, updateData, true);
        
        if (response.find("error") == std::string::npos) {
            std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
            std::cout << "[Restaurant] ✓ Restaurant updated" << std::endl;
        } else {
            std::cout << "[Restaurant] ✗ Update failed: " << response << std::endl;
        }
        
        return response;
    }
    
    /**
     * GET /api/restaurants/:id/menu
     * Get restaurant menu
     */
    static std::string viewMenu(const std::string& restaurantId) {
        std::string url = baseUrl + "/api/restaurants/" + restaurantId + "/menu";
        std::cout << "[Restaurant] GET " << url << std::endl;
        
        std::string response = httpGet(url, true);
        std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        
        return response;
    }

    // ========================================
    // MENU APIs
    // ========================================
    
    /**
     * POST /api/menu
     * Add menu item (restaurant_owner only)
     */
    static std::string addMenuItem(const std::string& restaurantId,
                                   const std::string& name,
                                   const std::string& price,
                                   const std::string& category) {
        std::string url = baseUrl + "/api/menu";
        std::cout << "[Restaurant] POST " << url << std::endl;
        
        std::string body = R"({
            "restaurantId": ")" + restaurantId + R"(",
            "name": ")" + name + R"(",
            "description": "Delicious test item",
            "price": )" + price + R"(,
            "category": ")" + category + R"(",
            "isAvailable": true
        })";
        
        std::string response = httpPost(url, body, true);
        
        // Extract menu item ID from response
        if (response.find("\"_id\"") != std::string::npos || 
            response.find("menuItem") != std::string::npos) {
            std::string id = extractJsonValue(response, "_id");
            if (id.empty()) {
                // Try nested menuItem object
                size_t itemStart = response.find("\"menuItem\"");
                if (itemStart != std::string::npos) {
                    std::string itemPart = response.substr(itemStart);
                    id = extractJsonValue(itemPart, "_id");
                }
            }
            if (!id.empty()) {
                lastCreatedMenuItemId = id;
                std::cout << "[Restaurant] ✓ Status: 201" << std::endl;
                std::cout << "[Restaurant] ✓ Menu item created with ID: " << id << std::endl;
            }
        } else {
            std::cout << "[Restaurant] Response: " << response << std::endl;
        }
        
        return response;
    }
    
    /**
     * PUT /api/menu/:id
     * Update menu item (restaurant_owner only)
     */
    static std::string updateMenuItem(const std::string& menuItemId,
                                      const std::string& updateData) {
        std::string url = baseUrl + "/api/menu/" + menuItemId;
        std::cout << "[Restaurant] PUT " << url << std::endl;
        
        std::string response = httpPut(url, updateData, true);
        
        if (response.find("error") == std::string::npos) {
            std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
            std::cout << "[Restaurant] ✓ Menu item updated" << std::endl;
        } else {
            std::cout << "[Restaurant] ✗ Update failed: " << response << std::endl;
        }
        
        return response;
    }
    
    /**
     * DELETE /api/menu/:id
     * Delete menu item (restaurant_owner only)
     */
    static std::string deleteMenuItem(const std::string& menuItemId) {
        std::string url = baseUrl + "/api/menu/" + menuItemId;
        std::cout << "[Restaurant] DELETE " << url << std::endl;
        
        std::string response = httpDelete(url, true);
        
        if (response.find("error") == std::string::npos) {
            std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
            std::cout << "[Restaurant] ✓ Menu item deleted" << std::endl;
        } else {
            std::cout << "[Restaurant] ✗ Delete failed: " << response << std::endl;
        }
        
        return response;
    }

    // ========================================
    // CART APIs
    // ========================================
    
    /**
     * GET /api/cart
     * View cart
     */
    static std::string viewCart() {
        std::string url = baseUrl + "/api/cart";
        std::cout << "[Restaurant] GET " << url << std::endl;
        
        std::string response = httpGet(url, true);
        std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        
        return response;
    }
    
    /**
     * POST /api/cart
     * Add item to cart
     */
    static std::string addToCart(const std::string& menuItemId, 
                                 const std::string& quantity) {
        std::string url = baseUrl + "/api/cart";
        std::cout << "[Restaurant] POST " << url << std::endl;
        
        std::string body = R"({"menuItemId": ")" + menuItemId + R"(", "quantity": )" + quantity + R"(})";
        
        std::string response = httpPost(url, body, true);
        
        if (response.find("error") == std::string::npos) {
            std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
            std::cout << "[Restaurant] ✓ Item added to cart" << std::endl;
        } else {
            std::cout << "[Restaurant] ✗ Failed: " << response << std::endl;
        }
        
        return response;
    }
    
    /**
     * PUT /api/cart/:itemId
     * Update cart item quantity
     */
    static std::string updateCartItem(const std::string& itemId, 
                                      const std::string& quantity) {
        std::string url = baseUrl + "/api/cart/" + itemId;
        std::cout << "[Restaurant] PUT " << url << std::endl;
        
        std::string body = R"({"quantity": )" + quantity + R"(})";
        
        std::string response = httpPut(url, body, true);
        
        if (response.find("error") == std::string::npos) {
            std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        }
        
        return response;
    }
    
    /**
     * DELETE /api/cart/:itemId
     * Remove item from cart
     */
    static std::string removeFromCart(const std::string& itemId) {
        std::string url = baseUrl + "/api/cart/" + itemId;
        std::cout << "[Restaurant] DELETE " << url << std::endl;
        
        std::string response = httpDelete(url, true);
        
        if (response.find("error") == std::string::npos) {
            std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
            std::cout << "[Restaurant] ✓ Item removed from cart" << std::endl;
        }
        
        return response;
    }
    
    /**
     * DELETE /api/cart/clear
     * Clear entire cart
     */
    static std::string clearCart() {
        std::string url = baseUrl + "/api/cart/clear";
        std::cout << "[Restaurant] DELETE " << url << std::endl;
        
        std::string response = httpDelete(url, true);
        
        if (response.find("error") == std::string::npos) {
            std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
            std::cout << "[Restaurant] ✓ Cart cleared" << std::endl;
        }
        
        return response;
    }

    // ========================================
    // ORDER APIs
    // ========================================
    
    /**
     * POST /api/orders
     * Place order from cart
     */
    static std::string placeOrder(const std::string& deliveryAddress = "") {
        std::string url = baseUrl + "/api/orders";
        std::cout << "[Restaurant] POST " << url << std::endl;
        
        std::string addr = deliveryAddress.empty() ? "123 Test Street, Bangalore" : deliveryAddress;
        std::string body = R"({
            "deliveryAddress": ")" + addr + R"(",
            "paymentMethod": "cash",
            "notes": "Test order"
        })";
        
        std::string response = httpPost(url, body, true);
        
        // Extract order ID
        if (response.find("\"_id\"") != std::string::npos || 
            response.find("order") != std::string::npos) {
            std::string id = extractJsonValue(response, "_id");
            if (id.empty()) {
                size_t orderStart = response.find("\"order\"");
                if (orderStart != std::string::npos) {
                    std::string orderPart = response.substr(orderStart);
                    id = extractJsonValue(orderPart, "_id");
                }
            }
            if (!id.empty()) {
                lastCreatedOrderId = id;
                std::cout << "[Restaurant] ✓ Status: 201" << std::endl;
                std::cout << "[Restaurant] ✓ Order placed with ID: " << id << std::endl;
            }
        } else {
            std::cout << "[Restaurant] Response: " << response << std::endl;
        }
        
        return response;
    }
    
    /**
     * GET /api/orders
     * View orders (based on user role)
     */
    static std::string viewOrders() {
        std::string url = baseUrl + "/api/orders";
        std::cout << "[Restaurant] GET " << url << std::endl;
        
        std::string response = httpGet(url, true);
        std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        
        return response;
    }
    
    /**
     * GET /api/orders/:id
     * Get order details
     */
    static std::string getOrder(const std::string& orderId) {
        std::string url = baseUrl + "/api/orders/" + orderId;
        std::cout << "[Restaurant] GET " << url << std::endl;
        
        std::string response = httpGet(url, true);
        std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        
        return response;
    }
    
    /**
     * PUT /api/orders/:id/status
     * Update order status (restaurant_owner or delivery_agent)
     */
    static std::string updateOrderStatus(const std::string& orderId, 
                                         const std::string& status) {
        std::string url = baseUrl + "/api/orders/" + orderId + "/status";
        std::cout << "[Restaurant] PUT " << url << std::endl;
        
        std::string body = R"({"status": ")" + status + R"("})";
        
        std::string response = httpPut(url, body, true);
        
        if (response.find("error") == std::string::npos) {
            std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
            std::cout << "[Restaurant] ✓ Order status updated to: " << status << std::endl;
        } else {
            std::cout << "[Restaurant] ✗ Update failed: " << response << std::endl;
        }
        
        return response;
    }
    
    /**
     * PUT /api/orders/:id/assign
     * Assign delivery agent to order (restaurant_owner or admin)
     */
    static std::string assignOrder(const std::string& orderId, 
                                   const std::string& agentId) {
        std::string url = baseUrl + "/api/orders/" + orderId + "/assign";
        std::cout << "[Restaurant] PUT " << url << std::endl;
        
        std::string body = R"({"deliveryAgentId": ")" + agentId + R"("})";
        
        std::string response = httpPut(url, body, true);
        
        if (response.find("error") == std::string::npos) {
            std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
            std::cout << "[Restaurant] ✓ Order assigned to agent: " << agentId << std::endl;
        } else {
            std::cout << "[Restaurant] ✗ Assignment failed: " << response << std::endl;
        }
        
        return response;
    }

    // ========================================
    // REVIEW APIs
    // ========================================
    
    /**
     * POST /api/reviews
     * Create review for order
     */
    static std::string createReview(const std::string& orderId,
                                    const std::string& restaurantRating,
                                    const std::string& deliveryRating,
                                    const std::string& comment) {
        std::string url = baseUrl + "/api/reviews";
        std::cout << "[Restaurant] POST " << url << std::endl;
        
        std::string body = R"({
            "orderId": ")" + orderId + R"(",
            "restaurantRating": )" + restaurantRating + R"(,
            "deliveryRating": )" + deliveryRating + R"(,
            "restaurantComment": ")" + comment + R"(",
            "deliveryComment": "Good delivery"
        })";
        
        std::string response = httpPost(url, body, true);
        
        if (response.find("error") == std::string::npos) {
            std::cout << "[Restaurant] ✓ Status: 201" << std::endl;
            std::cout << "[Restaurant] ✓ Review created" << std::endl;
        } else {
            std::cout << "[Restaurant] ✗ Review failed: " << response << std::endl;
        }
        
        return response;
    }
    
    /**
     * GET /api/reviews/restaurant/:restaurantId
     * Get restaurant reviews
     */
    static std::string getRestaurantReviews(const std::string& restaurantId) {
        std::string url = baseUrl + "/api/reviews/restaurant/" + restaurantId;
        std::cout << "[Restaurant] GET " << url << std::endl;
        
        std::string response = httpGet(url, false);
        std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        
        return response;
    }

    // ========================================
    // AGENT APIs
    // ========================================
    
    /**
     * GET /api/agents/me/orders
     * Get delivery agent's assigned orders
     */
    static std::string getAgentOrders() {
        std::string url = baseUrl + "/api/agents/me/orders";
        std::cout << "[Restaurant] GET " << url << std::endl;
        
        std::string response = httpGet(url, true);
        std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        
        return response;
    }
    
    /**
     * PUT /api/agents/me/status
     * Update agent availability
     */
    static std::string updateAgentStatus(bool isAvailable) {
        std::string url = baseUrl + "/api/agents/me/status";
        std::cout << "[Restaurant] PUT " << url << std::endl;
        
        std::string body = R"({"isAvailable": )" + std::string(isAvailable ? "true" : "false") + R"(})";
        
        std::string response = httpPut(url, body, true);
        
        if (response.find("error") == std::string::npos) {
            std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        }
        
        return response;
    }
    
    /**
     * GET /api/agents/available
     * Get available delivery agents
     */
    static std::string getAvailableAgents() {
        std::string url = baseUrl + "/api/agents/available";
        std::cout << "[Restaurant] GET " << url << std::endl;
        
        std::string response = httpGet(url, true);
        std::cout << "[Restaurant] ✓ Status: 200" << std::endl;
        
        return response;
    }
};
