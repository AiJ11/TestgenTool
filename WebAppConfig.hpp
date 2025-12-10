#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "ast.hpp"

/**
 * Configuration for a single web application
 * Contains all the information needed to generate tests for one app
 */
struct WebAppConfig {
    std::string appName;
    std::string description;
    int port = 8080;  // default port
    
    // Global variable declarations (name -> type specification)
    struct GlobalVar {
        std::string name;
        std::string type;  // "map", "set", "tuple", etc.
        std::string keyType;   // for maps
        std::string valueType; // for maps
        std::vector<std::string> tupleTypes; // for tuples
    };
    std::vector<GlobalVar> globals;
    
    // Initial values for globals
    std::map<std::string, std::string> initializations;
    
    // Function declarations
    struct FunctionSignature {
        std::string name;
        std::vector<std::string> paramTypes;
        HTTPResponseCode responseCode;
        std::vector<std::string> returnTypes;
    };
    std::vector<FunctionSignature> functions;
    
    // Available test scenarios (test strings)
    struct TestScenario {
        std::string id;
        std::string description;
        std::vector<std::string> apiSequence;  // e.g., ["signup", "login"]
        std::string builderFunction;  // e.g., "tourism_example1::example"
    };
    std::vector<TestScenario> testScenarios;
    
    // Test API configuration (for reading/writing global state)
    struct TestAPI {
        std::string globalName;   // e.g., "U"
        std::string getter;       // e.g., "get_U"
        std::string setter;       // e.g., "set_U"
    };
    std::vector<TestAPI> testAPIs;
    
    // Helper: Create TypeExpr from type string
    static std::unique_ptr<TypeExpr> createTypeExpr(const GlobalVar& var);
    
    // Helper: Create Decl from GlobalVar
    static std::unique_ptr<Decl> createDecl(const GlobalVar& var);
};

/**
 * Registry of all available web applications
 */
class WebAppRegistry {
public:
    // Register a new web app configuration
    void registerApp(const WebAppConfig& config);
    
    // Get config by app name
    const WebAppConfig* getConfig(const std::string& appName) const;
    
    // List all registered apps
    std::vector<std::string> listApps() const;
    
    // List all test scenarios for an app
    std::vector<std::string> listScenarios(const std::string& appName) const;
    
    // Load configurations from JSON file
    bool loadFromJSON(const std::string& jsonPath);
    
    // Save configurations to JSON file
    bool saveToJSON(const std::string& jsonPath) const;
    
private:
    std::map<std::string, WebAppConfig> apps;
};

/**
 * Builder class to programmatically construct WebAppConfig
 */
class WebAppConfigBuilder {
public:
    explicit WebAppConfigBuilder(const std::string& appName);
    
    WebAppConfigBuilder& withDescription(const std::string& desc);
    WebAppConfigBuilder& addMapGlobal(const std::string& name, 
                                      const std::string& keyType, 
                                      const std::string& valueType);
    WebAppConfigBuilder& addSetGlobal(const std::string& name, 
                                      const std::string& elementType);
    WebAppConfigBuilder& addTupleGlobal(const std::string& name, 
                                        const std::vector<std::string>& types);
    WebAppConfigBuilder& initializeGlobal(const std::string& name, 
                                          const std::string& value);
    WebAppConfigBuilder& addFunction(const std::string& name,
                                     const std::vector<std::string>& paramTypes,
                                     HTTPResponseCode responseCode,
                                     const std::vector<std::string>& returnTypes = {});
    WebAppConfigBuilder& addTestScenario(const std::string& id,
                                        const std::string& description,
                                        const std::vector<std::string>& apiSequence,
                                        const std::string& builderFunction);
    
    WebAppConfigBuilder& addTestAPI(const std::string& globalName,
                                     const std::string& getter,
                                     const std::string& setter);

    WebAppConfigBuilder& withPort(int port);                                 
    
    WebAppConfig build() const;
    
private:
    WebAppConfig config;
};
