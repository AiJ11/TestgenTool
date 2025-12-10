// ModularTestGen.hpp
#pragma once
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include "WebAppConfig.hpp"
#include "ast.hpp"
#include "symbol_table.hpp"

class ModularTestGen {
public:
    static std::pair<std::unique_ptr<Spec>, SymbolTable> generateSpec(
        const WebAppConfig& config,
        const std::string& scenarioId = ""
    );

    static std::pair<std::unique_ptr<Spec>, SymbolTable> generateSpecWithBuilders(
        const WebAppConfig& config,
        const std::vector<std::function<void(std::vector<std::unique_ptr<API>>&, SymbolTable&)>>& apiBuilders
    );

private:
    static std::vector<std::unique_ptr<Decl>> createGlobals(const WebAppConfig& config);
    static std::vector<std::unique_ptr<Init>> createInits(const WebAppConfig& config);
    static std::vector<std::unique_ptr<FuncDecl>> createFuncDecls(const WebAppConfig& config);
    static SymbolTable createRootSymbolTable(const WebAppConfig& config);
    static void executeScenarioBuilder(const WebAppConfig& config,
                                       const std::string& scenarioId,
                                       std::vector<std::unique_ptr<API>>& apis,
                                       SymbolTable& symtable);
};

// APIBuilderFactory + registration macro
class APIBuilderFactory {
public:
    using BuilderFunc = std::function<void(std::vector<std::unique_ptr<API>>&, SymbolTable&)>;
    static void registerBuilder(const std::string& name, BuilderFunc builder);
    static BuilderFunc getBuilder(const std::string& name);
    static bool hasBuilder(const std::string& name);
private:
    static std::map<std::string, BuilderFunc>& getRegistry();
};

#define REGISTER_API_BUILDER(name, builderClass) \
    namespace { \
        struct name##_registrar { \
            name##_registrar() { \
                APIBuilderFactory::registerBuilder(#name, \
                    [](std::vector<std::unique_ptr<API>>& apis, SymbolTable& root) { \
                        builderClass::example(apis, root); \
                    }); \
            } \
        }; \
        static name##_registrar name##_reg_instance; \
    }
