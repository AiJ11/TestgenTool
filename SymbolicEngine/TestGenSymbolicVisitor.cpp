// TestGenSymbolicVisitor.cpp
#include "TestGenSymbolicVisitor.hpp"
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <cctype>         // for std::isalpha / std::isdigit


// --- SMT helper utilities --------------------------------
static bool isSmtBuiltin(const std::string &name) {
   static const std::unordered_set<std::string> builtins = {
       // Core SMT operators / keywords we want to emit verbatim:
       "=", "and", "or", "not", "select", "store", "Array", "as", "const",
       "declare-const", "declare-fun", "define-fun", "set-logic", "str.len",
       "String", "Bool", "Int", "true", "false"
   };
   // also allow common names that are safe and meaningful in your encoding:
   static const std::unordered_set<std::string> safe_names = {
       "add_to_set", "not_empty", "in", "inMapVerify", "mapAccess"
       //"Val_T", "Val_U", "Dom_T", "Dom_U",
       //"Val_S", "Dom_S"
   };
   if (builtins.count(name)) return true;
   if (safe_names.count(name)) return true;
   // Also accept identifiers that look like valid bare symbols: start with letter/underscore and only alnum/_/-
   if (!name.empty() && (std::isalpha((unsigned char)name[0]) || name[0] == '_')) {
       bool ok = true;
       for (char c : name) {
           if (!(std::isalnum((unsigned char)c) || c == '_' || c == '-')) { ok = false; break; }
       }
       if (ok) return true;
   }
   return false;
}


static std::string sanitizeSmtIdent(const std::string &name) {
   if (isSmtBuiltin(name)) return name;
   // If the name looks like a number, return as-is
   bool allDigits = !name.empty();
   for (char c : name) if (!std::isdigit((unsigned char)c)) { allDigits = false; break; }
   if (allDigits) return name;
   // Otherwise quote with |...| escaping internal '|'
   std::string out = "|";
   for (char c : name) {
       if (c == '|') out += "\\|"; else out.push_back(c);
   }
   out += "|";
   return out;
}


static std::string escapeStringLiteral(const std::string &s) {
   std::string out;
   for (char c : s) {
       if (c == '\\') out += "\\\\";
       else if (c == '"') out += "\\\"";
       else out.push_back(c);
   }
   return out;
}

static std::string getRealisticValueForVar(const std::string& varName, const std::string& symbolicValue) {
    if (varName.find("email") != std::string::npos) return "test@example.com";
    if (varName.find("password") != std::string::npos) return "Pass123!";
    if (varName.find("fullName") != std::string::npos) return "TestUser";
    if (varName.find("mobile") != std::string::npos) return "1234567890";
    if (varName.find("role") != std::string::npos) return "customer";
    if (varName.find("restaurant") != std::string::npos || varName.find("Restaurant") != std::string::npos) 
        return "507f1f77bcf86cd799439011";
    if (varName.find("item") != std::string::npos || varName.find("Item") != std::string::npos) 
        return "507f191e810c19729de860ea";
    if (varName.find("order") != std::string::npos || varName.find("Order") != std::string::npos) 
        return "507f191e810c19729de860eb";
    if (varName.find("quantity") != std::string::npos || varName.find("Quantity") != std::string::npos) 
        return "2";
    if (varName.find("rating") != std::string::npos) return "5";
    if (varName.find("comment") != std::string::npos) return "Great service";
    return symbolicValue; // Return original if no match
}


void TestGenSymbolicVisitor::visit(const Assign& n) {
   // Handle assignments like: x = input() or x = fresh()
   if (!n.left) return;
   const std::string varName = n.left->name;


   // No hardcoded map list
   // Maps are now declared via env.initializeFromConfig()

   // Inputs: x = input()
   if (auto* funcCall = dynamic_cast<const FuncCall*>(n.right.get())) {
       if (funcCall->name == "input" && funcCall->args.empty()) {
           env.addInputVariable(varName);
           if (debugMode) {
               std::cout << "[TestGenSymbolic] Input variable: " << varName << std::endl;
           }
           return;
       }
   }


   // ---- Tightened heuristic for declaring maps at AST-time ----
   // Only declare a map here if it clearly *is* a map:
   //  - exact known map names (e.g., U, S, T, orders, tourDetails, cart, Tours, TD, tokenVal, tourList)
   //  - or if it ends with "_old" and the base name is a known map.
  /* static const std::unordered_set<std::string> knownMaps = {
       "U", "S", "T", "orders", "tourDetails"
   };


   if (!varName.empty()) {
       // Case A: exact known map name
       if (knownMaps.count(varName) && !env.isMap(varName)) {
           env.declareMap(varName);
           if (debugMode) std::cout << "[TestGenSymbolic] Declared map: " << varName << std::endl;
       }
       // Case B: trailing "_old" variant -> declare base only if base is known
       else if (varName.size() > 4 && varName.rfind("_old") == varName.size() - 4) {
           std::string base = varName.substr(0, varName.size() - 4);
           if (knownMaps.count(base) && !env.isMap(base)) {
               env.declareMap(base);
               if (debugMode) std::cout << "[TestGenSymbolic] Declared map (from _old): " << base << std::endl;
           }
       }
   }


   // Inputs: x = input()
   if (auto* funcCall = dynamic_cast<const FuncCall*>(n.right.get())) {
       if (funcCall->name == "input" && funcCall->args.empty()) {
           // This is an input variable declaration
           env.addInputVariable(varName);
           if (debugMode) {
               std::cout << "[TestGenSymbolic] Input variable: " << varName << std::endl;
           }
           return;
       }
   }*/


   // Other assignments - convert RHS to SMT and add equality between symbol and RHS-SMT.
   std::string rhsSMT = convertExprToSMT(*n.right);
   std::string lhsSymbol = env.getSymbolFor(varName);
   env.addConstraint("(= " + lhsSymbol + " " + rhsSMT + ")");
   if (debugMode) {
       std::cout << "[TestGenSymbolic] Assign -> " << lhsSymbol << " = " << rhsSMT << std::endl;
   }
  


   // IMPORTANT: Bind concrete values to variables so isConcreteExpr works!


   if (dynamic_cast<const String*>(n.right.get()) || dynamic_cast<const Num*>(n.right.get())) {


       // RHS is a concrete literal - bind it to BOTH the variable name and its symbol


       env.bindExpr(varName, n.right->clone());


       env.bindExpr(lhsSymbol, n.right->clone());  // Also bind to symbol (v1, v2, etc.)


       if (debugMode) {


           std::cout << "[TestGenSymbolic] Bound concrete value to " << varName


                     << " (symbol: " << lhsSymbol << ")" << std::endl;


       }


   }


}




void TestGenSymbolicVisitor::visit(const FuncCallStmt& n) {
   if (!n.call) return;


   const std::string& funcName = n.call->name;


   if (funcName == "assume" && !n.call->args.empty()) {
       // Convert assumption to SMT constraint
       std::string constraint = convertExprToSMT(*n.call->args[0]);
       env.addConstraint(constraint);


       if (debugMode) {
           std::cout << "[TestGenSymbolic] Added assumption: " << constraint << std::endl;
       }
   }
   else if (funcName == "assert" && !n.call->args.empty()) {
       // Convert assertion to SMT constraint
       std::string constraint = convertExprToSMT(*n.call->args[0]);
       env.addConstraint(constraint);


       if (debugMode) {
           std::cout << "[TestGenSymbolic] Added assertion: " << constraint << std::endl;
       }
   }
   else if (funcName == "input" && n.call->args.size() == 1) {
       // Handle input statements: input(varName)
       if (auto* var = dynamic_cast<const Var*>(n.call->args[0].get())) {
           env.addInputVariable(var->name);
           if (debugMode) {
               std::cout << "[TestGenSymbolic] Input variable: " << var->name << std::endl;
           }
       }
   }


   else if (funcName == "=" && n.call->args.size() == 2) {

    // Case 1: Both LHS and RHS are variables
    if (auto* lhs = dynamic_cast<const Var*>(n.call->args[0].get())) {
        if (auto* rhs = dynamic_cast<const Var*>(n.call->args[1].get())) {

            bool lhsIsOld = (lhs->name.size() > 4 &&
                            lhs->name.substr(lhs->name.size() - 4) == "_old");

            // Case A: Backup assignment like U_old = U or T_old = T
            if (lhsIsOld && env.isMap(rhs->name)) {
                // Only ensure maps are declared — NO equality constraint in SMT
                env.declareMap(lhs->name);
                env.declareMap(rhs->name);

                if (debugMode) {
                    std::cout << "[SMT] Backup assign detected — no equality added: "
                              << lhs->name << " := " << rhs->name << std::endl;
                }
                return;
            }

            // Case B: Normal map equality (not backup)
            if (env.isMap(lhs->name) && env.isMap(rhs->name)) {
                const auto& L = env.declareMap(lhs->name);
                const auto& R = env.declareMap(rhs->name);
                std::string c = "(and (= " + L.domainArray + " " + R.domainArray + ")"
                                     " (= " + L.valueArray  + " " + R.valueArray  + "))";
                env.addConstraint(c);
                if (debugMode) {
                    std::cout << "[SMT] Map equality constraint: " << c << std::endl;
                }
                return;
            }
        }
    }

    // Case 2: Fallback — scalar or mixed expressions
    std::string c = "(= " + convertExprToSMT(*n.call->args[0])
                        + " " + convertExprToSMT(*n.call->args[1]) + ")";
    env.addConstraint(c);

    if (debugMode) {
        std::cout << "[SMT] Scalar equality constraint: " << c << std::endl;
    }
    return;
}

 // HANDLE API CALLS - CHECK IF READY AND EXECUTE!
   else if (funcName != "assume" && funcName != "assert" && funcName != "input") {
       // This is potentially an API call
       if (isAPI(funcName)) {
           // Step 1: Check if all arguments are CONCRETE (ready)
           bool allArgsReady = true;
          
           for (const auto& arg : n.call->args) {
               if (!isConcreteExpr(*arg)) {
                   allArgsReady = false;
                   break;
               }
              
           }
          
           if (allArgsReady) {
    // ALL ARGUMENTS ARE CONCRETE - EXECUTE THE API!
    // Clone arguments as Expr objects AND apply realistic values
    std::vector<std::unique_ptr<Expr>> argExprs;
    for (const auto& arg : n.call->args) {
        // Resolve variables to their concrete values
        if (auto* var = dynamic_cast<const Var*>(arg.get())) {
            if (env.hasExpr(var->name)) {
                // CASE 1: Variable has a bound expression
                auto boundExpr = env.getExprClone(var->name);
                
                // Apply realistic value transformation for String types
                if (auto* strExpr = dynamic_cast<String*>(boundExpr.get())) {
                    std::string realistic = getRealisticValueForVar(var->name, strExpr->value);
                    if (realistic != strExpr->value && debugMode) {
                        std::cout << "[API] Transforming " << var->name 
                                  << ": \"" << strExpr->value << "\" → \"" << realistic << "\"" << std::endl;
                    }
                    argExprs.push_back(std::make_unique<String>(realistic));
                } else {
                    // Non-string types: use as-is
                    argExprs.push_back(std::move(boundExpr));
                }
            } else {
                //CASE 2: Variable NOT bound - clone the Var itself
                argExprs.push_back(cloneExpr(arg.get()));
            }
        } else {
            // CASE 3: Not a variable (literal String/Num) - clone directly
            argExprs.push_back(cloneExpr(arg.get()));
        }
    }
               if (debugMode) {
                   std::cout << "[TestGenSymbolic] Executing API: " << funcName << std::endl;
                  
               }
              
               //  Call with Expr vector
               try {
                   std::cout << "[DEBUG] About to call executeAPI" << std::endl;
                   std::cout.flush();
                   // Safety check before calling

                    bool allValid = true;

                    for (size_t i = 0; i < argExprs.size(); ++i) {

                        if (!argExprs[i]) {

                            std::cerr << "[ERROR] arg[" << i << "] is NULL!" << std::endl;

                            allValid = false;

                        } else {

                            std::cout << "[CHECK] arg[" << i << "] type: ";

                            if (dynamic_cast<String*>(argExprs[i].get())) {

                                std::cout << "String" << std::endl;

                            } else if (dynamic_cast<Var*>(argExprs[i].get())) {

                                std::cout << "Var (NOT RESOLVED!)" << std::endl;

                                allValid = false;

                            } else {

                                std::cout << "Other" << std::endl;

                            }

                        }

                    }

                    if (allValid) {

                        executeAPI(funcName, argExprs, n.call->args);

                    } else {

                        std::cerr << "[ERROR] Arguments not properly resolved!" << std::endl;

                    }
                   std::cout << "[DEBUG] executeAPI completed" << std::endl;
                   std::cout.flush();
               } catch (const std::exception& e) {
                   std::cerr << "[ERROR] executeAPI threw exception: " << e.what() << std::endl;
               } catch (...) {
                   std::cerr << "[ERROR] executeAPI threw unknown exception" << std::endl;
               }
              
               // API executed - continue symbolic execution
               // Don't stop here!
           } else {
               // Arguments still symbolic - NOT READY
               if (debugMode) {
                   std::cout << "[TestGenSymbolic]  API not ready: " << funcName
                             << " (arguments still symbolic)" << std::endl;
               }
               // Set flag to stop execution
               stopExecution = true;
           }
       } else {
           // Native helper function
           if (debugMode) {
               std::cout << "[TestGenSymbolic] Native function: " << funcName << std::endl;
           }
       }
   }
}


//assume, assert , input stmts


// Handle dedicated AssumeStmt node
void TestGenSymbolicVisitor::visit(const AssumeStmt &s) {
   if (!s.condition) return;
   std::string c = convertExprToSMT(*s.condition);
   env.addConstraint(c);
   if (debugMode) {
       std::cout << "[TestGenSymbolic] AssumeStmt -> constraint: " << c << std::endl;
   }
}


// Handle dedicated AssertStmt node
void TestGenSymbolicVisitor::visit(const AssertStmt &s) {
   if (!s.condition) return;
   std::string c = convertExprToSMT(*s.condition);
   env.addConstraint(c);
   if (debugMode) {
       std::cout << "[TestGenSymbolic] AssertStmt -> constraint: " << c << std::endl;
   }
}


// Handle InputStmt (AST-level input statement).
//Convert it to env input registration.
void TestGenSymbolicVisitor::visit(const InputStmt &s) {
   // Expect InputStmt contains a Var* named `var` (adjust if field name differs).
   if (!s.var) {
       if (debugMode) std::cout << "[TestGenSymbolic] InputStmt with null var\n";
       return;
   }
   env.addInputVariable(s.var->name);
   if (debugMode) {
       std::cout << "[TestGenSymbolic] InputStmt: " << s.var->name << std::endl;
   }
}
void TestGenSymbolicVisitor::visit(const Program& n) {
   if (debugMode) {
       std::cout << "[TestGenSymbolic] Processing program with "
                 << n.statements.size() << " statements" << std::endl;
   }

   // Process statements with isReady check
   for (const auto& stmt : n.statements) {
       if (!stmt) continue;
      
       if (isReadyStmt(*stmt)) {
           stmt->accept(*this);  // Process symbolically
            // Check if execution should stop (API not ready)
           if (stopExecution) {
               if (debugMode) {
                   std::cout << "[TestGenSymbolic] Stopping - API not ready" << std::endl;
               }
               break;
           }
       } else {
           if (debugMode) {
               std::cout << "[TestGenSymbolic] Stopping at not-ready statement"
                         << std::endl;
           }
           break;  // ← Stop at first not-ready
       }
   }
}


// Stub implementations for expression visitors (used by convertExprToSMT)
void TestGenSymbolicVisitor::visit(const Var& n) {
   // This gets called from convertExprToSMT (no-op)
}
void TestGenSymbolicVisitor::visit(const FuncCall& n) {
   // This gets called from convertExprToSMT  (no-op)
}
void TestGenSymbolicVisitor::visit(const Num& n) {
   // This gets called from convertExprToSMT (no-op)
}
void TestGenSymbolicVisitor::visit(const String& n) {
   // This gets called from convertExprToSMT (no-op)
}


std::string TestGenSymbolicVisitor::exprToSMT(const Expr& expr) {
   return convertExprToSMT(expr);
}


std::string TestGenSymbolicVisitor::convertExprToSMT(const Expr& expr) {
   // Handle variables
   if (auto* var = dynamic_cast<const Var*>(&expr)) {


       // Boolean literals
       if (var->name == "true" || var->name == "false") {
           return var->name;
       }


       // If this variable name is exactly the domain/value array symbol (Val_.. / Dom_..), emit it directly
       if (var->name.rfind("Val_", 0) == 0 || var->name.rfind("Dom_", 0) == 0) {
           return sanitizeSmtIdent(var->name);
       }


       // If variable name corresponds to an already-declared map (T, U, S, etc),
       // use its Value array symbol (Val_T) as the "array" expression when needed
       if (env.isMap(var->name)) {
           const auto &mi = env.getMapInfo(var->name);
           // Note: we return the map *value-array* identifier here (e.g., Val_T)
           return sanitizeSmtIdent(mi.valueArray);
       }


       // Otherwise it's a regular program var -> return its internal SMT symbol (v1, v2, ...)
       return env.getSymbolFor(var->name);
   }


   // Handle numbers
   if (auto* num = dynamic_cast<const Num*>(&expr)) {
       return std::to_string(num->value);
   }


   // Handle strings
   if (auto* str = dynamic_cast<const String*>(&expr)) {
       return "\"" + escapeStringLiteral(str->value) + "\"";
   }


   // Handle function calls (the main logic)
   if (auto* funcCall = dynamic_cast<const FuncCall*>(&expr)) {
       const std::string& name = funcCall->name;


      if (name == "dom" && funcCall->args.size() == 1) {
           const Expr* mapExpr = funcCall->args[0].get();

           
          
           // Case 1: dom(U) where U is a map variable
           if (auto* mapVar = dynamic_cast<const Var*>(mapExpr)) {
            std::string mapName = mapVar->name; 
               if (env.isMap(mapVar->name)) {
                   const auto& mi = env.getMapInfo(mapVar->name);
                   return sanitizeSmtIdent(mi.domainArray);
               }
              
               if (mapName.size() > 4 && mapName.substr(mapName.size() - 4) == "_old") {
            std::string baseName = mapName.substr(0, mapName.size() - 4);
            if (env.isMap(baseName)) {
                return sanitizeSmtIdent("Dom_" + mapName);
            }
        }

               // Case 2: dom(Val_U) where Val_U is already a value array
               if (mapVar->name.rfind("Val_", 0) == 0) {
                   std::string mapName = mapVar->name.substr(4);
                   if (env.isMap(mapName)) {
                       const auto& mi = env.getMapInfo(mapName);
                       return sanitizeSmtIdent(mi.domainArray);
                   }
               }
           }
          
           // Fallback
           throw std::runtime_error("dom() requires a map variable");
       }


       // Handle not_in() function
   if (name == "not_in" && funcCall->args.size() == 2) {
   const Expr* elem = funcCall->args[0].get();
   const Expr* set = funcCall->args[1].get();
  
   // not_in(x, Dom_U) → (not (select Dom_U x))
   std::string elemExpr = convertExprToSMT(*elem);
   std::string setExpr = convertExprToSMT(*set);
  
   return "(not (select " + setExpr + " " + elemExpr + "))";
   }
       // Special-case: map selection printed as "[]"
       // Handles:  ([] T key)    -> (select Val_T key)
       //           ([] Val_T key) -> (select Val_T key)
       //           ([] v3 key)    -> if v3 corresponds to map T → (select Val_T key)
       if (name == "[]" && funcCall->args.size() == 2) {
           const Expr* mexpr = funcCall->args[0].get();
           const Expr* kexpr = funcCall->args[1].get();


           // CASE 1: First arg is a Var
           if (auto *mv = dynamic_cast<const Var*>(mexpr)) {
            std::string mapName = mv->name;


               // A. Var directly names a declared map (T, U, S)
               if (env.isMap(mv->name)) {
                   const auto &mi = env.getMapInfo(mv->name);
                   return "(select " + sanitizeSmtIdent(mi.valueArray)
                          + " " + convertExprToSMT(*kexpr) + ")";
               }

               if (mapName.size() > 4 && mapName.substr(mapName.size() - 4) == "_old") {
            std::string baseName = mapName.substr(0, mapName.size() - 4);
            if (env.isMap(baseName)) {
                return "(select " + sanitizeSmtIdent("Val_" + mapName) + 
                       " " + convertExprToSMT(*kexpr) + ")";
            }
        }

               // B. Var is already a domain/value array
               if (mv->name.rfind("Val_", 0) == 0 || mv->name.rfind("Dom_", 0) == 0) {
                   return "(select " + sanitizeSmtIdent(mv->name)
                          + " " + convertExprToSMT(*kexpr) + ")";
               }


               // C. Var is a symbol that maps back to a declared map (v3 -> T)
               if (auto mapNameOpt = env.findMapForSymbol(mv->name)) {
                   const auto &mi = env.getMapInfo(*mapNameOpt);
                   return "(select " + sanitizeSmtIdent(mi.valueArray)
                          + " " + convertExprToSMT(*kexpr) + ")";
               }


               // D. If we reach here, the Var isn't a declared map nor known symbol — warn and fallback
               if (debugMode) std::cout << "[TestGenSymbolic] WARN: map-selection [] on unknown var " << mv->name << std::endl;
           }


           // CASE 2: fallback (complex expression)
           // We conservatively emit select on the converted expression (select <array-expr> <key>)
           return "(select " + convertExprToSMT(*mexpr) + " " + convertExprToSMT(*kexpr) + ")";
       }


       // Boolean operators
       if (name == "AND" && funcCall->args.size() >= 2) {
           std::ostringstream result;
           result << "(and";
           for (const auto& arg : funcCall->args) {
               result << " " << convertExprToSMT(*arg);
           }
           result << ")";
           return result.str();
       }


       if (name == "OR" && funcCall->args.size() >= 2) {
           std::ostringstream result;
           result << "(or";
           for (const auto& arg : funcCall->args) {
               result << " " << convertExprToSMT(*arg);
           }
           result << ")";
           return result.str();
       }


       // Must check BOTH: key in domain AND value matches
   if (name == "=" && funcCall->args.size() == 2) {
   // Check if LHS is map access: U[key]
      if (auto* lhs = dynamic_cast<const FuncCall*>(funcCall->args[0].get())) {
         if (lhs->name == "[]" && lhs->args.size() == 2) {
           if (auto* mapVar = dynamic_cast<const Var*>(lhs->args[0].get())) {
               if (env.isMap(mapVar->name)) {
                   // This is: U[key] = value
                   // Must encode: (and (key in domain) (U[key] = value))
                  
                   const auto& mi = env.getMapInfo(mapVar->name);
                   std::string key = convertExprToSMT(*lhs->args[1]);
                   std::string value = convertExprToSMT(*funcCall->args[1]);
                  
                   std::ostringstream result;
                   result << "(and ";
                  
                   // Part 1: Domain check - key must be in domain
                   result << "(select " << sanitizeSmtIdent(mi.domainArray) << " " << key << ")";
                  
                   result << " ";
                  
                   // Part 2: Value check - value must match
                   result << "(= (select " << sanitizeSmtIdent(mi.valueArray) << " " << key << ") " << value << ")";
                  
                   result << ")";
                  
                   if (debugMode) {
                       std::cout << "[convertExprToSMT] Map access equality: "
                                 << mapVar->name << "[key] = value" << std::endl;
                       std::cout << "  → " << result.str() << std::endl;
                   }
                  
                   return result.str();
               }
           }
       }
   }
  
   // Fallback: regular equality (not map access)
   return "(= " + convertExprToSMT(*funcCall->args[0]) + " " + convertExprToSMT(*funcCall->args[1]) + ")";
}


       // Equality
       // equals(S, Union(S_old, idExpr)) -> encode domain store + value array equality
       if (name == "equals" && funcCall->args.size() == 2) {
           if (auto* leftVar = dynamic_cast<const Var*>(funcCall->args[0].get())) {
               if (auto* unionCall = dynamic_cast<const FuncCall*>(funcCall->args[1].get())) {
                   if (unionCall->name == "Union" && unionCall->args.size() == 2) {
                       if (auto* baseMapVar = dynamic_cast<const Var*>(unionCall->args[0].get())) {
                           // Ensure both maps are declared (should be done at AST-time, but safe-check here)
                           if (!env.isMap(leftVar->name) || !env.isMap(baseMapVar->name)) {
                               if (debugMode) std::cout << "[WARN] equals(... Union(...)) on undeclared map(s)\n";
                               // fallback to conservative equality
                               return "(= " + convertExprToSMT(*funcCall->args[0]) + " " + convertExprToSMT(*funcCall->args[1]) + ")";
                           }
                           const auto& S    = env.getMapInfo(leftVar->name);
                           const auto& Sold = env.getMapInfo(baseMapVar->name);
                           std::string idExpr = convertExprToSMT(*unionCall->args[1]);


                           std::ostringstream r;
                           r << "(and (= " << S.domainArray << " (store " << Sold.domainArray << " " << idExpr << " true))"
                             << " (= " << S.valueArray  << " " << Sold.valueArray  << "))";
                           return r.str();
                       }
                   }
               }
           }
           // fallback: scalar equality
           return "(= " + convertExprToSMT(*funcCall->args[0]) + " " + convertExprToSMT(*funcCall->args[1]) + ")";
       }


       // Special-case: in(element, set) or in(set, element)
       if (name == "in" && funcCall->args.size() == 2) {
           const Expr* a0 = funcCall->args[0].get();
           const Expr* a1 = funcCall->args[1].get();


           // If second arg names a declared map, emit (select Dom_<map> <elem>)
           if (auto* mapVar = dynamic_cast<const Var*>(a1)) {
               if (env.isMap(mapVar->name)) {
                   std::string keyExpr = convertExprToSMT(*a0);
                   const auto &mi = env.getMapInfo(mapVar->name);
                   return "(select " + sanitizeSmtIdent(mi.domainArray) + " " + keyExpr + ")";
               }
           }
           // If first arg is declared map and second is element (swapped order)
           if (auto* mapVar = dynamic_cast<const Var*>(a0)) {
               if (env.isMap(mapVar->name)) {
                   std::string keyExpr = convertExprToSMT(*a1);
                   const auto &mi = env.getMapInfo(mapVar->name);
                   return "(select " + sanitizeSmtIdent(mi.domainArray) + " " + keyExpr + ")";
               }
           }


           // Fallback to uninterpreted 'in' predicate
           return "(in " + convertExprToSMT(*a0) + " " + convertExprToSMT(*a1) + ")";
       }


       // Map operations
       // Map check: inMapVerify(map, key)  OR inMapVerify(key, map)
       if (name == "inMapVerify" && funcCall->args.size() == 2) {
           // Case 1: (map, key)
           if (auto* mapVar = dynamic_cast<const Var*>(funcCall->args[0].get())) {
               if (env.isMap(mapVar->name)) {
                   const auto& mi = env.getMapInfo(mapVar->name);
                   std::string keyExpr = convertExprToSMT(*funcCall->args[1]);
                   return "(select " + sanitizeSmtIdent(mi.domainArray) + " " + keyExpr + ")";
               }
           }
           // Case 2: (key, map)
           if (auto* mapVar = dynamic_cast<const Var*>(funcCall->args[1].get())) {
               if (env.isMap(mapVar->name)) {
                   const auto& mi = env.getMapInfo(mapVar->name);
                   std::string keyExpr = convertExprToSMT(*funcCall->args[0]);
                   return "(select " + sanitizeSmtIdent(mi.domainArray) + " " + keyExpr + ")";
               }
           }
           if (debugMode) std::cout << "[TestGenSymbolic] WARN: inMapVerify on non-var map\n";
           // Fallback: safe tautology so SMT stays valid
           return "true";
       }


       // mapAccess handling: mapAccess(map, key) -> (select Val_<map> key)
       if (name == "mapAccess" && funcCall->args.size() == 2) {
           const Expr* mexpr = funcCall->args[0].get();
           const Expr* kexpr = funcCall->args[1].get();


           // Case 1: map is a Var referencing a declared map (T, U, S, etc.)
           if (auto* mv = dynamic_cast<const Var*>(mexpr)) {


               // A. If Var itself is a declared map
               if (env.isMap(mv->name)) {
                   const auto &mi = env.getMapInfo(mv->name);
                   return "(select " + sanitizeSmtIdent(mi.valueArray) +
                          " " + convertExprToSMT(*kexpr) + ")";
               }


               // B. If Var is a symbol corresponding to a map (v4 -> U)
               if (auto mapNameOpt = env.findMapForSymbol(mv->name)) {
                   const auto &mi = env.getMapInfo(*mapNameOpt);
                   return "(select " + sanitizeSmtIdent(mi.valueArray) +
                          " " + convertExprToSMT(*kexpr) + ")";
               }


               // C. If Var already *is* Val_T or Dom_U
               if (mv->name.rfind("Val_", 0) == 0 || mv->name.rfind("Dom_", 0) == 0) {
                   return "(select " + sanitizeSmtIdent(mv->name) +
                          " " + convertExprToSMT(*kexpr) + ")";
               }


               if (debugMode) std::cout << "[TestGenSymbolic] WARN: mapAccess on non-map var " << mv->name << std::endl;
           }


           // Case 2: fallback — evaluate map expr and attempt select on it
           return "(select " + convertExprToSMT(*mexpr) + " " + convertExprToSMT(*kexpr) + ")";
       }


       // Field access functions
       if (name == "getId" && funcCall->args.size() == 1) {
           // For now, treat as identity function ,may need to adjust this
           // based on the domain model
           return convertExprToSMT(*funcCall->args[0]);
       }


       if (name == "getRoles" && funcCall->args.size() == 1) {
           // Similar to getId, may need domain-specific handling
           return convertExprToSMT(*funcCall->args[0]);
       }


       // Set operations
       if (name == "inList" && funcCall->args.size() == 2) {
           // inList(element, set) -> check membership
           return "(select " + convertExprToSMT(*funcCall->args[1]) + " " +
                             convertExprToSMT(*funcCall->args[0]) + ")";
       }


       // Input/fresh
       if (name == "input" && funcCall->args.empty()) {
           return env.createFreshSymbol("input");
       }


       if (name == "fresh" && funcCall->args.empty()) {
           return env.createFreshSymbol("fresh");
       }


       // Dash operator (primed variables)
       if (name == "'" && funcCall->args.size() == 1) {
           // Handle primed variables like S'
           return convertExprToSMT(*funcCall->args[0]);  // ← JUST THIS ONE LINE!
       }


       // Set operations
       if (name == "\\" && funcCall->args.size() == 2) {
           // Set difference - may need specific encoding
           return "(set_minus " + convertExprToSMT(*funcCall->args[0]) + " " +
                                 convertExprToSMT(*funcCall->args[1]) + ")";
       }


       if (name == "Union") {
           // Should be handled inside equals(S, Union(...)).
           if (debugMode) std::cout << "[TestGenSymbolic] WARN: standalone Union encountered\n";
           // Conservative fallback: just return the base map expression unchanged
           // (keeps SMT valid; semantics come from the equals pattern above).
           return convertExprToSMT(*funcCall->args[0]);
       }
      // Restaurant / webapp-specific predicates: just print as SMT predicates
       if (name == "authenticated" ||
           name == "token_present" ||
           name == "cart_contains" ||
           name == "order_recorded" ||
           name == "result_is_restaurant_list" ||
           name == "result_is_menu" ||
           name == "result_is_cart" ||
           name == "review_added") {

           std::ostringstream out;
           out << "(" << sanitizeSmtIdent(name);
           for (const auto& arg : funcCall->args) {
               out << " " << convertExprToSMT(*arg);
           }
           out << ")";
           return out.str();
       }


       // Default: uninterpreted function
       {
           std::ostringstream result;
           result << "(" << sanitizeSmtIdent(name);
           for (const auto& arg : funcCall->args) {
               result << " " << convertExprToSMT(*arg);
           }
           result << ")";
           return result.str();
       }
   }


   throw std::runtime_error("Unsupported expression type in convertExprToSMT");
}
//isReady Implementation


bool TestGenSymbolicVisitor::isAPI(const std::string& fname) const {
   if (nativeHelpers.count(fname)) return false;
   return true;
}


bool TestGenSymbolicVisitor::isSymbolicExpr(const Expr& e) const {
   if (auto* var = dynamic_cast<const Var*>(&e)) {
       if (var->name == "true" || var->name == "false") {
           return false;
       }
       if (env.hasExpr(var->name)) {
           Expr* bound = env.getExprPtr(var->name);
           if (bound) {
               if (bound->expressionType == ExpressionType::NUM ||
                   bound->expressionType == ExpressionType::STRING) {
                   return false;
               }
           }
       }
       return true;
   }
  
   if (dynamic_cast<const Num*>(&e)) return false;
   if (dynamic_cast<const String*>(&e)) return false;
  
   if (auto* fc = dynamic_cast<const FuncCall*>(&e)) {
       for (const auto& arg : fc->args) {
           if (isSymbolicExpr(*arg)) return true;
       }
       return false;
   }
  
   if (auto* set = dynamic_cast<const Set*>(&e)) {
       for (const auto& elem : set->elements) {
           if (isSymbolicExpr(*elem)) return true;
       }
       return false;
   }
  
   if (auto* tuple = dynamic_cast<const Tuple*>(&e)) {
       for (const auto& elem : tuple->expr) {
           if (isSymbolicExpr(*elem)) return true;
       }
       return false;
   }
  
   return true;
}


bool TestGenSymbolicVisitor::isReadyExpr(const Expr& e) const {
   if (auto* var = dynamic_cast<const Var*>(&e)) {
       return !isSymbolicExpr(e);
   }
  
   if (dynamic_cast<const Num*>(&e)) return true;
   if (dynamic_cast<const String*>(&e)) return true;
  
   if (auto* fc = dynamic_cast<const FuncCall*>(&e)) {
       if (isAPI(fc->name)) {
           for (const auto& arg : fc->args) {
               if (!isReadyExpr(*arg)) return false;
           }
           return true;
       } else {
           return true;
       }
   }
  
   if (auto* set = dynamic_cast<const Set*>(&e)) {
       for (const auto& elem : set->elements) {
           if (!isReadyExpr(*elem)) return false;
       }
       return true;
   }
  
   if (auto* tuple = dynamic_cast<const Tuple*>(&e)) {
       for (const auto& elem : tuple->expr) {
           if (!isReadyExpr(*elem)) return false;
       }
       return true;
   }
  
   return true;
}


bool TestGenSymbolicVisitor::isReadyStmt(const Stmt& s) const {
   switch (s.statementType) {
       case StatementType::ASSIGN: {
           const Assign& a = static_cast<const Assign&>(s);
           if (!a.right) return true;
           return isReadyExpr(*a.right);
       }
       case StatementType::ASSUME:
           return true;
       case StatementType::ASSERT:
           return true;
       case StatementType::FUNCTIONCALL_STMT: {
           const FuncCallStmt& fcs = static_cast<const FuncCallStmt&>(s);
           if (!fcs.call) return true;
           const FuncCall& fc = *fcs.call;
           if (fc.name == "assume" || fc.name == "assert" || fc.name == "input") {
               return true;
           }
           if (isAPI(fc.name)) {
               for (const auto& arg : fc.args) {
                   if (!isReadyExpr(*arg)) return false;
               }
               return true;
           }
           return true;
       }
       case StatementType::INPUT:
           return true;
       default:
           return true;
   }
}


// HELPER METHODS FOR API EXECUTION


bool TestGenSymbolicVisitor::isConcreteExpr(const Expr& e) const {
    // Literals are always concrete
    if (dynamic_cast<const String*>(&e)) return true;
    if (dynamic_cast<const Num*>(&e)) return true;

    // Variables
    if (auto* var = dynamic_cast<const Var*>(&e)) {
        // Boolean constants
        if (var->name == "true" || var->name == "false") return true;

        // If env has ANY binding → treat as concrete
        if (env.hasExpr(var->name)) {
            const Expr* bound = env.getExprPtr(var->name);
            if (bound) return true;
        }

        return false;
    }

    // FuncCall recursive check
    if (auto* fc = dynamic_cast<const FuncCall*>(&e)) {
        for (const auto& arg : fc->args) {
            if (!isConcreteExpr(*arg)) return false;
        }
        return true;
    }

    return false;
}


// Helper to clone expressions
std::unique_ptr<Expr> TestGenSymbolicVisitor::cloneExpr(const Expr* expr) const {
   if (!expr) {
       throw std::runtime_error("Cannot clone null expression");
   }
  
   if (auto* str = dynamic_cast<const String*>(expr)) {
       return std::make_unique<String>(str->value);
   }
   if (auto* num = dynamic_cast<const Num*>(expr)) {
       return std::make_unique<Num>(num->value);
   }
   if (auto* var = dynamic_cast<const Var*>(expr)) {
       return std::make_unique<Var>(var->name);
   }
  
   throw std::runtime_error("Unsupported Expr type for cloning");
}


//  Generic, works with any webapp!
void TestGenSymbolicVisitor::executeAPI(
   const std::string& apiName,
   const std::vector<std::unique_ptr<Expr>>& argExprs,
   const std::vector<std::unique_ptr<Expr>>& originalArgs
) {
   if (!functionFactory) {
       if (debugMode) {
           std::cout << "[API] No function factory configured - skipping execution"
                     << std::endl;
       }
       return;
   }
  
   // Check if factory handles this API
   if (!functionFactory->hasFunction(apiName)) {
       if (debugMode) {
           std::cout << "[API] Function not found: " << apiName << std::endl;
       }
       return;
   }
  
   // Get function object from factory
   // This is: f ← FunctionFactory.getFunction("signup", [a1, a2])
   auto func = functionFactory->getFunction(apiName, argExprs);
  
   if (!func) {
       if (debugMode) {
           std::cout << "[API] Failed to create function: " << apiName << std::endl;
       }
       return;
   }
  
   // Execute the function
   // This is: r := f.execute()
   try {
    if (debugMode) {
        std::cout << "[API] Executing: " << apiName << std::endl;
    }
   
    auto result = func->execute();
   
    if (debugMode) {
        std::cout << "[API] " << apiName << " executed successfully" << std::endl;
       
        // Show result if it's a number (status code)
        if (auto* num = dynamic_cast<const Num*>(result.get())) {
            std::cout << "[API] Response code: " << num->value << std::endl;
        }
    }
    
    //  Update symbolic state after successful execution
    if (auto* statusCode = dynamic_cast<const Num*>(result.get())) {
        if (statusCode->value >= 200 && statusCode->value < 300) {
            updateStateAfterAPI(apiName, originalArgs, statusCode->value);
        }
    }
    
} catch (const std::exception& e) {
    std::cerr << "[API] ✗ " << apiName << " failed: " << e.what() << std::endl;
}

}

// UPDATED updateStateAfterAPI - Actually update symbolic state!
// Location: SymbolicEngine/TestGenSymbolicVisitor.cpp

void TestGenSymbolicVisitor::updateStateAfterAPI(
    const std::string& apiName,
    const std::vector<std::unique_ptr<Expr>>& argExprs,
    int statusCode
) {
    if (statusCode < 200 || statusCode >= 300) {
        return; // Only update on success
    }
    
    // After successful signup: Add user to U
    if ((apiName == "signup" || apiName == "registerUser") && argExprs.size() >= 3) {
        // Get email and password arguments
        std::string emailSym = convertExprToSMT(*argExprs[1]);
        std::string passwordSym = convertExprToSMT(*argExprs[2]);
        
        // Update Dom_U: (= Dom_U (store Dom_U_old email true))
        //std::string domUpdate = "(= Dom_U (store Dom_U_old " + emailSym + " true))";
        //env.addConstraint(domUpdate);
        
        // Update Val_U: (= Val_U (store Val_U_old email password))
        //std::string valUpdate = "(= Val_U (store Val_U_old " + emailSym + " " + passwordSym + "))";
        //env.addConstraint(valUpdate);
        // Update Dom_U: add email to domain
        std::string domUpdate = "(= Dom_U (store Dom_U " + emailSym + " true))";
        env.addConstraint(domUpdate);
        
        // Update Val_U: set email -> password mapping
        std::string valUpdate = "(= Val_U (store Val_U " + emailSym + " " + passwordSym + "))";
        env.addConstraint(valUpdate);
        
        
        if (debugMode) {
            std::cout << "[StateUpdate] Updated U after signup:" << std::endl;
            std::cout << "  " << domUpdate << std::endl;
            std::cout << "  " << valUpdate << std::endl;
        }
    }
    
    // After successful login: Add token to T
    else if (apiName == "login" && argExprs.size() >= 2) {
        // Get email argument
        std::string emailSym = convertExprToSMT(*argExprs[0]);
        
        // Create fresh symbol for token (returned by API)
        std::string tokenSym = env.createFreshSymbol("token");
        
        // Update Dom_T: (= Dom_T (store Dom_T_old token true))
        std::string domUpdate = "(= Dom_T (store Dom_T_old " + tokenSym + " true))";
        env.addConstraint(domUpdate);
        
        // Update Val_T: (= Val_T (store Val_T_old token email))
        std::string valUpdate = "(= Val_T (store Val_T_old " + tokenSym + " " + emailSym + "))";
        env.addConstraint(valUpdate);
        
        if (debugMode) {
            std::cout << "[StateUpdate] Updated T after login:" << std::endl;
            std::cout << "  " << domUpdate << std::endl;
            std::cout << "  " << valUpdate << std::endl;
        }
    }
}


// EXPLANATION:
// The problem: Assertions check conditions but don't UPDATE state
//
// Before :
//   ASSERT: U[email] = password  ← Just checks, doesn't modify U
//   Z3 sees: "email must be in U" but U is still empty!
//
// After:
//   UPDATE: Dom_U = store(Dom_U_old, email, true)
//   UPDATE: Val_U = store(Val_U_old, email, password)
//   Now Z3 sees: U actually contains the new entry!
//
// The SMT store operator creates a new array with one element updated:
//   store(array, key, value) = array with array[key] = value
//
// This way, after signup executes:
//   Dom_U = store(empty, "A", true)  → Dom_U["A"] = true
//   Val_U = store(empty, "A", "C")   → Val_U["A"] = "C"
//
// Now login's precondition U[email1] = password1 can be satisfied:
//   Z3 sets: email1 = "A", password1 = "C" → SAT!
