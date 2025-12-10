#pragma once
#include <string>
#include <vector>
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <map>

/**
 * Executor for PESU Foods backend API calls
 * Handles HTTP requests to the Node.js backend
 */
class PesuFoodsExecutor {
private:
    inline static std::string jwtToken = "";  
    inline static int port = 5001;            
    
    // Callback for curl to write response data
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t totalSize = size * nmemb;
        userp->append((char*)contents, totalSize);
        return totalSize;
    }
    
    // Parse JWT token from login response
    static std::string extractToken(const std::string& responseBody) {
        // Response format: {"message": "Login successful", "token": "..."}
        size_t pos = responseBody.find("\"token\"");
        if (pos == std::string::npos) return "";
        
        size_t start = responseBody.find("\"", pos + 7);
        if (start == std::string::npos) return "";
        
        size_t end = responseBody.find("\"", start + 1);
        if (end == std::string::npos) return "";
        
        return responseBody.substr(start + 1, end - start - 1);
    }

public:
    struct APIResponse {
        int statusCode;
        std::string body;
        bool success;
    };

    // Initialize with port
    static void setPort(int p) {
        port = p;
    }

    // Reset state before each test
    static void reset() {
        jwtToken = "";
    }

    
    // FUNCTIONAL API METHODS
    
    static APIResponse signup(const std::string& name, 
                              const std::string& email, 
                              const std::string& password) {
        std::string url = "http://localhost:" + std::to_string(port) + "/signup";
        
        std::ostringstream json;
        json << "{"
             << "\"name\":\"" << name << "\","
             << "\"usermail\":\"" << email << "\","
             << "\"password\":\"" << password << "\""
             << "}";
        
        std::cout << "[PesuFoods] Calling: POST " << url << std::endl;
        std::cout << "[PesuFoods] Body: " << json.str() << std::endl;
        
        return makeRequest(url, "POST", json.str(), false);
    }
    
    static APIResponse login(const std::string& email, 
                            const std::string& password) {
        std::string url = "http://localhost:" + std::to_string(port) + "/login";
        
        std::ostringstream json;
        json << "{"
             << "\"usermail\":\"" << email << "\","
             << "\"password\":\"" << password << "\""
             << "}";
        
        std::cout << "[PesuFoods] Calling: POST " << url << std::endl;
        
        APIResponse response = makeRequest(url, "POST", json.str(), false);
        
        // If login successful, extract and store token
        if (response.success) {
            jwtToken = extractToken(response.body);
            if (!jwtToken.empty()) {
                std::cout << "[PesuFoods] JWT Token obtained and stored" << std::endl;
            }
        }
        
        return response;
    }
    
    static APIResponse getAccount(const std::string& token = "") {
        std::string url = "http://localhost:" + std::to_string(port) + "/account";
        std::string useToken = token.empty() ? jwtToken : token;
        
        std::cout << "[PesuFoods] Calling: GET " << url << std::endl;
        
        return makeRequest(url, "GET", "", true, useToken);
    }

    // TEST API METHODS
    
    static std::string getGlobalU() {
        std::string url = "http://localhost:" + std::to_string(port) + "/test/get_U";
        
        APIResponse response = makeRequest(url, "GET", "", false);
        
        if (response.success) {
            std::cout << "[TestAPI] get_U: " << response.body << std::endl;
            return response.body;
        }
        return "{}";
    }
    
    static bool setGlobalU(const std::string& usersJSON) {
        std::string url = "http://localhost:" + std::to_string(port) + "/test/set_U";
        
        std::cout << "[TestAPI] set_U: " << usersJSON << std::endl;
        
        APIResponse response = makeRequest(url, "POST", usersJSON, false);
        return response.success;
    }
    
    static std::string getGlobalT() {
        std::string url = "http://localhost:" + std::to_string(port) + "/test/get_T";
        
        APIResponse response = makeRequest(url, "GET", "", false);
        
        if (response.success) {
            return response.body;
        }
        return "{}";
    }
    
    static bool resetTestData() {
        std::string url = "http://localhost:" + std::to_string(port) + "/test/reset";
        
        std::cout << "[TestAPI] Resetting database..." << std::endl;
        
        APIResponse response = makeRequest(url, "GET", "", false);
        
        if (response.success) {
            std::cout << "[TestAPI] Database reset complete" << std::endl;
        }
        
        return response.success;
    }
    
    static bool checkHealth() {
        std::string url = "http://localhost:" + std::to_string(port) + "/test/health";
        
        APIResponse response = makeRequest(url, "GET", "", false);
        
        if (response.success) {
            std::cout << "[TestAPI] Health check: OK" << std::endl;
            return true;
        }
        
        std::cerr << "[TestAPI] Health check FAILED!" << std::endl;
        return false;
    }

private:
    // Generic HTTP request helper
    static APIResponse makeRequest(const std::string& url,
                                   const std::string& method,
                                   const std::string& body,
                                   bool useAuth,
                                   const std::string& authToken = "") {
        APIResponse response;
        response.success = false;

        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "[PesuFoods] Failed to initialize curl" << std::endl;
            return response;
        }

        std::string responseBody;
        struct curl_slist* headers = nullptr;
        
        // Set headers
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        // Add authorization if needed
        if (useAuth && !authToken.empty()) {
            std::string authHeader = "Authorization: " + authToken;
            headers = curl_slist_append(headers, authHeader.c_str());
        }

        // Configure curl
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        
        // Set method and body
        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            if (!body.empty()) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            }
        } else if (method == "GET") {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }

        // Execute
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            long statusCode;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
            response.statusCode = (int)statusCode;
            response.body = responseBody;
            response.success = (statusCode >= 200 && statusCode < 300);
            
            std::cout << "[PesuFoods] Response: " << statusCode << std::endl;
            if (!responseBody.empty()) {
                std::cout << "[PesuFoods] Body: " << responseBody << std::endl;
            }
        } else {
            std::cerr << "[PesuFoods] curl_easy_perform() failed: " 
                      << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        return response;
    }
};

