#include "WebAppConfig.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>


// WebAppConfig Implementation
std::unique_ptr<TypeExpr> WebAppConfig::createTypeExpr(const GlobalVar& var) {
    if (var.type == "map") {
        auto keyType = std::make_unique<TypeConst>(var.keyType);
        auto valType = std::make_unique<TypeConst>(var.valueType);
        return std::make_unique<MapType>(std::move(keyType), std::move(valType));
    } else if (var.type == "set") {
        auto elemType = std::make_unique<TypeConst>(var.valueType);
        return std::make_unique<SetType>(std::move(elemType));
    } else if (var.type == "tuple") {
        std::vector<std::unique_ptr<TypeExpr>> types;
        for (const auto& t : var.tupleTypes) {
            types.push_back(std::make_unique<TypeConst>(t));
        }
        return std::make_unique<TupleType>(std::move(types));
    } else {
        // Simple type
        return std::make_unique<TypeConst>(var.type);
    }
}

std::unique_ptr<Decl> WebAppConfig::createDecl(const GlobalVar& var) {
    return std::make_unique<Decl>(var.name, createTypeExpr(var));
}
// WebAppRegistry Implementation

void WebAppRegistry::registerApp(const WebAppConfig& config) {
    apps[config.appName] = config;
}

const WebAppConfig* WebAppRegistry::getConfig(const std::string& appName) const {
    auto it = apps.find(appName);
    if (it == apps.end()) return nullptr;
    return &it->second;
}

std::vector<std::string> WebAppRegistry::listApps() const {
    std::vector<std::string> result;
    for (const auto& pair : apps) {
        result.push_back(pair.first);
    }
    return result;
}

std::vector<std::string> WebAppRegistry::listScenarios(const std::string& appName) const {
    auto* config = getConfig(appName);
    if (!config) return {};
    
    std::vector<std::string> result;
    for (const auto& scenario : config->testScenarios) {
        result.push_back(scenario.id);
    }
    return result;
}

bool WebAppRegistry::loadFromJSON(const std::string& jsonPath) {
    // TODO: Implement JSON parsing
    // For now, return false to indicate not implemented
    std::cerr << "JSON loading not yet implemented. Use programmatic registration." << std::endl;
    return false;
}

bool WebAppRegistry::saveToJSON(const std::string& jsonPath) const {
    // TODO: Implement JSON serialization
    std::cerr << "JSON saving not yet implemented." << std::endl;
    return false;
}
// WebAppConfigBuilder Implementation

WebAppConfigBuilder::WebAppConfigBuilder(const std::string& appName) {
    config.appName = appName;
}

WebAppConfigBuilder& WebAppConfigBuilder::withDescription(const std::string& desc) {
    config.description = desc;
    return *this;
}

WebAppConfigBuilder& WebAppConfigBuilder::addMapGlobal(
    const std::string& name,
    const std::string& keyType,
    const std::string& valueType) {
    
    WebAppConfig::GlobalVar var;
    var.name = name;
    var.type = "map";
    var.keyType = keyType;
    var.valueType = valueType;
    config.globals.push_back(var);
    return *this;
}

WebAppConfigBuilder& WebAppConfigBuilder::addSetGlobal(
    const std::string& name,
    const std::string& elementType) {
    
    WebAppConfig::GlobalVar var;
    var.name = name;
    var.type = "set";
    var.valueType = elementType;
    config.globals.push_back(var);
    return *this;
}

WebAppConfigBuilder& WebAppConfigBuilder::addTupleGlobal(
    const std::string& name,
    const std::vector<std::string>& types) {
    
    WebAppConfig::GlobalVar var;
    var.name = name;
    var.type = "tuple";
    var.tupleTypes = types;
    config.globals.push_back(var);
    return *this;
}

WebAppConfigBuilder& WebAppConfigBuilder::initializeGlobal(
    const std::string& name,
    const std::string& value) {
    
    config.initializations[name] = value;
    return *this;
}

WebAppConfigBuilder& WebAppConfigBuilder::addFunction(
    const std::string& name,
    const std::vector<std::string>& paramTypes,
    HTTPResponseCode responseCode,
    const std::vector<std::string>& returnTypes) {
    
    WebAppConfig::FunctionSignature func;
    func.name = name;
    func.paramTypes = paramTypes;
    func.responseCode = responseCode;
    func.returnTypes = returnTypes;
    config.functions.push_back(func);
    return *this;
}

WebAppConfigBuilder& WebAppConfigBuilder::addTestScenario(
    const std::string& id,
    const std::string& description,
    const std::vector<std::string>& apiSequence,
    const std::string& builderFunction) {
    
    WebAppConfig::TestScenario scenario;
    scenario.id = id;
    scenario.description = description;
    scenario.apiSequence = apiSequence;
    scenario.builderFunction = builderFunction;
    config.testScenarios.push_back(scenario);
    return *this;
}

WebAppConfigBuilder& WebAppConfigBuilder::addTestAPI(
    const std::string& globalName,
    const std::string& getter,
    const std::string& setter) {
    
    WebAppConfig::TestAPI testAPI;
    testAPI.globalName = globalName;
    testAPI.getter = getter;
    testAPI.setter = setter;
    config.testAPIs.push_back(testAPI);
    return *this;
}
//for port thing
WebAppConfigBuilder& WebAppConfigBuilder::withPort(int port) {
    config.port = port;
    return *this;
}

WebAppConfig WebAppConfigBuilder::build() const {
    return config;
}
