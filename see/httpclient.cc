#include "httpclient.hh"
#include <iostream>
#include <sstream>

// Static callback for CURL to write response data
size_t HttpClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    string* response = static_cast<string*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

HttpClient::HttpClient(const string& baseUrl) : baseUrl(baseUrl) {
    curl = curl_easy_init();
    if (!curl) {
        throw runtime_error("Failed to initialize CURL");
    }
}

HttpClient::~HttpClient() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

HttpResponse HttpClient::get(const string& endpoint, const map<string, string>& headers) {
    HttpResponse response;
    string fullUrl = baseUrl + endpoint;
    string responseBody;
    // CRITICAL: Reset CURL state
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    
    // Set headers
    struct curl_slist* headerList = nullptr;
    for (const auto& header : headers) {
        string headerStr = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerStr.c_str());
    }
    if (headerList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (headerList) {
        curl_slist_free_all(headerList);
    }
    
    if (res != CURLE_OK) {
        throw runtime_error("GET request failed: " + string(curl_easy_strerror(res)));
    }
    
    long statusCode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
    
    response.statusCode = static_cast<int>(statusCode);
    response.body = responseBody;
    
    return response;
}

HttpResponse HttpClient::post(const string& endpoint, const json& body, const map<string, string>& headers) {
    HttpResponse response;
    string fullUrl = baseUrl + endpoint;
    string responseBody;
    string requestBody = body.dump();
    
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    
    // Set headers (default to JSON)
    struct curl_slist* headerList = nullptr;
    headerList = curl_slist_append(headerList, "Content-Type: application/json");
    for (const auto& header : headers) {
        string headerStr = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerStr.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headerList);
    
    if (res != CURLE_OK) {
        throw runtime_error("POST request failed: " + string(curl_easy_strerror(res)));
    }
    
    long statusCode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
    
    response.statusCode = static_cast<int>(statusCode);
    response.body = responseBody;
    
    cout << "[HttpClient] POST " << endpoint << " -> " << statusCode << endl;
    
    return response;
}

HttpResponse HttpClient::put(const string& endpoint, const json& body, const map<string, string>& headers) {
    HttpResponse response;
    string fullUrl = baseUrl + endpoint;
    string responseBody;
    string requestBody = body.dump();
    
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    
    struct curl_slist* headerList = nullptr;
    headerList = curl_slist_append(headerList, "Content-Type: application/json");
    for (const auto& header : headers) {
        string headerStr = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerStr.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headerList);
    
    if (res != CURLE_OK) {
        throw runtime_error("PUT request failed: " + string(curl_easy_strerror(res)));
    }
    
    long statusCode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
    
    response.statusCode = static_cast<int>(statusCode);
    response.body = responseBody;
    
    return response;
}

HttpResponse HttpClient::del(const string& endpoint, const map<string, string>& headers) {
    HttpResponse response;
    string fullUrl = baseUrl + endpoint;
    string responseBody;
    
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    
    struct curl_slist* headerList = nullptr;
    for (const auto& header : headers) {
        string headerStr = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerStr.c_str());
    }
    if (headerList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (headerList) {
        curl_slist_free_all(headerList);
    }
    
    if (res != CURLE_OK) {
        throw runtime_error("DELETE request failed: " + string(curl_easy_strerror(res)));
    }
    
    long statusCode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
    
    response.statusCode = static_cast<int>(statusCode);
    response.body = responseBody;
    
    return response;
}
