#include "TestGenDriver.hpp"
#include "WebAppConfig.hpp" 
#include "../config/executors/PesuFoodsExecutor.hpp"
#include "../config/executors/RestaurantExecutor.hpp"
#include "../APIFunction.hpp"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <regex>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <chrono>
#include "TestGenSymbolicExecutor.hpp"

class PesuFoodsFunctionFactory;
APIFunctionFactory* getPesuFoodsFunctionFactory();

class RestaurantFunctionFactory;
extern APIFunctionFactory* getRestaurantFunctionFactory();

//helpers :

static std::unique_ptr<Expr> makeConcreteExprFromStringLocal(const std::string &val) {
    bool looksLikeInt = !val.empty();
    for (char c : val) {
        if (!(std::isdigit(static_cast<unsigned char>(c)) || c == '-')) { looksLikeInt = false; break; }
    }
    if (looksLikeInt) {
        try {
            int v = std::stoi(val);
            return std::make_unique<Num>(v);
        } catch(...) { /* fall through to string */ }
    }
    return std::make_unique<String>(val);
}

static bool isInputStmtLocal(const Stmt &s, std::string &outVarName) {
    if (s.statementType != StatementType::INPUT) return false;
    const InputStmt &inp = static_cast<const InputStmt&>(s);
    if (!inp.var) return false;
    outVarName = inp.var->name;
    return true;
}

static std::unique_ptr<Stmt> cloneStmtLocal(const Stmt &s) {
    return s.clone();
}
static std::string getRealisticValueForVar(const std::string& varName, const std::string& z3Value) {
    // Counter for truly unique items (menu items, etc.)
    static int itemCounter = 0;
    static int restaurantCounter = 0;
    
    // ========================================
    // OWNER-SPECIFIC VARIABLES - FIXED VALUES
    // (Same owner used across scenarios 6-10)
    // ========================================
    
    // Owner Email - FIXED
    if (varName.find("ownerEmail") != std::string::npos) {
        return "owner@test.com";
    }
    
    // Owner Role - FIXED
    if (varName.find("ownerRole") != std::string::npos) {
        return "restaurant_owner";
    }
    
    // Owner Name - FIXED
    if (varName.find("ownerName") != std::string::npos) {
        return "TestOwner";
    }
    
    // Owner Mobile - FIXED
    if (varName.find("ownerMobile") != std::string::npos) {
        return "8123456789";
    }
    
    // ========================================
    // CUSTOMER VARIABLES - FIXED VALUES
    // (Same customer used across scenarios 1-5)
    // ========================================
    
    // Customer Email - FIXED
    if (varName.find("email") != std::string::npos) {
        return "customer@test.com";
    }
    
    // Password - FIXED
    if (varName.find("password") != std::string::npos) {
        return "Pass123!";
    }
    
    // Full name - FIXED
    if (varName.find("fullName") != std::string::npos || 
        varName.find("name") != std::string::npos) {
        return "TestUser";
    }
    
    // Mobile number - FIXED
    if (varName.find("mobile") != std::string::npos || 
        varName.find("phone") != std::string::npos) {
        return "9123456789";
    }
    
    // Role - FIXED (customer)
    if (varName.find("role") != std::string::npos) {
        return "customer";
    }
    
    // ========================================
    // RESTAURANT VARIABLES
    // ========================================
    
    // Restaurant Name - uses counter for uniqueness
    if (varName.find("restaurantName") != std::string::npos) {
        return "TestRestaurant_" + std::to_string(restaurantCounter++);
    }
    // restaurantId or just "restaurant" → MongoDB ObjectId
    if (varName.find("restaurantId") != std::string::npos) {
        return "507f1f77bcf86cd799439011";
    }
    
    // ========================================
    // MENU ITEM VARIABLES
    // ========================================
    
    // Item Name - UNIQUE per item
    if (varName.find("itemName") != std::string::npos) {
        return "TestItem_" + std::to_string(itemCounter++);
    }
    
    // Menu Item ID
    // menuItemId or just "item" → MongoDB ObjectId
    if (varName.find("menuItemId") != std::string::npos ||
        varName.find("menuItem") != std::string::npos) {
        return "507f191e810c19729de860ea";
    }
    
    // ========================================
    // ORDER VARIABLES
    // ========================================
    
    // Order Status
    if (varName.find("orderStatus") != std::string::npos ||
        varName.find("status") != std::string::npos) {
        return "accepted";
    }
    
    // Order ID
    if (varName.find("orderStatus") != std::string::npos ||
        varName.find("status") != std::string::npos) {
        return "accepted";
    }
    
    // ========================================
    // CART VARIABLES
    // ========================================
    
    // Quantity
    if (varName.find("quantity") != std::string::npos || 
        varName.find("Quantity") != std::string::npos) {
        return "2";
    }
    
    // ========================================
    // DELIVERY AGENT VARIABLES
    // ========================================
    
    // Agent ID
    if (varName.find("agent") != std::string::npos || 
        varName.find("Agent") != std::string::npos) {
        return "507f191e810c19729de860ec";
    }
    
    // ========================================
    // REVIEW VARIABLES
    // ========================================
    
    // Rating
    if (varName.find("rating") != std::string::npos || 
        varName.find("Rating") != std::string::npos) {
        return "5";
    }
    
    // Comment
    if (varName.find("comment") != std::string::npos || 
        varName.find("Comment") != std::string::npos) {
        return "Great food and excellent service!";
    }
    
    // ========================================
    // DEFAULT
    // ========================================
    return z3Value;
}
//generateConcreteTestCase implementation
TestGenDriver::TestResult TestGenDriver::generateConcreteTestCase(const Program& testGenProgram, const WebAppConfig& config,
                                                                 bool debugMode, bool executeAPIs) {
    TestResult result;
    result.satisfiable = false;

    try {
        // 1) Make a deep clone of the incoming Program so we can own it in genCTC
        std::vector<std::unique_ptr<Stmt>> clonedStmts;
        clonedStmts.reserve(testGenProgram.statements.size());
        for (const auto &sPtr : testGenProgram.statements) {
            if (!sPtr) continue;
            clonedStmts.push_back(sPtr->clone());
        }
        auto programCopy = std::make_unique<Program>(std::move(clonedStmts));

        // Call genCTC with config
        auto concreteProgram = TestGenDriver::genCTC(
            std::move(programCopy),
            config,  // Pass config
            /*initialL=*/{},
            debugMode
        );

        if (!concreteProgram) {
            result.errorMessage = "genCTC returned null (no concrete program produced)";
            return result;
        }

        // 3) Run a final symbolic visitor on the final concrete program to get final SMT + model
        TestGenSymbolicEnv finalEnv;
        //Initialize finalEnv from config
        finalEnv.initializeFromConfig(config);
        TestGenSymbolicVisitor finalVisitor(finalEnv, nullptr, debugMode);
        concreteProgram->accept(finalVisitor);

        // 4) Generate SMT-LIB and run Z3 to obtain final model (if any)
        result.smtContent = finalEnv.generateSMTLib(true);
        writeFile("final_constraints.smt2", result.smtContent);

        if (debugMode) {
            std::cout << "\n=== Final SMT-LIB ===\n";
            std::cout << result.smtContent.substr(0, std::min<size_t>(result.smtContent.size(), 1000));
            if (result.smtContent.size() > 1000) std::cout << "\n... (truncated)";
            std::cout << std::endl;
        }

        result.z3Output = runZ3("final_constraints.smt2");
        if (debugMode) {
            std::cout << "\n Final Z3 Output \n" << result.z3Output << std::endl;
        }

        if (result.z3Output.find("sat") != std::string::npos &&
            result.z3Output.find("unsat") == std::string::npos) {

            result.satisfiable = true;
            // Parse raw Z3 values first
            auto rawValues = parseZ3Model(result.z3Output, finalEnv);
            // Apply realistic values transformation
             result.concreteValues.clear();
             for (const auto &kv : rawValues) {
             std::string realisticValue = getRealisticValueForVar(kv.first, kv.second);
             result.concreteValues[kv.first] = realisticValue;
             }

            if (debugMode) {
    std::cout << "\n Concrete Test Values \n";
    for (const auto &kv : result.concreteValues) {
        std::cout << kv.first << " = " << kv.second;
        // Show original Z3 value if different
        auto it = rawValues.find(kv.first);
        if (it != rawValues.end() && it->second != kv.second) {
            std::cout << " [Z3: " << it->second << "]";
        }
        std::cout << std::endl;
    }
}
        } else if (result.z3Output.find("unsat") != std::string::npos) {
            result.errorMessage = "Final constraints are unsatisfiable";
        } else {
            result.errorMessage = "Unexpected Z3 output: " + result.z3Output;
        }

    } catch (const std::exception &e) {
        result.errorMessage = "Exception: " + std::string(e.what());
    }

    return result;
}


void TestGenDriver::writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot write file: " + filename);
    }
    file << content;
}

std::string TestGenDriver::runZ3(const std::string& smtFile) {
    const std::string outputFile = "z3_result.txt";
    //std::string command = "z3 -model " + smtFile + " > " + outputFile + " 2>&1";
    std::string command = "z3 -smt2 " + smtFile + " > " + outputFile + " 2>&1";

    
    int exitCode = std::system(command.c_str());
    if (exitCode != 0) {
        std::cerr << "Warning: Z3 exited with code " << exitCode << std::endl;
    }
    
    std::ifstream file(outputFile);
    if (!file) {
        return "Error: Could not read Z3 output";
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    return content;
}

std::map<std::string, std::string> TestGenDriver::parseZ3Model(const std::string& z3Output, 
                                                              const TestGenSymbolicEnv& env) {
    std::map<std::string, std::string> result;
    
    std::regex modelRegex("\\(define-fun\\s+(\\w+)\\s+\\(\\)\\s+String\\s+\"([^\"]*)\"\\)");
    std::smatch match;
    
    const auto& symbolToVar = env.getSymbolToVar();
    
    auto searchStart = z3Output.cbegin();
    while (std::regex_search(searchStart, z3Output.cend(), match, modelRegex)) {
        std::string symbol = match[1].str();
        std::string value = match[2].str();
        
        auto it = symbolToVar.find(symbol);
        if (it != symbolToVar.end()) {
            result[it->second] = value;
        } else {
            result[symbol] = value;
        }
        
        searchStart = match.suffix().first;
    }
    
    return result;
}

// Helper: Check if statement is an input call (FuncCallStmt with "input")
static bool isInputFuncCall(const Stmt &s, std::string &outVarName) {
    if (s.statementType != StatementType::FUNCTIONCALL_STMT) return false;
    
    const FuncCallStmt &fcs = static_cast<const FuncCallStmt&>(s);
    if (!fcs.call) return false;
    
    // Check if it's: input(varName)
    if (fcs.call->name == "input" && fcs.call->args.size() == 1) {
        if (auto* var = dynamic_cast<const Var*>(fcs.call->args[0].get())) {
            outVarName = var->name;
            return true;
        }
    }
    
    return false;
}

// ---------------- rewriteATC ----------------
std::unique_ptr<Program> TestGenDriver::rewriteATC(
        const Program &t,
        std::vector<std::string> &L,
        bool &error)
{
    error = false;

    // If no concrete inputs, return identical program
    if (L.empty()) {
        std::vector<std::unique_ptr<Stmt>> out;
        out.reserve(t.statements.size());
        for (const auto &stmtPtr : t.statements) {
            out.push_back(cloneStmtLocal(*stmtPtr));
        }
        return std::make_unique<Program>(std::move(out));
    }

    std::vector<std::unique_ptr<Stmt>> out;
    out.reserve(t.statements.size());

    for (const auto &stmtPtr : t.statements) {
        const Stmt &s = *stmtPtr;
        std::string varName;

        // Handle InputStmt: "input(x)"
        if (isInputStmtLocal(s, varName)) {
            if (L.empty()) {
                std::cerr << "[rewriteATC] ERROR: Missing concrete value for input " 
                          << varName << "\n";
                error = true;
                return nullptr;
            }
            std::string val = L.front();
            L.erase(L.begin());

            out.push_back(
                std::make_unique<Assign>(
                    std::make_unique<Var>(varName),
                    makeConcreteExprFromStringLocal(val)
                )
            );
        }
        // Handle "input(expr)" inside function calls
        else if (isInputFuncCall(s, varName)) {
            if (!L.empty()) {
                std::string val = L.front();
                L.erase(L.begin());

                out.push_back(
                    std::make_unique<Assign>(
                        std::make_unique<Var>(varName),
                        makeConcreteExprFromStringLocal(val)
                    )
                );
            } 
            else {
                out.push_back(cloneStmtLocal(s));
            }
        }
        else {
            out.push_back(cloneStmtLocal(s));
        }
    }

    if (!L.empty()) {
        std::cerr << "[rewriteATC] WARNING: leftover values in L = " 
                  << L.size() << "\n";
    }

    return std::make_unique<Program>(std::move(out));
}




// ---------------- genCTC ----------------
std::unique_ptr<Program> TestGenDriver::genCTC(std::unique_ptr<Program> t, const WebAppConfig& config,
                                               std::vector<std::string> initialL,
                                               bool debugMode, bool executeAPIs) {
    if (!t) return nullptr;

    TestGenSymbolicEnv env;
    env.initializeFromConfig(config);
    const int MAX_ITERS = 50;
    std::vector<std::string> L = std::move(initialL);

    // Create function factory based on webapp
    APIFunctionFactory* factory = nullptr;
    
    if (config.appName == "pesu_foods") {
       factory = getPesuFoodsFunctionFactory();  // Call the extern function
        
        // Initialize backend
        PesuFoodsExecutor::setPort(config.port);
        
        if (debugMode) {
            std::cout << "[genCTC] Initializing PESU Foods backend on port " 
                      << config.port << std::endl;
        }
        
        // Check health and reset
        if (PesuFoodsExecutor::checkHealth()) {
            PesuFoodsExecutor::resetTestData();
            if (debugMode) std::cout << "[genCTC] Backend reset complete\n";
        } else {
            std::cerr << "[genCTC] Warning: Backend not responding on port " 
                      << config.port << std::endl;
            std::cerr << "[genCTC] Make sure to run: PORT=" << config.port 
                      << " node authServer.js" << std::endl;
        }
    }
    // Add more webapps here as needed
    // else if (config.appName == "library") { ... }

    else if (config.appName == "restaurant") {
    factory = getRestaurantFunctionFactory();
    
    // Initialize backend
    RestaurantExecutor::setPort(config.port);
    
    if (debugMode) {
        std::cout << "[genCTC] Initializing Restaurant backend on port " 
                  << config.port << std::endl;
    }
    
    // Check health and reset
    if (RestaurantExecutor::checkHealth()) {
        RestaurantExecutor::resetTestData();
        if (debugMode) std::cout << "[genCTC] Backend reset complete\n";
    } else {
        std::cerr << "[genCTC] Warning: Restaurant backend not responding on port " 
                  << config.port << std::endl;
        std::cerr << "[genCTC] Make sure to run: PORT=" << config.port 
                  << " npm start" << std::endl;
    }
}
else {
    std::cerr << "[genCTC] Warning: Unknown webapp '" << config.appName 
              << "' - no executor configured\n";
}

    for (int iter = 0; iter < MAX_ITERS; ++iter) {
        if (debugMode) std::cout << "[genCTC] iteration " << iter << ", L.size=" << L.size() << "\n";

        // Rewrite ATC (to handle empty L by returning unchanged)
        bool rewriteErr = false;
        auto tprime = rewriteATC(*t, L, rewriteErr);
        if (!tprime) {
            if (debugMode) std::cout << "[genCTC] rewriteATC failed; returning original program\n";
            return t;
        }

        // Run symbolic execution
        env = TestGenSymbolicEnv();  // Reset
        env.initializeFromConfig(config);
        TestGenSymbolicVisitor visitor(env, factory, debugMode, executeAPIs);  // Pass factory!
        tprime->accept(visitor);

        // generate SMT-LIB
        std::string smt = env.generateSMTLib(true);
        if (debugMode) std::cout << "[genCTC] SMT-LIB length: " << smt.size() << "\n";

        // write and call Z3
        writeFile("genctc_query.smt2", smt);
        std::string z3out = runZ3("genctc_query.smt2");
        if (debugMode) std::cout << "[genCTC] z3 output:\n" << z3out << "\n";

        // Check for UNSAT
        if (z3out.find("unsat") != std::string::npos) {
            if (debugMode) std::cout << "[genCTC] UNSAT - test string is infeasible!\n";
            return nullptr;
        }

        // parse model
        auto model = parseZ3Model(z3out, env);
        if (model.empty()) {
            if (z3out.find("unsat") != std::string::npos) {
                if (debugMode) std::cout << "[genCTC] UNSAT returned by Z3\n";
            } else {
                if (debugMode) std::cout << "[genCTC] Z3 returned no model\n";
            }
            return tprime;
        }

        // --- Bind model values back into the symbolic environment so exprBindings are populated ---
        // model maps either symbol -> value or "in_var" tag -> value (depending on parseZ3Model).
        // We'll call env.bindSymbolValue for each model entry so the env holds concrete AST nodes.
        for (const auto &p : model) {
        const std::string &k = p.first;
        const std::string &v = p.second;

        std::string realisticValue = getRealisticValueForVar(k, v);
        
        env.bindSymbolValue(k, realisticValue);
        if (debugMode) {
            if (realisticValue != v) {
            std::cout << "[genCTC] bindSymbolValue(" << k 
                      << ", \"" << realisticValue << "\") [Z3: " << v << "]\n";
        } else {
            std::cout << "[genCTC] bindSymbolValue(" << k << ", \"" << v << "\")\n";
        }
    }
}


        // construct newL by selecting symbols tagged as inputs
        // ---------- robust newL construction ----------
       std::vector<std::string> newL;

// get env views
const auto &varToSym = env.getVariableSymbols();   // var -> symbol
const auto &symToVar = env.getSymbolToVar();      // symbol -> var/tag
const auto &inputVars = env.getInputVariables();  // set of input var names (plain)
const auto &symOrder = env.getSymbolOrder();      // insertion order

// Iterate symbols in order
if (!symOrder.empty()) {
    for (const auto &sym : symOrder) {
        auto itVar = symToVar.find(sym);
        if (itVar == symToVar.end()) continue;
        
        std::string varTag = itVar->second;
        
        // Check if this is an input variable (tagged with "in_" prefix)
        if (varTag.rfind("in_", 0) == 0) {
            // Strip the "in_" prefix to get the plain variable name
            std::string plainVar = varTag.substr(3);
            
            // Verify it's in inputVars set
            if (inputVars.find(plainVar) == inputVars.end()) continue;
            
            // Try to find value in model by symbol name first
            auto mit = model.find(sym);
            if (mit != model.end()) {
                newL.push_back(mit->second);
                if (debugMode) {
                    std::cout << "[genCTC] Added to newL: " << plainVar 
                              << " (sym=" << sym << ") = " << mit->second << "\n";
                }
            } else {
                // Fallback: try plain var name or varTag
                auto mit2 = model.find(plainVar);
                if (mit2 != model.end()) {
                    newL.push_back(mit2->second);
                    if (debugMode) {
                        std::cout << "[genCTC] Added to newL: " << plainVar 
                                  << " = " << mit2->second << "\n";
                    }
                } else {
                    auto mit3 = model.find(varTag);
                    if (mit3 != model.end()) {
                        newL.push_back(mit3->second);
                        if (debugMode) {
                            std::cout << "[genCTC] Added to newL: " << varTag 
                                      << " = " << mit3->second << "\n";
                        }
                    }
                }
            }
        }
    }
} else {
    // Fallback: iterate inputVars directly
    for (const auto &plainVar : inputVars) {
        // Find the symbol for this input variable
        auto itSym = varToSym.find(plainVar);
        if (itSym == varToSym.end()) continue;
        
        const std::string &sym = itSym->second;
        
        // Try to find value in model
        auto mit = model.find(sym);
        if (mit != model.end()) {
            newL.push_back(mit->second);
        } else {
            // Fallback to plain var name
            auto mit2 = model.find(plainVar);
            if (mit2 != model.end()) {
                newL.push_back(mit2->second);
            }
        }
    }
}

if (debugMode) std::cout << "[genCTC] newL size: " << newL.size() << "\n";

        if (newL.empty()) {
            if (debugMode) std::cout << "[genCTC] no input symbols found in the model; returning tprime\n";
            return tprime;
        }

        if (newL == L) {
            if (debugMode) std::cout << "[genCTC] no progress in L; returning tprime\n";
            return tprime;
        }

        t = std::move(tprime);
        L = std::move(newL);

        if (L.empty()) return t;
    }

    std::cerr << "[genCTC] reached iteration limit; returning current program\n";
    return t;
}
//demonstrateTestGenSymbolicEngine implementation
void TestGenDriver::demonstrateTestGenSymbolicEngine() {
    std::cout << "Demo function not available in standalone mode" << std::endl;
}
