// TestGenSymbolicEnv.hpp
#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <map>
#include <set>
#include <optional>
#include <ast.hpp>

struct WebAppConfig;

class TestGenSymbolicEnv {
public:
    struct MapInfo { 
        std::string domainArray;  // Array for domain membership
        std::string valueArray;   // Array for map values  
    };

    //Initialize from webconfig
    void initializeFromConfig(const WebAppConfig& config);

    // Map management for TestGen map variables (U, S, T, etc.)
    const MapInfo& declareMap(const std::string& mapName);
    bool isMap(const std::string& name) const;
    const MapInfo& getMapInfo(const std::string& name) const;
    // Find the declared map corresponding to a symbol (domain/value array or map symbol)
    // Resolve "v3", "Dom_T", "Val_U", etc. back to the map name ("T", "U"), if any
    std::optional<std::string> findMapForSymbol(const std::string &sym) const;


    // Variable symbol management
    const std::string& getSymbolFor(const std::string& varName);
    std::string createFreshSymbol(const std::string& tag = "fresh");

    // Constraint management
    void addConstraint(const std::string& constraint);
    void addInputVariable(const std::string& varName);

    // SMT-LIB generation
    std::string generateSMTLib(bool includeFooter = true) const;

    void setDebugMode(bool debug) { debugMode = debug; }
    bool isDebugMode() const { return debugMode; }


// return var -> symbol mapping
inline const std::unordered_map<std::string, std::string>& getVariableSymbols() const {
    return variableSymbols;
}

// return symbol -> var mapping
inline const std::unordered_map<std::string, std::string>& getSymbolToVar() const {
    return symbolToVar;
}

// return vector of SMT constraints (strings)
inline const std::vector<std::string>& getConstraints() const {
    return constraints;
}

// return insertion-order list of symbols (may be empty if you don't populate it)
inline const std::vector<std::string>& getSymbolOrder() const {
    return symbolOrder;
}

// return set of input variable names (vars that were flagged as inputs)
inline const std::set<std::string>& getInputVariables() const {
    return inputVariables;
}

// small helper: is var currently symbolic (no concrete binding in exprBindings)
inline bool isSymbolicVarName(const std::string &name) const {
    auto it = exprBindings.find(name);
    if (it == exprBindings.end()) {
        // no binding -> treat as symbolic (fresh input)
        return true;
    }
    // if bound to an AST Num or String then not symbolic; otherwise symbolic
    Expr* p = it->second.get();
    if (!p) return true;
    // check concrete types
    if (p->expressionType == ExpressionType::NUM || p->expressionType == ExpressionType::STRING) {
        return false;
    }
    return true;
}
    // Bind a solver symbol (model name) -> concrete string value back into exprBindings (AST)
    void bindSymbolValue(const std::string &symbol, const std::string &value);


    
    // Debug info
    void printDebugInfo() const;

    bool hasExpr(const std::string &name) const;
    Expr* getExprPtr(const std::string &name) const;
    std::unique_ptr<Expr> getExprClone(const std::string &name) const;
    void bindExpr(const std::string &name, std::unique_ptr<Expr> e);
    
private:
    // Symbol management
    std::unordered_map<std::string, std::string> variableSymbols;  // var -> symbol
    std::unordered_map<std::string, std::string> symbolToVar;      // symbol -> var
    std::vector<std::string> symbolOrder;
    std::set<std::string> inputVariables;
    int symbolCounter = 0;
    
    // Map information
    std::map<std::string, MapInfo> mapInfo;
    
    // Constraints
    std::vector<std::string> constraints;
    std::vector<std::string> constraintNames;
    int constraintCounter = 0;
    //  AST-bindings used by symbolic execution
    // Program-variable -> AST expression binding
    std::unordered_map<std::string, std::unique_ptr<Expr>> exprBindings;

    // Counter for creating fresh AST symvars: Var("__sym_1"), Var("__sym_2"), ...
    unsigned long freshSymVarCounter = 0;

    bool debugMode = false;
};
