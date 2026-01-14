#ifndef HTTPCLIENT_HH
#define HTTPCLIENT_HH

#include <string>
#include <map>
#include <memory>
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

// HTTP Response structure
struct HttpResponse {
    int statusCode;
    string body;
    map<string, string> headers;
    
    HttpResponse() : statusCode(0) {}
    
    // Parse JSON response body (safe version)
    json getJson() const {
        if (body.empty()) {
            return json::object();
        }
        
        // Trim whitespace
        string trimmed = body;
        trimmed.erase(0, trimmed.find_first_not_of(" \n\r\t"));
        trimmed.erase(trimmed.find_last_not_of(" \n\r\t") + 1);
        
        if (trimmed.empty()) {
            return json::object();
        }
        
        try {
            return json::parse(trimmed);
        } catch (const json::parse_error& e) {
            cerr << "[HttpResponse] JSON parse error: " << e.what() << endl;
            cerr << "[HttpResponse] Body was: '" << body << "'" << endl;
            return json::object();  // Return empty object instead of throwing
        }
    }
};

// HTTP Client for making REST API calls
class HttpClient {
private:
    string baseUrl;
    CURL* curl;
    
    // Callback for curl to write response data
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
public:
    HttpClient(const string& baseUrl);
    ~HttpClient();
    
    // HTTP Methods
    HttpResponse get(const string& endpoint, const map<string, string>& headers = {});
    HttpResponse post(const string& endpoint, const json& body, const map<string, string>& headers = {});
    HttpResponse put(const string& endpoint, const json& body, const map<string, string>& headers = {});
    HttpResponse del(const string& endpoint, const map<string, string>& headers = {});
    
    // Helper to set base URL
    void setBaseUrl(const string& url) { baseUrl = url; }
    string getBaseUrl() const { return baseUrl; }
};

#endif // HTTPCLIENT_HH
