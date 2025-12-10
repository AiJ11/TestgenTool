// ModularTestGen.cpp
#include "ModularTestGen.hpp"
#include <stdexcept>
#include <iostream>

using namespace std;

std::pair<std::unique_ptr<Spec>, SymbolTable> ModularTestGen::generateSpec(
    const WebAppConfig& config,
    const std::string& scenarioId) {

    auto globals = createGlobals(config);
    auto inits = createInits(config);
    auto funcDecls = createFuncDecls(config);
    SymbolTable symtable = createRootSymbolTable(config);

    std::vector<std::unique_ptr<API>> apis;

    if (scenarioId.empty()) {
        for (const auto& scenario : config.testScenarios) {
            executeScenarioBuilder(config, scenario.id, apis, symtable);
        }
    } else {
        executeScenarioBuilder(config, scenarioId, apis, symtable);
    }

    auto spec = std::make_unique<Spec>(std::move(globals), std::move(inits), std::move(funcDecls), std::move(apis));
    return std::make_pair(std::move(spec), std::move(symtable));
}

std::pair<std::unique_ptr<Spec>, SymbolTable> ModularTestGen::generateSpecWithBuilders(
    const WebAppConfig& config,
    const std::vector<std::function<void(std::vector<std::unique_ptr<API>>&, SymbolTable&)>>& apiBuilders) {

    auto globals = createGlobals(config);
    auto inits = createInits(config);
    auto funcDecls = createFuncDecls(config);
    SymbolTable symtable = createRootSymbolTable(config);
    std::vector<std::unique_ptr<API>> apis;
    for (const auto& builder : apiBuilders) builder(apis, symtable);
    auto spec = std::make_unique<Spec>(std::move(globals), std::move(inits), std::move(funcDecls), std::move(apis));
    return std::make_pair(std::move(spec), std::move(symtable));
}

std::vector<std::unique_ptr<Decl>> ModularTestGen::createGlobals(const WebAppConfig& config) {
    std::vector<std::unique_ptr<Decl>> globals;
    for (const auto& g : config.globals) globals.push_back(WebAppConfig::createDecl(g));
    return globals;
}

std::vector<std::unique_ptr<Init>> ModularTestGen::createInits(const WebAppConfig& config) {
    std::vector<std::unique_ptr<Init>> inits;
    for (const auto& kv : config.initializations) {
        // Default: empty map initializer (AST Map node)
        auto emptyMap = std::make_unique<Map>(std::vector<std::pair<std::unique_ptr<Var>, std::unique_ptr<Expr>>>());
        inits.push_back(std::make_unique<Init>(kv.first, std::move(emptyMap)));
    }
    return inits;
}

std::vector<std::unique_ptr<FuncDecl>> ModularTestGen::createFuncDecls(const WebAppConfig& config) {
    std::vector<std::unique_ptr<FuncDecl>> out;
    for (const auto& f : config.functions) {
        std::vector<std::unique_ptr<TypeExpr>> params;
        for (const auto& p : f.paramTypes) params.push_back(std::make_unique<TypeConst>(p));
        std::vector<std::unique_ptr<TypeExpr>> ret;
        for (const auto& r : f.returnTypes) ret.push_back(std::make_unique<TypeConst>(r));
        out.push_back(std::make_unique<FuncDecl>(f.name, std::move(params), std::make_pair(f.responseCode, std::move(ret))));
    }
    return out;
}

SymbolTable ModularTestGen::createRootSymbolTable(const WebAppConfig& config) {
    SymbolTable root;
    for (const auto& g : config.globals) {
        root.symtable.insert(Var(g.name));
    }
    return root;
}

void ModularTestGen::executeScenarioBuilder(
    const WebAppConfig& config,
    const std::string& scenarioId,
    std::vector<std::unique_ptr<API>>& apis,
    SymbolTable& symtable) {

    const WebAppConfig::TestScenario* scenario = nullptr;
    for (const auto& s : config.testScenarios) if (s.id == scenarioId) { scenario = &s; break; }
    if (!scenario) throw std::runtime_error("Test scenario not found: " + scenarioId);
    if (!APIBuilderFactory::hasBuilder(scenario->builderFunction))
        throw std::runtime_error("Builder function not registered: " + scenario->builderFunction);
    auto builder = APIBuilderFactory::getBuilder(scenario->builderFunction);
    builder(apis, symtable);
}

// APIBuilderFactory implementations
std::map<std::string, APIBuilderFactory::BuilderFunc>& APIBuilderFactory::getRegistry() {
    static std::map<std::string, BuilderFunc> registry;
    return registry;
}
void APIBuilderFactory::registerBuilder(const std::string &name, BuilderFunc b) { getRegistry()[name] = b; }
APIBuilderFactory::BuilderFunc APIBuilderFactory::getBuilder(const std::string &name) {
    auto &r = getRegistry();
    auto it = r.find(name);
    if (it == r.end()) throw std::runtime_error("Builder not found: " + name);
    return it->second;
}
bool APIBuilderFactory::hasBuilder(const std::string &name) {
    return getRegistry().find(name) != getRegistry().end();
}
