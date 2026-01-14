#include "../env.hh" // will change this to normal env.hh later
#include "./see.hh"
#include "../clonevisitor.hh"
#include "functionfactory.hh"
#include <iostream>
#include <set>
using namespace std;

//  Helper to check if an expression is concrete (i.e., Num, String, Set, Map)
static bool isConcrete(Expr* expr) {
    if (!expr) return false;
    
    ExprType type = expr->exprType;
    return (type == ExprType::NUM || 
            type == ExprType::STRING || 
            type == ExprType::SET || 
            type == ExprType::MAP);
}

// Helper to check if a Num represents a boolean value
static bool isNumBool(Expr* expr) {
    if (expr && expr->exprType == ExprType::NUM) {
        Num* n = dynamic_cast<Num*>(expr);
        return (n->value == 0 || n->value == 1);
    }
    return false;
}

// Helper function to print expressions (raw pointer version)
static string exprToString(Expr* expr) {
    if (!expr) return "null";
    
    if (expr->exprType == ExprType::SYMVAR) {
        SymVar* sv = dynamic_cast<SymVar*>(expr);
        return "X" + ::to_string(sv->getNum());
    }
    else if (expr->exprType == ExprType::NUM) {
        Num* num = dynamic_cast<Num*>(expr);
        return ::to_string(num->value);
    }
    else if (expr->exprType == ExprType::VAR) {
        Var* var = dynamic_cast<Var*>(expr);
        return var->name;
    }
    else if (expr->exprType == ExprType::FUNCCALL) {
        FuncCall* fc = dynamic_cast<FuncCall*>(expr);
        string result = fc->name + "(";
        for (size_t i = 0; i < fc->args.size(); i++) {
            if (i > 0) result += ", ";
            result += exprToString(fc->args[i].get());
        }
        result += ")";
        return result;
    }
    else if (expr->exprType == ExprType::STRING) {
        String* str = dynamic_cast<String*>(expr);
        return "\"" + str->value + "\"";
    }
    else if (expr->exprType == ExprType::SET) {
        Set* set = dynamic_cast<Set*>(expr);
        string result = "{";
        for (size_t i = 0; i < set->elements.size(); i++) {
            if (i > 0) result += ", ";
            result += exprToString(set->elements[i].get());
        }
        result += "}";
        return result;
    }
    else if (expr->exprType == ExprType::MAP) {
        Map* map = dynamic_cast<Map*>(expr);
        string result = "{";
        for (size_t i = 0; i < map->value.size(); i++) {
            if (i > 0) result += ", ";
            result += exprToString(map->value[i].first.get());
            result += " -> ";
            result += exprToString(map->value[i].second.get());
        }
        result += "}";
        return result;
    }
    else if (expr->exprType == ExprType::TUPLE) {
        Tuple* tuple = dynamic_cast<Tuple*>(expr);
        string result = "(";
        for (size_t i = 0; i < tuple->exprs.size(); i++) {
            if (i > 0) result += ", ";
            result += exprToString(tuple->exprs[i].get());
        }
        result += ")";
        return result;
    }
    // NEW: Handle Boolean nodes
    else if (expr->exprType == ExprType::BOOL_CONST) {
        BoolConst* bc = dynamic_cast<BoolConst*>(expr);
        return bc->value ? "true" : "false";
    }
    else if (expr->exprType == ExprType::BINARY_OP) {
        BinaryOpExpr* binop = dynamic_cast<BinaryOpExpr*>(expr);
        string opStr;
        switch (binop->op) {
            case BinOp::EQ: opStr = "="; break;
            case BinOp::NEQ: opStr = "!="; break;
            case BinOp::LT: opStr = "<"; break;
            case BinOp::LE: opStr = "<="; break;
            case BinOp::GT: opStr = ">"; break;
            case BinOp::GE: opStr = ">="; break;
            case BinOp::AND: opStr = "And"; break;
            case BinOp::OR: opStr = "Or"; break;
            case BinOp::IMPLIES: opStr = "Implies"; break;
            case BinOp::IN: opStr = "In"; break;
            case BinOp::NOT_IN: opStr = "NotIn"; break;
        }
        return "(" + opStr + " " + exprToString(binop->left.get()) + " " + exprToString(binop->right.get()) + ")";
    }
    else if (expr->exprType == ExprType::UNARY_OP) {
        UnaryOpExpr* unop = dynamic_cast<UnaryOpExpr*>(expr);
        string opStr;
        switch (unop->op) {
            case UnOp::NOT: opStr = "Not"; break;
        }
        return "(" + opStr + " " + exprToString(unop->operand.get()) + ")";
    }
    
    
    return "Unknown";
}

// Helper function to print expressions (unique_ptr version)
static string exprToString(const unique_ptr<Expr>& expr) {
    return exprToString(expr.get());
}

unique_ptr<Expr> SEE::computePathConstraint(vector<Expr*> C) {
    if (C.empty()) {
        // No constraints, return true (represented as 1 == 1)
        vector<unique_ptr<Expr>> args;
        args.push_back(make_unique<Num>(1));
        args.push_back(make_unique<Num>(1));
        return make_unique<FuncCall>("Eq", std::move(args));
    }
    
    CloneVisitor cloner;
    if (C.size() == 1) {
        // Single constraint, return it
        return cloner.cloneExpr(C[0]);
    }
    
    // Multiple constraints, conjoin them with AND
    // For now, we'll create a nested structure: C1 AND (C2 AND (C3 AND ...))
    unique_ptr<Expr> result = cloner.cloneExpr(C.back());
    
    for (int i = C.size() - 2; i >= 0; i--) {
        vector<unique_ptr<Expr>> args;
        args.push_back(cloner.cloneExpr(C[i]));
        args.push_back(std::move(result));
        result = make_unique<FuncCall>("And", std::move(args));
    }
    
    return result;
}

unique_ptr<Expr> SEE::computePathConstraint() {
    return computePathConstraint(pathConstraint);
}

bool SEE::isReady(Stmt& s, SymbolTable& st) {
    if(s.statementType == StmtType::ASSIGN) {
        Assign& assign = dynamic_cast<Assign&>(s);
        
        // Check if this is an API call assignment (e.g., r1 := f(x1))
        if(assign.right->exprType == ExprType::FUNCCALL) {
            FuncCall& fc = dynamic_cast<FuncCall&>(*assign.right);
            
            if(isAPI(fc)) {
                // This is an API call - check if arguments are ready
                // If any argument is symbolic, we need to interrupt and solve first
                for(const auto& arg : fc.args) {
                    if(isSymbolic(*arg, st)) {
                        cout << "[SEE] API call '" << fc.name << "' with symbolic arguments - interruption point" << endl;
                        return false; // Not ready - need to solve constraints first
                    }
                }
                
                // All arguments are concrete, API call is ready for execution
                cout << "[SEE] API call '" << fc.name << "' ready for actual execution" << endl;
                return true;
            } else{
                // Built-in function call, always ready
                return true;

            }
        }
        
        // Not an API call, check if right-hand side is ready
        return isReady(*assign.right, st);
    }
    else if(s.statementType == StmtType::ASSUME) {
        Assume& assume = dynamic_cast<Assume&>(s);
        return isReady(*assume.expr, st);
    }
    else if(s.statementType == StmtType::DECL) {
        // Declaration statements are always ready
        return true;
    }
    else {
        return false;
    }
}

bool SEE::isReady(Expr& e, SymbolTable& st) {
    if(e.exprType == ExprType::FUNCCALL) {
        FuncCall& fc = dynamic_cast<FuncCall&>(e);
        
        // Special case: input() with no arguments IS ready for symbolic execution
        // It will create a new symbolic variable
        if(fc.name == "input" && fc.args.size() == 0) {
            return true;
        }
        
        // Check if this is an API call
        if(isAPI(fc)) {
            // API calls need all arguments to be concrete (not symbolic)
            // Check if any argument is symbolic
            for(unsigned int i = 0; i < fc.args.size(); i++) {
                if(isSymbolic(*fc.args[i], st)) {
                    return false; // Not ready - has symbolic arguments
                }
            }
            return true; // All arguments are concrete
        }
        
        // Built-in functions (Add, Gt, etc.) are always ready
        // They can work with symbolic arguments
        return true;
    }
    if(e.exprType == ExprType::MAP) {
        Map& map = dynamic_cast<Map&>(e);
        for(unsigned int i = 0; i < map.value.size(); i++) {
            if(isReady(*map.value[i].second, st) == false) {
                return false;
            }
        }
        return true;
    }
    if(e.exprType == ExprType::NUM) {
        return true;
    }
    if(e.exprType == ExprType::SET) {
        Set& set = dynamic_cast<Set&>(e);
        for(unsigned int i = 0; i < set.elements.size(); i++) {
            if(isReady(*set.elements[i], st) == false) {
                return false;
            }
        }
        return true;
    }
    if(e.exprType == ExprType::STRING) {
        return true;
    }
    if(e.exprType == ExprType::SYMVAR) {
        // Symbolic variables are ready (they're the result of evaluation)
        return false;
    }

    //  Boolean expression types
    if(e.exprType == ExprType::BOOL_CONST) {
        return true; // BoolConst is always ready
    }
    if(e.exprType == ExprType::BINARY_OP) {
        BinaryOpExpr& binop = dynamic_cast<BinaryOpExpr&>(e);
        return isReady(*binop.left, st) && isReady(*binop.right, st);
    }
    if(e.exprType == ExprType::UNARY_OP) {
        UnaryOpExpr& unop = dynamic_cast<UnaryOpExpr&>(e);
        return isReady(*unop.operand, st);
    }
    if(e.exprType == ExprType::TUPLE) {
        Tuple& tuple = dynamic_cast<Tuple&>(e);
        for(unsigned int i = 0; i < tuple.exprs.size(); i++) {
            if(isReady(*tuple.exprs[i], st) == false) {
                return false;
            }
        }
        return true;
    }
    if(e.exprType == ExprType::VAR) {
    // Variables are ready if they're bound in sigma AND their value is concrete
    Var& var = dynamic_cast<Var&>(e);

    // First, try direct lookup
    if(sigma.hasValue(var.name)) {
        Expr* val = sigma.getValue(var.name);
        if(isSymbolic(*val, st)) {
            return false;
        }
        return true;
    }
    
    // NEW: Check if this is a base name with a mapped suffixed name
    auto it = baseNameToSuffixed.find(var.name);
    if (it != baseNameToSuffixed.end()) {
        string suffixedName = it->second;
        if (sigma.hasValue(suffixedName)) {
            Expr* val = sigma.getValue(suffixedName);
            if(isSymbolic(*val, st)) {
                return false;
            }
            return true;
        }
    }
    
    // Variable not found anywhere - not ready
    return false;
}
    else {
        return false;
    }
}

bool SEE::isAPI(const FuncCall& fc) {
    // Built-in functions that are NOT API calls
    static const set<string> builtInFunctions = {
        // Arithmetic
        "Add", "Sub", "Mul", "Div",
        // Comparison
        "Eq", "Lt", "Gt", "Le", "Ge", "Neq",
        "=", "==", "!=", "<>", "<", ">", "<=", ">=",
        // Logical
        "And", "Or", "Not", "Implies",
        "and", "or", "not", "&&", "||", "!",
        // Input
        "input",
        // Set operations
        "in", "not_in", "member", "not_member", "contains", "not_contains",
        "union", "intersection", "intersect", "difference", "diff", "minus",
        "subset", "is_subset", "add_to_set", "remove_from_set", "is_empty_set",
        // Map operations
        "get", "put", "lookup", "select", "store", "update",
        "contains_key", "has_key",
        // List/Sequence operations
        "concat", "append_list", "length", "at", "nth",
        "prefix", "suffix", "contains_seq",
        // Prime notation (for postconditions)
        "'"
    };
    
    return builtInFunctions.find(fc.name) == builtInFunctions.end();
}

bool SEE::isSymbolic(Expr& e, SymbolTable& st) {
    if(e.exprType == ExprType::SYMVAR) {
        return true;
    }
    // Boolean expression types
    else if(e.exprType == ExprType::BOOL_CONST) {
        return false; // BoolConst is concrete
    }
    else if(e.exprType == ExprType::BINARY_OP) {
        BinaryOpExpr& binop = dynamic_cast<BinaryOpExpr&>(e);
        return isSymbolic(*binop.left, st) || isSymbolic(*binop.right, st);
    }
    else if(e.exprType == ExprType::UNARY_OP) {
        UnaryOpExpr& unop = dynamic_cast<UnaryOpExpr&>(e);
        return isSymbolic(*unop.operand, st);
    }
    else if(e.exprType == ExprType::FUNCCALL) {
        FuncCall& fc = dynamic_cast<FuncCall&>(e);
        for(unsigned int i = 0; i < fc.args.size(); i++) {
            if(isSymbolic(*fc.args[i], st) == true) {
                return true;
            }
        }
        return false;
    }
    else if(e.exprType == ExprType::MAP) {
        Map& map = dynamic_cast<Map&>(e);
        for(unsigned int i = 0; i < map.value.size(); i++) {
            if(isSymbolic(*map.value[i].second, st) == true) {
                return true;
            }
        }
        return false;
    }
    else if(e.exprType == ExprType::NUM) {
        return false;
    }
    else if(e.exprType == ExprType::SET) {
        Set& set = dynamic_cast<Set&>(e);
        for(unsigned int i = 0; i < set.elements.size(); i++) {
            if(isSymbolic(*set.elements[i], st) == true) {
                return true;
            }
        }
        return false;
    }
    else if(e.exprType == ExprType::STRING) {
        return false;
    }
    else if(e.exprType == ExprType::TUPLE) {
        Tuple& tuple = dynamic_cast<Tuple&>(e);
        for(unsigned int i = 0; i < tuple.exprs.size(); i++) {
            if(isSymbolic(*tuple.exprs[i], st) == true) {
                return true;
            }
        }
        return false;
    }
    else if(e.exprType == ExprType::VAR) {
    Var& var = dynamic_cast<Var&>(e);
    
    // First, try direct lookup
    if(sigma.hasValue(var.name)) {
        Expr* val = sigma.getValue(var.name);
        return isSymbolic(*val, st);
    }
    
    // NEW: Check if this is a base name with a mapped suffixed name
    auto it = baseNameToSuffixed.find(var.name);
    if (it != baseNameToSuffixed.end()) {
        string suffixedName = it->second;
        if (sigma.hasValue(suffixedName)) {
            Expr* val = sigma.getValue(suffixedName);
            return isSymbolic(*val, st);
        }
    }
    
    return false;
}
    else {
        return false;
    }
}

// Symbolic Execution function following the algorithm:
// function symex([s1, s2, ..., sn], σ)
//   C ← []
//   for i = 1 to n do
//     if isReady(si) then
//       symexInstr(si, σ, C)
//     else
//       break
//   pc ← computePathConstraint(C)
//   return solve(pc)
void SEE::execute(Program &pg, SymbolTable& st) {
    // C is represented by pathConstraint (already a member variable)
    // σ is represented by sigma (already a member variable)
    
    // Clear previous state
    pathConstraint.clear();
    baseNameToSuffixed.clear(); // Clear base name mapping
    
    // Iterate through statements
    for (size_t i = 0; i < pg.statements.size(); i++) {
        const auto& stmt = pg.statements[i];
        
        // Check if statement is ready for execution
        if (isReady(*stmt, st)) {
            // Execute the statement (symexInstr)
            executeStmt(*stmt, st);
        } else {
            // Statement not ready (e.g., contains input() that needs concrete value)
            cout << "[SEE] Statement " << i << " not ready, interrupting execution" << endl;
            break;
        }
    }
    
    // Compute path constraint from collected constraints
    auto pc = computePathConstraint();
    cout << "\n[SEE] Path Constraint: " << exprToString(pc) << endl;
    
    // Note: solve(pc) is called externally by the caller (Tester class)
    return;
}

void SEE::executeStmt(Stmt& stmt, SymbolTable& st) {
    // the various if conditions for different statement types

    if(stmt.statementType == StmtType::ASSIGN) {
        Assign& assign = dynamic_cast<Assign&>(stmt);
        
        // Get the variable name from the left-hand side
        string varName;
        if (assign.left->exprType == ExprType::VAR) {
            Var* leftVar = dynamic_cast<Var*>(assign.left.get());
            varName = leftVar->name;
        } else if (assign.left->exprType == ExprType::TUPLE) {
            // Handle tuple assignment - use first element name or placeholder
            varName = "_tuple_result";
        } else {
            varName = "_unknown";
        }
        
        cout << "\n[ASSIGN] Evaluating: " << varName << " := " << exprToString(assign.right.get()) << endl;

        // Track base name mapping for ALL suffixed variable assignments
          string baseName = extractBaseName(varName);
          if (baseName != varName) {
          baseNameToSuffixed[baseName] = varName;
         cout << "[SEE] Mapping base name '" << baseName << "' -> '" << varName << "'" << endl;
         }
        
        // Check if this is an API call assignment (e.g., r1 := f(x1))
        if(assign.right->exprType == ExprType::FUNCCALL) {
            FuncCall& fc = dynamic_cast<FuncCall&>(*assign.right);
            
            if(isAPI(fc)) {
                // This is an API call - execute it
                cout << "[API_CALL] Executing API function: " << fc.name << endl;
                
                // Evaluate all arguments to get concrete values
                vector<Expr*> concreteArgs;
                for(const auto& arg : fc.args) {
                    Expr* evaluatedArg = evaluateExpr(*arg, st);
                    concreteArgs.push_back(evaluatedArg);
                    cout << "  [API_ARG] " << exprToString(evaluatedArg) << endl;
                }
                
                // Execute the actual API function if factory is available
                if(functionFactory != nullptr) {
                    try {
                        // Get the function implementation from the factory
                        cout << "  [API_CALL] Getting function from factory..." << endl;
                        unique_ptr<Function> function = functionFactory->getFunction(fc.name, concreteArgs);
                        
                        // Execute the function with concrete arguments
                        cout << "  [API_CALL] Executing function..." << endl;
                        unique_ptr<Expr> result = function->execute();
                        cout << "  [API_CALL] Function returned: " << exprToString(result) << endl;
                        
                        // Store the return value in sigma
                        cout << "  [API_CALL] Storing result in variable: " << varName << endl;
                        sigma.setValue(varName, result.release());
                        
                        cout << "[ASSIGN] Result: " << varName << " := " << exprToString(sigma.getValue(varName)) << endl;
                        return;
                    } catch(const char* error) {
                        cout << "  [API_CALL] Error: " << error << endl;
                        throw runtime_error(string("Function execution failed: ") + error);
                    } catch(const exception& e) {
                        cout << "  [API_CALL] Error: " << e.what() << endl;
                        throw;
                    }
                } else {
                    // No function factory available - use placeholder behavior
                    cout << "  [API_CALL] Warning: No FunctionFactory set, using placeholder" << endl;
                    cout << "  [API_CALL] Storing placeholder value in: " << varName << endl;
                    throw runtime_error("FunctionFactory not set in SEE");
                    cout << "[ASSIGN] Result: " << varName << " := 1 (placeholder)" << endl;
                    return;
                }
            } else {
                // Built-in function call (input, Add, etc.) - evaluate symbolically
                Expr* rhsExpr = evaluateExpr(*assign.right, st);
                
                cout << "[ASSIGN] Result: " << varName << " := " << exprToString(rhsExpr) << endl;

                // Store the mapping in sigma (value environment)
                sigma.setValue(varName, rhsExpr);
            }
        } else {
            // Not a function call - evaluate normally
            Expr* rhsExpr = evaluateExpr(*assign.right, st);
            
            cout << "[ASSIGN] Result: " << varName << " := " << exprToString(rhsExpr) << endl;

            // Store the mapping in sigma (value environment)
            sigma.setValue(varName, rhsExpr);
        }
    }     
    else if(stmt.statementType == StmtType::ASSUME) {
        Assume& assume = dynamic_cast<Assume&>(stmt);
        
        cout << "\n[ASSUME] Evaluating: " << exprToString(assume.expr.get()) << endl;
        
        // Add the assumption expression to the path constraint
        Expr* constraint = evaluateExpr(*assume.expr, st);
        
        cout << "[ASSUME] Adding constraint: " << exprToString(constraint) << endl;
        
        pathConstraint.push_back(constraint);
    } else if(stmt.statementType == StmtType::DECL) {
        // taking this as the declaration of a symbolic variable or the input statement
        // we need to get the last symbolic variable and add it to sigma with a new symbolic expression
        Decl& decl = dynamic_cast<Decl&>(stmt);
        string varName = decl.name;
        // we need to get the latest symbolic variable
        
        cout << "\n[DECL] Declaring symbolic variable: " << varName << endl;
        
        SymVar* symVarExpr = SymVar::getNewSymVar().release();
        
        cout << "[DECL] Created: " << varName << " := " << exprToString(symVarExpr) << endl;
        
        sigma.setValue(varName, symVarExpr);
    }

}

Expr* SEE::evaluateExpr(Expr& expr, SymbolTable& st) {
    // Evaluate expressions based on their type
    CloneVisitor cloner;
    
    if(expr.exprType == ExprType::FUNCCALL) {
        FuncCall& fc = dynamic_cast<FuncCall&>(expr);
        
        cout << "  [EVAL] FuncCall: " << fc.name << " with " << fc.args.size() << " args" << endl;
        
        // Special case: "input" function with no arguments returns a new symbolic variable
        if(fc.name == "input" && fc.args.size() == 0) {
            SymVar* symVar = SymVar::getNewSymVar().release();
            cout << "    [EVAL] input() returns new symbolic variable: " << exprToString(symVar) << endl;
            return symVar;
        }

        // ALL MAP OPERATIONS HERE 
        // =========================================================
        
        // MAP ACCESS: []
        if (fc.name == "[]" && fc.args.size() == 2) {
            cout << "    [EVAL] Map access: []" << endl;
            
            // Evaluate map expression
            Expr* mapExpr = evaluateExpr(*fc.args[0], st);
            cout << "    [EVAL] Map expr evaluated: " << exprToString(mapExpr) << endl;
            
            // Evaluate key expression
            Expr* keyExpr = evaluateExpr(*fc.args[1], st);
            cout << "    [EVAL] Key expr evaluated: " << exprToString(keyExpr) << endl;
            
            // If map is a Map literal, do lookup
            if (mapExpr && mapExpr->exprType == ExprType::MAP) {
                Map* map = dynamic_cast<Map*>(mapExpr);
                
                // Search for key in map
                for (const auto& kv : map->value) {
                    Expr* mapKey = kv.first.get();
                    
                    // Compare keys
                    bool keysMatch = false;
                    
                    if (keyExpr->exprType == ExprType::VAR && mapKey->exprType == ExprType::VAR) {
                        Var* k1 = dynamic_cast<Var*>(keyExpr);
                        Var* k2 = dynamic_cast<Var*>(mapKey);
                        keysMatch = (k1->name == k2->name);
                    }
                    else if (keyExpr->exprType == ExprType::STRING && mapKey->exprType == ExprType::VAR) {
                        String* k1 = dynamic_cast<String*>(keyExpr);
                        Var* k2 = dynamic_cast<Var*>(mapKey);
                        keysMatch = (k1->value == k2->name);
                    }
                    else if (keyExpr->exprType == ExprType::VAR && mapKey->exprType == ExprType::STRING) {
                        Var* k1 = dynamic_cast<Var*>(keyExpr);
                        String* k2 = dynamic_cast<String*>(mapKey);
                        keysMatch = (k1->name == k2->value);
                    }
                    else if (keyExpr->exprType == ExprType::STRING && mapKey->exprType == ExprType::STRING) {
                        String* k1 = dynamic_cast<String*>(keyExpr);
                        String* k2 = dynamic_cast<String*>(mapKey);
                        keysMatch = (k1->value == k2->value);
                    }
                    else if (keyExpr->exprType == ExprType::SYMVAR) {
                        // Symbolic key - return symbolic FuncCall
                        cout << "    [EVAL] Symbolic key, returning symbolic []" << endl;
                        vector<unique_ptr<Expr>> args;
                        args.push_back(cloner.cloneExpr(mapExpr));
                        args.push_back(cloner.cloneExpr(keyExpr));
                        return new FuncCall("[]", std::move(args));
                    }

                
                    if (keysMatch) {
                        // Found key, return value
                        cout << "    [EVAL] Key found in map, returning value" << endl;
                        return cloner.cloneExpr(kv.second.get()).release();
                    }
                }
                
                // Key not found - return symbolic result
                cout << "    [EVAL] Key not found in map, returning symbolic []" << endl;
                vector<unique_ptr<Expr>> args;
                args.push_back(cloner.cloneExpr(mapExpr));
                args.push_back(cloner.cloneExpr(keyExpr));
                return new FuncCall("[]", std::move(args));
            }
            
            // Map is symbolic - return symbolic FuncCall
            cout << "    [EVAL] Map is symbolic, returning symbolic []" << endl;
            vector<unique_ptr<Expr>> args;
            args.push_back(cloner.cloneExpr(mapExpr));
            args.push_back(cloner.cloneExpr(keyExpr));
            return new FuncCall("[]", std::move(args));
        }
        
        // MAP DOMAIN: dom
        if (fc.name == "dom" && fc.args.size() == 1) {
            cout << "    [EVAL] Map domain: dom" << endl;
            
            // Evaluate map expression
            Expr* mapExpr = evaluateExpr(*fc.args[0], st);
            cout << "    [EVAL] Map expr evaluated: " << exprToString(mapExpr) << endl;
            
            if (mapExpr && mapExpr->exprType == ExprType::MAP) {
                Map* map = dynamic_cast<Map*>(mapExpr);
                
                // Extract all keys into a Set
                vector<unique_ptr<Expr>> keys;
                for (const auto& kv : map->value) {
                    keys.push_back(cloner.cloneExpr(kv.first.get()));
                }
                
                cout << "    [EVAL] Domain has " << keys.size() << " keys" << endl;
                return new Set(std::move(keys));
            }
            
            // Map is symbolic - return symbolic dom call
            cout << "    [EVAL] Map is symbolic, returning symbolic dom()" << endl;
            vector<unique_ptr<Expr>> args;
            args.push_back(cloner.cloneExpr(mapExpr));
            return new FuncCall("dom", std::move(args));
        }
        
        // MAP UPDATE: put
        if (fc.name == "put" && fc.args.size() == 3) {
            cout << "    [EVAL] Map update: put" << endl;
            
            // Evaluate arguments
            Expr* mapExpr = evaluateExpr(*fc.args[0], st);
            Expr* keyExpr = evaluateExpr(*fc.args[1], st);
            Expr* valueExpr = evaluateExpr(*fc.args[2], st);
            
            cout << "    [EVAL] Map: " << exprToString(mapExpr) << endl;
            cout << "    [EVAL] Key: " << exprToString(keyExpr) << endl;
            cout << "    [EVAL] Value: " << exprToString(valueExpr) << endl;
            
            // If all are concrete, perform functional update
            if (mapExpr && mapExpr->exprType == ExprType::MAP && keyExpr && valueExpr) {
                Map* originalMap = dynamic_cast<Map*>(mapExpr);
                
                // Clone the map
                unique_ptr<Expr> clonedMapExpr = cloner.cloneExpr(originalMap);
                Map* newMap = dynamic_cast<Map*>(clonedMapExpr.get());
                
                // Search for existing key and update, or add new key
                bool keyFound = false;
                
                // Cast away const to modify (needed for Map's const vector)
                auto& mutableValue = const_cast<vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>&>(newMap->value);
                
                for (auto& kv : mutableValue) {
                    Expr* mapKey = kv.first.get();
                    
                    // Compare keys
                    bool keysMatch = false;
                    if (keyExpr->exprType == ExprType::VAR && mapKey->exprType == ExprType::VAR) {
                        Var* k1 = dynamic_cast<Var*>(keyExpr);
                        Var* k2 = dynamic_cast<Var*>(mapKey);
                        keysMatch = (k1->name == k2->name);
                    }
                    else if (keyExpr->exprType == ExprType::STRING && mapKey->exprType == ExprType::VAR) {
                        String* k1 = dynamic_cast<String*>(keyExpr);
                        Var* k2 = dynamic_cast<Var*>(mapKey);
                        keysMatch = (k1->value == k2->name);
                    }
                    else if (keyExpr->exprType == ExprType::VAR && mapKey->exprType == ExprType::STRING) {
                        Var* k1 = dynamic_cast<Var*>(keyExpr);
                        String* k2 = dynamic_cast<String*>(mapKey);
                        keysMatch = (k1->name == k2->value);
                    }
                    
                    if (keysMatch) {
                        // Update existing value
                        cout << "    [EVAL] Updating existing key in map" << endl;
                        kv.second = cloner.cloneExpr(valueExpr);
                        keyFound = true;
                        break;
                    }
                }
                
                if (!keyFound) {
                    cout << "    [EVAL] Adding new key to map" << endl;
                    // Add new key-value pair
                    unique_ptr<Var> newKey;
                    if (keyExpr->exprType == ExprType::VAR) {
                        newKey = make_unique<Var>(dynamic_cast<Var*>(keyExpr)->name);
                    }
                    else if (keyExpr->exprType == ExprType::STRING) {
                        newKey = make_unique<Var>(dynamic_cast<String*>(keyExpr)->value);
                    }
                    else if (keyExpr->exprType == ExprType::SYMVAR) {
                        // Symbolic key - create var with unique name
                        SymVar* sv = dynamic_cast<SymVar*>(keyExpr);
                        newKey = make_unique<Var>("_symkey_" + to_string(sv->getNum()));
                    }
                    
                    if (newKey) {
                        mutableValue.emplace_back(
                            std::move(newKey),
                            cloner.cloneExpr(valueExpr)
                        );
                    }
                }
                
                return clonedMapExpr.release();
            }
            
            // Symbolic case - return symbolic put call
            cout << "    [EVAL] Symbolic put, returning symbolic put()" << endl;
            vector<unique_ptr<Expr>> args;
            args.push_back(cloner.cloneExpr(mapExpr));
            args.push_back(cloner.cloneExpr(keyExpr));
            args.push_back(cloner.cloneExpr(valueExpr));
            return new FuncCall("put", std::move(args));
        }
        
        // SET MEMBERSHIP: in
        if (fc.name == "in" && fc.args.size() == 2) {
            cout << "    [EVAL] Set membership: in" << endl;
            
            Expr* elem = evaluateExpr(*fc.args[0], st);
            Expr* setExpr = evaluateExpr(*fc.args[1], st);
            
            cout << "    [EVAL] Element: " << exprToString(elem) << endl;
            cout << "    [EVAL] Set: " << exprToString(setExpr) << endl;

            if (isConcrete(elem) && setExpr && setExpr->exprType == ExprType::SET) {
            Set* set = dynamic_cast<Set*>(setExpr);
                
                // Check if elem is in set
                for (const auto& setElem : set->elements) {
                    bool match = false;
                    
                    if (elem->exprType == ExprType::VAR && setElem->exprType == ExprType::VAR) {
                        Var* v1 = dynamic_cast<Var*>(elem);
                        Var* v2 = dynamic_cast<Var*>(setElem.get());
                        match = (v1->name == v2->name);
                    }
                    else if (elem->exprType == ExprType::STRING && setElem->exprType == ExprType::STRING) {
                        String* s1 = dynamic_cast<String*>(elem);
                        String* s2 = dynamic_cast<String*>(setElem.get());
                        match = (s1->value == s2->value);
                    }
                    else if (elem->exprType == ExprType::NUM && setElem->exprType == ExprType::NUM) {
                        Num* n1 = dynamic_cast<Num*>(elem);
                        Num* n2 = dynamic_cast<Num*>(setElem.get());
                        match = (n1->value == n2->value);
                    }
                    
                    if (match) {
                        cout << "    [EVAL] Element found in set: true" << endl;
                        return new BoolConst(true); // true
                    }
                }
                
                cout << "    [EVAL] Element not found in set: false" << endl;
                return new BoolConst(false); // false
            }
            
            // Symbolic case
            cout << "    [EVAL] Symbolic in, returning BinaryOpExpr(IN)" << endl;
            return new BinaryOpExpr(
            BinOp::IN,
            cloner.cloneExpr(elem),
            cloner.cloneExpr(setExpr)
    );
}
        
        // NOT_IN operator
if (fc.name == "not_in" && fc.args.size() == 2) {
    cout << "    [EVAL] Set non-membership: not_in" << endl;
    
    Expr* elem = evaluateExpr(*fc.args[0], st);
    Expr* setExpr = evaluateExpr(*fc.args[1], st);
    
    cout << "    [EVAL] Element: " << exprToString(elem) << endl;
    cout << "    [EVAL] Set: " << exprToString(setExpr) << endl;
    
    // Check if BOTH are concrete
    if (isConcrete(elem) && setExpr && setExpr->exprType == ExprType::SET) {
        Set* set = dynamic_cast<Set*>(setExpr);
        
        // Check if elem is in set
        bool found = false;
        for (const auto& setElem : set->elements) {
            bool match = false;
            
            if (elem->exprType == ExprType::VAR && setElem->exprType == ExprType::VAR) {
                Var* v1 = dynamic_cast<Var*>(elem);
                Var* v2 = dynamic_cast<Var*>(setElem.get());
                match = (v1->name == v2->name);
            }
            else if (elem->exprType == ExprType::STRING && setElem->exprType == ExprType::STRING) {
                String* s1 = dynamic_cast<String*>(elem);
                String* s2 = dynamic_cast<String*>(setElem.get());
                match = (s1->value == s2->value);
            }
            else if (elem->exprType == ExprType::NUM && setElem->exprType == ExprType::NUM) {
                Num* n1 = dynamic_cast<Num*>(elem);
                Num* n2 = dynamic_cast<Num*>(setElem.get());
                match = (n1->value == n2->value);
            }
            
            if (match) {
                found = true;
                break;
            }
        }
        
        cout << "    [EVAL] not_in result: " << (!found ? "true" : "false") << endl;
        // FIXED: Return BoolConst instead of Num
        return new BoolConst(!found);
    }
    
    //  Symbolic case - return BinaryOpExpr(NOT_IN)
    cout << "    [EVAL] Symbolic not_in, returning BinaryOpExpr(NOT_IN)" << endl;
    return new BinaryOpExpr(
        BinOp::NOT_IN,
        cloner.cloneExpr(elem),
        cloner.cloneExpr(setExpr)
    );
}

// COMPARISON OPERATIONS
// Eq, Neq, Lt, Le, Gt, Ge
if ((fc.name == "Eq" || fc.name == "=" || fc.name == "==") && fc.args.size() == 2) {
    cout << "    [EVAL] Equality: Eq" << endl;
    
    Expr* left = evaluateExpr(*fc.args[0], st);
    Expr* right = evaluateExpr(*fc.args[1], st);
    
    // If both are concrete, evaluate
    if (isConcrete(left) && isConcrete(right)) {
        bool result = false;
        
        if (left->exprType == ExprType::NUM && right->exprType == ExprType::NUM) {
            result = (dynamic_cast<Num*>(left)->value == dynamic_cast<Num*>(right)->value);
        }
        else if (left->exprType == ExprType::STRING && right->exprType == ExprType::STRING) {
            result = (dynamic_cast<String*>(left)->value == dynamic_cast<String*>(right)->value);
        }
        
        cout << "    [EVAL] Eq result: " << (result ? "true" : "false") << endl;
        return new BoolConst(result);
    }
    
    // Symbolic
    cout << "    [EVAL] Symbolic Eq, returning BinaryOpExpr(EQ)" << endl;
    return new BinaryOpExpr(BinOp::EQ, cloner.cloneExpr(left), cloner.cloneExpr(right));
}

if ((fc.name == "Neq" || fc.name == "!=" || fc.name == "<>") && fc.args.size() == 2) {
    cout << "    [EVAL] Inequality: Neq" << endl;
    
    Expr* left = evaluateExpr(*fc.args[0], st);
    Expr* right = evaluateExpr(*fc.args[1], st);
    
    if (isConcrete(left) && isConcrete(right)) {
        bool result = false;
        
        if (left->exprType == ExprType::NUM && right->exprType == ExprType::NUM) {
            result = (dynamic_cast<Num*>(left)->value != dynamic_cast<Num*>(right)->value);
        }
        else if (left->exprType == ExprType::STRING && right->exprType == ExprType::STRING) {
            result = (dynamic_cast<String*>(left)->value != dynamic_cast<String*>(right)->value);
        }
        
        cout << "    [EVAL] Neq result: " << (result ? "true" : "false") << endl;
        return new BoolConst(result);
    }
    
    cout << "    [EVAL] Symbolic Neq, returning BinaryOpExpr(NEQ)" << endl;
    return new BinaryOpExpr(BinOp::NEQ, cloner.cloneExpr(left), cloner.cloneExpr(right));
}

if ((fc.name == "Lt" || fc.name == "<") && fc.args.size() == 2) {
    cout << "    [EVAL] Less than: Lt" << endl;
    
    Expr* left = evaluateExpr(*fc.args[0], st);
    Expr* right = evaluateExpr(*fc.args[1], st);
    
    if (left->exprType == ExprType::NUM && right->exprType == ExprType::NUM) {
        bool result = (dynamic_cast<Num*>(left)->value < dynamic_cast<Num*>(right)->value);
        cout << "    [EVAL] Lt result: " << (result ? "true" : "false") << endl;
        return new BoolConst(result);
    }
    
    cout << "    [EVAL] Symbolic Lt, returning BinaryOpExpr(LT)" << endl;
    return new BinaryOpExpr(BinOp::LT, cloner.cloneExpr(left), cloner.cloneExpr(right));
}

if ((fc.name == "Le" || fc.name == "<=") && fc.args.size() == 2) {
    cout << "    [EVAL] Less than or equal: Le" << endl;
    
    Expr* left = evaluateExpr(*fc.args[0], st);
    Expr* right = evaluateExpr(*fc.args[1], st);
    
    if (left->exprType == ExprType::NUM && right->exprType == ExprType::NUM) {
        bool result = (dynamic_cast<Num*>(left)->value <= dynamic_cast<Num*>(right)->value);
        cout << "    [EVAL] Le result: " << (result ? "true" : "false") << endl;
        return new BoolConst(result);
    }
    
    cout << "    [EVAL] Symbolic Le, returning BinaryOpExpr(LE)" << endl;
    return new BinaryOpExpr(BinOp::LE, cloner.cloneExpr(left), cloner.cloneExpr(right));
}

if ((fc.name == "Gt" || fc.name == ">") && fc.args.size() == 2) {
    cout << "    [EVAL] Greater than: Gt" << endl;
    
    Expr* left = evaluateExpr(*fc.args[0], st);
    Expr* right = evaluateExpr(*fc.args[1], st);
    
    if (left->exprType == ExprType::NUM && right->exprType == ExprType::NUM) {
        bool result = (dynamic_cast<Num*>(left)->value > dynamic_cast<Num*>(right)->value);
        cout << "    [EVAL] Gt result: " << (result ? "true" : "false") << endl;
        return new BoolConst(result);
    }
    
    cout << "    [EVAL] Symbolic Gt, returning BinaryOpExpr(GT)" << endl;
    return new BinaryOpExpr(BinOp::GT, cloner.cloneExpr(left), cloner.cloneExpr(right));
}

if ((fc.name == "Ge" || fc.name == ">=") && fc.args.size() == 2) {
    cout << "    [EVAL] Greater than or equal: Ge" << endl;
    
    Expr* left = evaluateExpr(*fc.args[0], st);
    Expr* right = evaluateExpr(*fc.args[1], st);
    
    if (left->exprType == ExprType::NUM && right->exprType == ExprType::NUM) {
        bool result = (dynamic_cast<Num*>(left)->value >= dynamic_cast<Num*>(right)->value);
        cout << "    [EVAL] Ge result: " << (result ? "true" : "false") << endl;
        return new BoolConst(result);
    }
    
    cout << "    [EVAL] Symbolic Ge, returning BinaryOpExpr(GE)" << endl;
    return new BinaryOpExpr(BinOp::GE, cloner.cloneExpr(left), cloner.cloneExpr(right));
}

// LOGICAL OPERATIONS
if ((fc.name == "And" || fc.name == "and" || fc.name == "&&") && fc.args.size() == 2) {
    cout << "    [EVAL] Logical AND" << endl;
    
    Expr* left = evaluateExpr(*fc.args[0], st);
    Expr* right = evaluateExpr(*fc.args[1], st);
    
    // If both are BoolConst, evaluate
    if (left->exprType == ExprType::BOOL_CONST && right->exprType == ExprType::BOOL_CONST) {
        bool result = dynamic_cast<BoolConst*>(left)->value && dynamic_cast<BoolConst*>(right)->value;
        cout << "    [EVAL] And result: " << (result ? "true" : "false") << endl;
        return new BoolConst(result);
    }
    
    cout << "    [EVAL] Symbolic And, returning BinaryOpExpr(AND)" << endl;
    return new BinaryOpExpr(BinOp::AND, cloner.cloneExpr(left), cloner.cloneExpr(right));
}

if ((fc.name == "Or" || fc.name == "or" || fc.name == "||") && fc.args.size() == 2) {
    cout << "    [EVAL] Logical OR" << endl;
    
    Expr* left = evaluateExpr(*fc.args[0], st);
    Expr* right = evaluateExpr(*fc.args[1], st);
    
    // If both are BoolConst, evaluate
    if (left->exprType == ExprType::BOOL_CONST && right->exprType == ExprType::BOOL_CONST) {
        bool result = dynamic_cast<BoolConst*>(left)->value || dynamic_cast<BoolConst*>(right)->value;
        cout << "    [EVAL] Or result: " << (result ? "true" : "false") << endl;
        return new BoolConst(result);
    }
    
    cout << "    [EVAL] Symbolic Or, returning BinaryOpExpr(OR)" << endl;
    return new BinaryOpExpr(BinOp::OR, cloner.cloneExpr(left), cloner.cloneExpr(right));
}

if ((fc.name == "Not" || fc.name == "not" || fc.name == "!") && fc.args.size() == 1) {
    cout << "    [EVAL] Logical NOT" << endl;
    
    Expr* operand = evaluateExpr(*fc.args[0], st);
    
    // If BoolConst, evaluate
    if (operand->exprType == ExprType::BOOL_CONST) {
        bool result = !dynamic_cast<BoolConst*>(operand)->value;
        cout << "    [EVAL] Not result: " << (result ? "true" : "false") << endl;
        return new BoolConst(result);
    }
    
    cout << "    [EVAL] Symbolic Not, returning UnaryOpExpr(NOT)" << endl;
    return new UnaryOpExpr(UnOp::NOT, cloner.cloneExpr(operand));
}

    
        // ================================================================
        // END OF MAP OPERATIONS
        
        // Evaluate all arguments
        vector<unique_ptr<Expr>> evaluatedArgs;
        for (size_t i = 0; i < fc.args.size(); i++) {
            cout << "    [EVAL] Arg[" << i << "]: " << exprToString(fc.args[i]) << endl;
            Expr* argResult = evaluateExpr(*fc.args[i], st);
            cout << "    [EVAL] Arg[" << i << "] result: " << exprToString(argResult) << endl;
            evaluatedArgs.push_back(cloner.cloneExpr(argResult));
        }
        
        FuncCall* result = new FuncCall(fc.name, ::move(evaluatedArgs));
        cout << "    [EVAL] FuncCall result: " << exprToString(result) << endl;
        
        return result;
    }
    else if(expr.exprType == ExprType::NUM) {
        Num* result = new Num(dynamic_cast<Num&>(expr).value);
        cout << "  [EVAL] Num: " << exprToString(result) << endl;
        return result;
    }
    else if(expr.exprType == ExprType::STRING) {
        String* result = new String(dynamic_cast<String&>(expr).value);
        cout << "  [EVAL] String: " << exprToString(result) << endl;
        return result;
    }
    else if(expr.exprType == ExprType::SYMVAR) {
        // Return the symbolic variable as-is
        cout << "  [EVAL] SymVar: " << exprToString(&expr) << endl;
        return &expr;
    }
    else if(expr.exprType == ExprType::VAR) {
    Var& v = dynamic_cast<Var&>(expr);
    cout << "  [EVAL] Var lookup: " << v.name << endl;
    
    // First, try direct lookup
    if (sigma.hasValue(v.name)) {
        Expr* value = sigma.getValue(v.name);
        cout << "    [EVAL] Found in sigma: " << exprToString(value) << endl;
        return value;
    }
    
    // NEW: If not found, check if this is a base name with a mapped suffixed name
    auto it = baseNameToSuffixed.find(v.name);
    if (it != baseNameToSuffixed.end()) {
        string suffixedName = it->second;
        cout << "    [EVAL] Resolved base name '" << v.name << "' -> '" << suffixedName << "'" << endl;
        
        if (sigma.hasValue(suffixedName)) {
            Expr* value = sigma.getValue(suffixedName);
            cout << "    [EVAL] Found in sigma: " << exprToString(value) << endl;
            return value;
        }
    }
    
    cout << "    [EVAL] Not found in sigma, returning as-is" << endl;
    return &expr;
}
    else if(expr.exprType == ExprType::SET) {
        // Evaluate each element in the set
        Set& set = dynamic_cast<Set&>(expr);
        cout << "  [EVAL] Set with " << set.elements.size() << " elements" << endl;
        
        vector<unique_ptr<Expr>> evaluatedElements;
        for (size_t i = 0; i < set.elements.size(); i++) {
            Expr* elemResult = evaluateExpr(*set.elements[i], st);
            evaluatedElements.push_back(cloner.cloneExpr(elemResult));
        }
        
        Set* result = new Set(::move(evaluatedElements));
        cout << "    [EVAL] Set result: " << exprToString(result) << endl;
        return result;
    }
    else if(expr.exprType == ExprType::MAP) {
        // Evaluate each key-value pair in the map
        Map& map = dynamic_cast<Map&>(expr);
        cout << "  [EVAL] Map with " << map.value.size() << " entries" << endl;
        
        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> evaluatedPairs;
        for (size_t i = 0; i < map.value.size(); i++) {
            // Clone the key (Var)
            unique_ptr<Var> keyClone = make_unique<Var>(map.value[i].first->name);
            // Evaluate the value
            Expr* valResult = evaluateExpr(*map.value[i].second, st);
            evaluatedPairs.push_back(make_pair(::move(keyClone), cloner.cloneExpr(valResult)));
        }
        
        Map* result = new Map(::move(evaluatedPairs));
        cout << "    [EVAL] Map result: " << exprToString(result) << endl;
        return result;
    }
    else if(expr.exprType == ExprType::TUPLE) {
        // Evaluate each element in the tuple
        Tuple& tuple = dynamic_cast<Tuple&>(expr);
        cout << "  [EVAL] Tuple with " << tuple.exprs.size() << " elements" << endl;
        
        vector<unique_ptr<Expr>> evaluatedExprs;
        for (size_t i = 0; i < tuple.exprs.size(); i++) {
            Expr* elemResult = evaluateExpr(*tuple.exprs[i], st);
            evaluatedExprs.push_back(cloner.cloneExpr(elemResult));
        }
        
        Tuple* result = new Tuple(::move(evaluatedExprs));
        cout << "    [EVAL] Tuple result: " << exprToString(result) << endl;
        return result;
    }

                  // Add Boolean expression types
                    else if(expr.exprType == ExprType::BOOL_CONST) {
                    BoolConst* result = new BoolConst(dynamic_cast<BoolConst&>(expr).value);
                    cout << "  [EVAL] BoolConst: " << exprToString(result) << endl;
                    return result;
                    }
                    else if(expr.exprType == ExprType::BINARY_OP) {
                    BinaryOpExpr& binop = dynamic_cast<BinaryOpExpr&>(expr);
                    cout << "  [EVAL] BinaryOpExpr" << endl;
        
                    // Evaluate operands
                    Expr* left = evaluateExpr(*binop.left, st);
                    Expr* right = evaluateExpr(*binop.right, st);
        
                    // Return new BinaryOpExpr with evaluated operands
                    return new BinaryOpExpr(
                    binop.op,
                    cloner.cloneExpr(left),
                    cloner.cloneExpr(right)
                    );
                    }
                    else if(expr.exprType == ExprType::UNARY_OP) {
                    UnaryOpExpr& unop = dynamic_cast<UnaryOpExpr&>(expr);
                    cout << "  [EVAL] UnaryOpExpr" << endl;
        
                    // Evaluate operand
                    Expr* operand = evaluateExpr(*unop.operand, st);
        
                    // Return new UnaryOpExpr with evaluated operand
                    return new UnaryOpExpr(
                    unop.op,
                    cloner.cloneExpr(operand)
                    );
                    }
                    
    
    // Default case: return the expression as-is
    cout << "  [EVAL] Unknown type, returning as-is" << endl;
    return &expr;
}

// Extract base name: "email0" -> "email", "password1" -> "password"
string SEE::extractBaseName(const string& suffixedName) {
    // Find trailing digits
    size_t i = suffixedName.length();
    while (i > 0 && isdigit(suffixedName[i - 1])) {
        i--;
    }
    
    if (i == suffixedName.length()) {
        // No trailing digits, return as-is
        return suffixedName;
    }
    
    return suffixedName.substr(0, i);
}
