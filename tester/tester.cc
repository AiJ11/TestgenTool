#include "tester.hh"
#include "../rewrite_globals_visitor.hh"
#include "../algo.hpp" 
#include "../clonevisitor.hh"
#include "../printvisitor.hh"
#include <iostream>


// Generate realistic test values based on variable name hints
Expr* generateRealisticValue(const string& varName, int index) {
    // Extract base name (email0 -> email)
    string baseName = varName;
    size_t i = varName.length();
    while (i > 0 && isdigit(varName[i - 1])) {
        i--;
    }
    if (i < varName.length()) {
        baseName = varName.substr(0, i);
    }
    
    string suffix = to_string(index);
    
    if (baseName == "email") {
        return new String("testuser" + suffix + "@example.com");
    }
    else if (baseName == "password") {
        return new String("Password" + suffix + "!");
    }
    else if (baseName == "fullName") {
        return new String("Test User " + suffix);
    }
    else if (baseName == "mobile") {
        // Generate 10-digit mobile number (no dashes)
        // Format: 555000XXXX where XXXX is padded index
        string paddedIndex = suffix;
        while (paddedIndex.length() < 4) {
            paddedIndex = "0" + paddedIndex;
        }
        // Ensure exactly 10 digits
        return new String("555000" + paddedIndex.substr(paddedIndex.length() - 4));
    }
    else if (baseName == "token") {
        return new String("token_" + suffix);
    }
    else if (baseName == "name") {
        return new String("TestName" + suffix);
    }
    else if (baseName == "address") {
        return new String("123 Test Street " + suffix);
    }
    else if (baseName == "contact") {
        return new String("contact" + suffix + "@test.com");
    }
    else {
        // Default string value
        return new String("value_" + suffix);
    }
}
void Tester::generateTest() {}

// Check if a statement is an Input statement (x := input())
bool isInputStmt(const Stmt& stmt) {
    if(stmt.statementType == StmtType::ASSIGN) {
        const Assign* assign = dynamic_cast<const Assign*>(&stmt);
        if(assign && assign->right->exprType == ExprType::FUNCCALL) {
            const FuncCall* fc = dynamic_cast<const FuncCall*>(assign->right.get());
            if(fc) {
                return (fc->name == "input" && fc->args.size() == 0);
            }
        }
    }
    return false;
}

// Check if the test case has at least one input statement
bool isAbstract(const Program& prog) {
    for(const auto& stmt : prog.statements) {
        if(isInputStmt(*stmt)) {
            return true;
        }
    }
    return false;
}

// Generate Concrete Test Case (genCTC)
// function genCTC(t, L, σ)
//   if ¬isAbstract(t) then return t
//   else
//     t' ← rewriteATC(t, L)
//     L' ← symex(t', σ)
//     return getCTC(t', L', σ)
unique_ptr<Program> Tester::generateCTC(unique_ptr<Program> atc, vector<Expr*> ConcreteVals, ValueEnvironment* ve) {
    cout << "\n========================================" << endl;
    cout << ">>> generateCTC: Starting iteration" << endl;
    cout << "========================================" << endl;
    
    // If not abstract (no input statements), return as-is
    if(!isAbstract(*atc)) {
        cout << ">>> generateCTC: Program is concrete, returning" << endl;
        return atc;
    }
    
    cout << ">>> generateCTC: Program is abstract, needs concretization" << endl;
    cout << ">>> generateCTC: Concrete values provided: " << ConcreteVals.size() << endl;
    
    // Rewrite the abstract test case by replacing Input statements with concrete values
    cout << "\n>>> generateCTC: STEP 1 - Rewriting ATC with concrete values" << endl;
    unique_ptr<Program> rewritten = rewriteATC(atc, ConcreteVals);
    
    // Run symbolic execution on the rewritten test case using class member
    cout << "\n>>> generateCTC: STEP 2 - Running symbolic execution" << endl;
    SymbolTable st(nullptr);
    see.execute(*rewritten, st);
    
    // Get the path constraints from symbolic execution and store in class member
    pathConstraints = see.getPathConstraint();
    unique_ptr<Expr> pathConstraint = see.computePathConstraint();
    
    // Solve the path constraints to get new concrete values using class member
    cout << "\n>>> generateCTC: STEP 3 - Solving path constraints with Z3" << endl;
    Result result = solver.solve(std::move(pathConstraint));
    
    // Extract concrete values from the solver result
    vector<Expr*> newConcreteVals;
    if(result.isSat) {
        cout << ">>> generateCTC: SAT - Extracting " << result.model.size() << " concrete values" << endl;
        
        // Extract values from the model in order (X0, X1, X2, ...)
        for(const auto& entry : result.model) {
            if(entry.second->type == ResultType::INT) {
                const IntResultValue* intVal = dynamic_cast<const IntResultValue*>(entry.second.get());
                cout << "    " << entry.first << " = " << intVal->value << endl;
                
                // CHANGE 1: Generate realistic string value instead of using the integer
                // Find which input variable this corresponds to
                int varIndex = 0;
                try {
                    // Extract number from "X0", "X1", etc.
                    string varName = entry.first;
                    if (varName.length() > 1 && varName[0] == 'X') {
                        varIndex = stoi(varName.substr(1));
                    }
                } catch (...) {
                    varIndex = newConcreteVals.size();
                }
                
                // Find the corresponding input statement to get variable name
                int inputIndex = 0;
                string targetVarName = "unknown";
                for(const auto& stmt : rewritten->statements) {
                    if(isInputStmt(*stmt)) {
                        if(inputIndex == varIndex) {
                            const Assign* assign = dynamic_cast<const Assign*>(stmt.get());
                            const Var* leftVar = dynamic_cast<const Var*>(assign->left.get());
                            if(leftVar) {
                                targetVarName = leftVar->name;
                            }
                            break;
                        }
                        inputIndex++;
                    }
                }
                
                // Generate realistic value based on variable name
                Expr* realisticValue = generateRealisticValue(targetVarName, varIndex);
                newConcreteVals.push_back(realisticValue);
                
                if(realisticValue->exprType == ExprType::STRING) {
                    cout << "    -> Converted to: \"" << dynamic_cast<String*>(realisticValue)->value << "\"" << endl;
                }
            }
        }
        
        // CHANGE 2: If no values from Z3 but we still have input() statements,
        // generate realistic default values for remaining inputs
        if(newConcreteVals.empty() && isAbstract(*rewritten)) {
    cout << ">>> generateCTC: No constrained values, generating defaults for remaining inputs" << endl;
    
    // STEP 1: Build a map of base names to concrete values from ALREADY REWRITTEN statements
    map<string, Expr*> baseNameToValue;
    
    for(const auto& stmt : rewritten->statements) {
        if(stmt->statementType == StmtType::ASSIGN) {
            const Assign* assign = dynamic_cast<const Assign*>(stmt.get());
            if(!assign) continue;
            
            const Var* leftVar = dynamic_cast<const Var*>(assign->left.get());
            if(!leftVar) continue;
            
            // Skip if RHS is input() - not concrete yet
            if(assign->right->exprType == ExprType::FUNCCALL) {
                const FuncCall* fc = dynamic_cast<const FuncCall*>(assign->right.get());
                if(fc && fc->name == "input") continue;
            }
            
            // This is a concrete assignment like: email0 := "testuser0@example.com"
            string varName = leftVar->name;
            
            // Extract base name (email0 -> email)
            string baseName = varName;
            size_t i = varName.length();
            while (i > 0 && isdigit(varName[i - 1])) i--;
            if (i < varName.length()) baseName = varName.substr(0, i);
            
            // Store first concrete value for this base name
            if(baseNameToValue.find(baseName) == baseNameToValue.end()) {
                baseNameToValue[baseName] = assign->right.get();
                cout << "    [Found existing] " << baseName << " -> " << varName << endl;
            }
        }
    }
    
    // STEP 2: Generate values for remaining input() statements
    int inputCount = 0;
    for(const auto& stmt : rewritten->statements) {
        if(isInputStmt(*stmt)) {
            const Assign* assign = dynamic_cast<const Assign*>(stmt.get());
            const Var* leftVar = dynamic_cast<const Var*>(assign->left.get());
            string varName = leftVar ? leftVar->name : "unknown";
            
            // Extract base name
            string baseName = varName;
            size_t i = varName.length();
            while (i > 0 && isdigit(varName[i - 1])) i--;
            if (i < varName.length()) baseName = varName.substr(0, i);
            
            Expr* value;
            
            // Check if we already have a concrete value for this base name
            if (baseNameToValue.find(baseName) != baseNameToValue.end()) {
                // REUSE the existing value!
                value = baseNameToValue[baseName];
                cout << "    " << varName << " = (reusing " << baseName << ") ";
            } else {
                // Generate NEW realistic value and store for future reuse
                value = generateRealisticValue(varName, inputCount);
                baseNameToValue[baseName] = value;
                cout << "    " << varName << " = ";
            }
            
            newConcreteVals.push_back(value);
            
            if(value->exprType == ExprType::STRING) {
                cout << "\"" << dynamic_cast<String*>(value)->value << "\"" << endl;
            } else if(value->exprType == ExprType::NUM) {
                cout << dynamic_cast<Num*>(value)->value << endl;
            }
            inputCount++;
        }
    }
}
    } else {
        cout << ">>> generateCTC: UNSAT - No solution found, cannot continue" << endl;
    }
    
    // If we didn't get any new concrete values, we can't make progress
    if(newConcreteVals.empty()) {
        cout << ">>> generateCTC: No new concrete values, returning partially rewritten program" << endl;
        return rewritten;
    }
    
    // Recursively generate CTC with the new concrete values
    cout << "\n>>> generateCTC: STEP 4 - Recursing with " << newConcreteVals.size() << " new concrete values" << endl;
    return generateCTC(std::move(rewritten), newConcreteVals, ve);
}

// Generate Abstract Test Case from specification
unique_ptr<Program> Tester::generateATC(
    unique_ptr<Spec> spec,
    vector<string> ts
) {
    // Step 1: generate logical ATC
    Program raw = genATC(*spec, ts);

    // Step 2: move to heap
    auto& mutable_stmts = const_cast<vector<unique_ptr<Stmt>>&>(raw.statements);
auto logicalATC = unique_ptr<Program>(new Program(std::move(mutable_stmts)));

        PrintVisitor printer;  // ← Declare ONCE at the top

    // Step 3: rewrite globals -> Test API
    RewriteGlobalsVisitor rewriter;
    rewriter.visitProgram(*logicalATC);   // runs rewrite on logical ATC

    unique_ptr<Program> testApiATC = std::move(rewriter.rewrittenProgram);

    // DEBUG: Print Test-API ATC
    cout << "\n=== TEST-API ATC (After Rewrite) ===" << endl;
    printer.visitProgram(*testApiATC);

    // Step 4: return rewritten ATC
    return testApiATC;
}

// Rewrite Abstract Test Case (rewriteATC)
// function rewriteATC(t, L)
//   if |t| = 0 ∧ |L| ≠ 0 then raise Error
//   match s₁ with
//   | case Input(x) ⇒
//       s'₁ ← Assign(x, v₁)
//       return s'₁ :: rewriteATC([s₂; ...; sₙ] [v₂; ...; vₘ])
//   | _ ⇒ return s₁ :: rewriteATC([s₂; ...; sₙ] [v₁; ...; vₘ])
unique_ptr<Program> Tester::rewriteATC(unique_ptr<Program>& atc, vector<Expr*> ConcreteVals) {
    // Check error condition: empty test case but concrete values provided
    if(atc->statements.size() == 0 && ConcreteVals.size() != 0) {
        throw runtime_error("Empty test case but concrete values provided");
    }
    
    // Create a new program with rewritten statements
    vector<unique_ptr<Stmt>> newStmts;
    int concreteValIndex = 0;
    CloneVisitor cloner;
    
    for(int i = 0; i < atc->statements.size(); i++) {
        const auto& stmt = atc->statements[i];

        // Check if this is an Input statement (x := input())
        if(stmt->statementType == StmtType::ASSIGN) {
            Assign* assign = dynamic_cast<Assign*>(stmt.get());
            
            if(assign && assign->right->exprType == ExprType::FUNCCALL) {
                FuncCall* fc = dynamic_cast<FuncCall*>(assign->right.get());
                
                // If it's input() and we have concrete values, replace it
                if(fc && fc->name == "input" && fc->args.size() == 0) {
                    if(concreteValIndex < ConcreteVals.size()) {
                        // Create new assignment: x := concreteValue
                        Var* leftVarPtr = dynamic_cast<Var*>(assign->left.get());
                        if (!leftVarPtr) {
                            throw runtime_error("Expected Var on left side of input assignment");
                        }
                        unique_ptr<Var> leftVar = make_unique<Var>(leftVarPtr->name);
                        unique_ptr<Expr> rightExpr = cloner.cloneExpr(ConcreteVals[concreteValIndex]);
                        
                        newStmts.push_back(make_unique<Assign>(move(leftVar), move(rightExpr)));
                        concreteValIndex++;
                    } else {
                        // No concrete value available yet, keep the input() statement
                        newStmts.push_back(cloner.cloneStmt(stmt.get()));
                    }
                    continue;
                }
            }
        }
        
        // For all other statements, clone them as-is
        newStmts.push_back(cloner.cloneStmt(stmt.get()));
    }
    
    return make_unique<Program>(move(newStmts));
}
