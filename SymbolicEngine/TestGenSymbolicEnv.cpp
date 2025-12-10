// TestGenSymbolicEnv.cpp
#include "TestGenSymbolicEnv.hpp"
#include <sstream>
#include <optional>
#include <iostream>
#include "WebAppConfig.hpp"


// NEW: Initialize from WebAppConfig

void TestGenSymbolicEnv::initializeFromConfig(const WebAppConfig& config) {
    // Declare maps from config globals
    for (const auto& globalVar : config.globals) {
        if (globalVar.type == "map") {
            declareMap(globalVar.name);
            if (debugMode) {
                std::cout << "[TestGenSymbolicEnv] Declared map from config: " 
                          << globalVar.name << std::endl;
            }
        }
    }
}


const TestGenSymbolicEnv::MapInfo& TestGenSymbolicEnv::declareMap(const std::string& mapName) {
    auto& info = mapInfo[mapName];
    if (info.domainArray.empty()) {
        info.domainArray = "Dom_" + mapName;
        info.valueArray = "Val_" + mapName;
    }
    return info;
}

    bool TestGenSymbolicEnv::isMap(const std::string& name) const { 
    return mapInfo.count(name) > 0; 
}

    const TestGenSymbolicEnv::MapInfo& TestGenSymbolicEnv::getMapInfo(const std::string& name) const { 
    return mapInfo.at(name); 
}

    std::optional<std::string> TestGenSymbolicEnv::findMapForSymbol(const std::string &sym) const {
    // 1) If sym exactly matches a declared map's valueArray or domainArray, return that map name.
    for (const auto &p : mapInfo) {
        const std::string &mapName = p.first;
        const MapInfo &mi = p.second;
        if (mi.valueArray == sym || mi.domainArray == sym) return mapName;
    }

    // 2) If sym is a variable-symbol that was created for a map name (variableSymbols[mapName] == sym)
    //    then return that mapName.
    for (const auto &p : mapInfo) {
        const std::string &mapName = p.first;
        auto it = variableSymbols.find(mapName);
        if (it != variableSymbols.end() && it->second == sym) return mapName;
    }

    // Not found
    return std::nullopt;
}

const std::string& TestGenSymbolicEnv::getSymbolFor(const std::string& varName) {
    auto [it, inserted] = variableSymbols.emplace(varName, "");
    if (inserted) {
        it->second = "v" + std::to_string(++symbolCounter);
        symbolOrder.push_back(it->second);
        symbolToVar[it->second] = varName;
    }
    return it->second;
}

std::string TestGenSymbolicEnv::createFreshSymbol(const std::string &tag) {
    // Increment counter first (not post-increment inside the string!)
    ++symbolCounter;

    // Create fresh symbol with tag
    std::ostringstream ss;
    ss << "__fresh_" << tag << "_" << symbolCounter;
    std::string sym = ss.str();

    // Record symbol → tag mapping
    // e.g.,  "__fresh_input_1" → "input"
    symbolToVar[sym] = tag;

    // Append to declaration order
    symbolOrder.push_back(sym);

    return sym;
}



void TestGenSymbolicEnv::addConstraint(const std::string& constraint) {
    if (!constraint.empty()) {
        constraints.push_back(constraint);
        constraintNames.push_back("c" + std::to_string(++constraintCounter));
    }
}

void TestGenSymbolicEnv::addInputVariable(const std::string& varName) {
    // If variable already has a symbol, just ensure it's registered as an input-tagged symbol.
    auto it = variableSymbols.find(varName);
    if (it != variableSymbols.end()) {
        const std::string &existingSym = it->second;
        // If symbolToVar already contains an "in_" tag for this sym, we're done.
        auto sit = symbolToVar.find(existingSym);
        if (sit != symbolToVar.end()) {
            if (sit->second.rfind("in_", 0) == 0) {
                inputVariables.insert(varName);
                return;
            } else {
                // Overwrite mapping to include in_ tag so genCTC can find it
                sit->second = std::string("in_") + varName;
                inputVariables.insert(varName);
                // ensure symbolOrder contains it (likely already does)
                if (std::find(symbolOrder.begin(), symbolOrder.end(), existingSym) == symbolOrder.end())
                    symbolOrder.push_back(existingSym);
                return;
            }
        }
        // fallthrough if symbolToVar missing: continue to create a new input symbol below
    }

    // Create a new dedicated input symbol (if none existed)
    ++symbolCounter;
    std::ostringstream ss;
    ss << "__in_" << varName << "_" << symbolCounter;
    std::string sym = ss.str();

    variableSymbols[varName] = sym;
    symbolToVar[sym] = std::string("in_") + varName; // tag this symbol as input for varName
    symbolOrder.push_back(sym);
    inputVariables.insert(varName);
}


std::string TestGenSymbolicEnv::generateSMTLib(bool includeFooter) const {
    std::ostringstream smt;
    
    //  Logic Declarations

    smt << "(set-logic ALL)\n";
    smt << "(set-option :produce-models true)\n\n";

    smt << ";; helper / uninterpreted declarations for predicates used by encoding\n";
    smt << "(declare-fun in (String (Array String Bool)) Bool)\n";
    smt << "(declare-fun add_to_set (String String) Bool)\n";
    smt << "(declare-fun not_empty (String) Bool)\n\n";

    // restaurant-specific uninterpreted predicates
    smt << "(declare-fun authenticated (String) Bool)\n";
    smt << "(declare-fun token_present (String) Bool)\n";
    smt << "(declare-fun cart_contains (String String) Bool)\n";
    smt << "(declare-fun order_recorded (String String) Bool)\n";
    smt << "(declare-fun result_is_restaurant_list () Bool)\n";
    smt << "(declare-fun result_is_menu () Bool)\n";
    smt << "(declare-fun result_is_cart () Bool)\n";
    smt << "(declare-fun review_added () Bool)\n\n";

    
    // Declare variables as strings

    smt << ";; Variable declarations (String type)\n";
    for (const auto& symbol : symbolOrder) {
        smt << "(declare-fun " << symbol << " () String)\n";
    }
    if (!symbolOrder.empty()) smt << "\n";
    
    // Declare maps as arrays
    smt << ";; Map declarations (Array String String/Bool)\n";
    
    for (const auto& [mapName, info] : mapInfo) {

        // mapInfo must be fully initialized
        if (info.domainArray.empty() || info.valueArray.empty()) {
            std::cerr << "[FATAL] TestGenSymbolicEnv::generateSMTLib(): "
                      << "mapInfo for '" << mapName << "' not initialized properly.\n"
                      << "  domainArray='" << info.domainArray << "' "
                      << " valueArray='" << info.valueArray << "'\n";
            std::abort();
        }

        smt << "; Map: " << mapName << "\n";
        smt << "(declare-const " << info.domainArray
            << " (Array String Bool))\n";
        smt << "(declare-const " << info.valueArray
            << " (Array String String))\n\n";
    }

    // (need to REMOVE THIS):
    // smt << ";; Initialize domain arrays to empty (all keys map to false)\n";
    // for (const auto& [mapName, info] : mapInfo) {
    //     smt << "(assert (= " << info.domainArray 
    //         << " ((as const (Array String Bool)) false)))\n";
    // }
    // smt << "\n";

    //  (LEAVE MAP ARRAYS UNCONSTRAINED INITIALLY)
    smt << ";; Domain arrays left unconstrained initially\n\n";
    
    // Input variable constraints
    if (!inputVariables.empty()) {
        smt << ";; Input variable constraints (non-empty)\n";
        for (const auto& inputVar : inputVariables) {
            auto it = variableSymbols.find(inputVar);
            if (it != variableSymbols.end()) {
                smt << "(assert (> (str.len " << it->second << ") 0))  ; " 
                    << inputVar << " must be non-empty\n";
            }
        }
        smt << "\n";
    }
    
    // Add constraints with names
    for (size_t i = 0; i < constraints.size(); ++i) {
        smt << "(assert (! " << constraints[i] << " :named " << constraintNames[i] << "))\n";
    }
    
    if (includeFooter) {
        smt << "\n(check-sat)\n";
        smt << "(get-model)\n";
    }
    
    return smt.str();
}



// AST-bindings used by symbolic execution
bool TestGenSymbolicEnv::hasExpr(const std::string &name) const {
    return exprBindings.find(name) != exprBindings.end();
}

Expr* TestGenSymbolicEnv::getExprPtr(const std::string &name) const {
    auto it = exprBindings.find(name);
    return (it == exprBindings.end()) ? nullptr : it->second.get();
}

std::unique_ptr<Expr> TestGenSymbolicEnv::getExprClone(const std::string &name) const {
    auto it = exprBindings.find(name);
    if (it == exprBindings.end()) return nullptr;
    return it->second->clone();
}

void TestGenSymbolicEnv::bindExpr(const std::string &name, std::unique_ptr<Expr> e) {
    exprBindings[name] = std::move(e);
}

// Bind solver symbol -> concrete value stored back into exprBindings for program vars
void TestGenSymbolicEnv::bindSymbolValue(const std::string &symbol, const std::string &value) {
    // Try to find the symbol -> varTag mapping
    auto it = symbolToVar.find(symbol);
    if (it == symbolToVar.end()) {
        // Unknown symbol: store it under the symbol name (use String literal)
        exprBindings[symbol] = std::make_unique<String>(value);
        return;
    }

    std::string varTag = it->second;
    // If varTag starts with "in_" we consider it input: plainVar = varTag.substr(3)
    std::string plainVar = varTag;
    if (plainVar.rfind("in_", 0) == 0) {
        plainVar = plainVar.substr(3);
    }

    // If looks like integer, store Num, otherwise String
    bool looksInt = !value.empty();
    for (char c : value) {
        if (!(std::isdigit(static_cast<unsigned char>(c)) || c == '-')) { looksInt = false; break; }
    }

    if (looksInt) {
        try {
            int v = std::stoi(value);
            exprBindings[plainVar] = std::make_unique<Num>(v);
        } catch(...) {
            exprBindings[plainVar] = std::make_unique<String>(value);
        }
    } else {
        exprBindings[plainVar] = std::make_unique<String>(value);
    }

    // Ensure variableSymbols contains an entry for plainVar (so getSymbolFor behavior remains consistent)
    if (variableSymbols.find(plainVar) == variableSymbols.end()) {
        variableSymbols[plainVar] = symbol;
    }
}


void TestGenSymbolicEnv::printDebugInfo() const {
    std::cout << "=== TestGen Symbolic Environment Debug ===" << std::endl;
    std::cout << "Variables: " << variableSymbols.size() << std::endl;
    for (const auto& [var, sym] : variableSymbols) {
        std::cout << "  " << var << " -> " << sym << std::endl;
    }
    std::cout << "Maps: " << mapInfo.size() << std::endl;
    for (const auto& [map, info] : mapInfo) {
        std::cout << "  " << map << " -> dom:" << info.domainArray 
                  << " val:" << info.valueArray << std::endl;
    }
    std::cout << "Constraints: " << constraints.size() << std::endl;
    for (size_t i = 0; i < constraints.size() && i < 5; ++i) {
        std::cout << "  [" << i << "] " << constraints[i] << std::endl;
    }
    if (constraints.size() > 5) {
        std::cout << "  ... and " << (constraints.size() - 5) << " more" << std::endl;
    }
}
