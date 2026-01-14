#include "rewrite_globals_visitor.hh"
#include "clonevisitor.hh"
#include <iostream>

using namespace std;

/* ============================================================
 * CONSTRUCTOR
 * ============================================================ */

RewriteGlobalsVisitor::RewriteGlobalsVisitor() {}

/* ============================================================
 * HELPER: Fresh Temporary Variable
 * ============================================================ */

string RewriteGlobalsVisitor::freshTemp(const string& globalName) {
    int& counter = tmpCounters[globalName];
    string result = "tmp_" + globalName + "_" + to_string(counter);
    counter++;
    return result;
}

/* ============================================================
 * HELPER: Detect Init Assignment
 * ============================================================ */

bool RewriteGlobalsVisitor::isInitAssign(const Stmt* stmt) {
    const Assign* as = dynamic_cast<const Assign*>(stmt);
    if (!as) return false;

    const Var* lhs = dynamic_cast<const Var*>(as->left.get());
    const Map* rhs = dynamic_cast<const Map*>(as->right.get());

    if (!lhs || !rhs) return false;

    // Empty map means initialization
    return rhs->value.empty();
}

/* ============================================================
 * HELPER: Check if expression is a global variable
 * ============================================================ */

bool RewriteGlobalsVisitor::isGlobalVar(const Expr* expr) {
    if (!expr) return false;
    
    const Var* v = dynamic_cast<const Var*>(expr);
    if (!v) return false;
    
    return globals.count(v->name) > 0;
}

/* ============================================================
 * HELPER: Check if expression contains globals
 * ============================================================ */

bool RewriteGlobalsVisitor::containsGlobals(const Expr* expr) {
    if (!expr) return false;
    
    // Check if it's a global variable
    if (isGlobalVar(expr)) return true;
    
    // Recursively check function call arguments
    if (const FuncCall* fc = dynamic_cast<const FuncCall*>(expr)) {
        for (const auto& arg : fc->args) {
            if (containsGlobals(arg.get())) return true;
        }
    }
    
    // Check tuple elements
    if (const Tuple* t = dynamic_cast<const Tuple*>(expr)) {
        for (const auto& e : t->exprs) {
            if (containsGlobals(e.get())) return true;
        }
    }
    
    // Check set elements
    if (const Set* s = dynamic_cast<const Set*>(expr)) {
        for (const auto& e : s->elements) {
            if (containsGlobals(e.get())) return true;
        }
    }
    
    // Check map values
    if (const Map* m = dynamic_cast<const Map*>(expr)) {
        for (const auto& kv : m->value) {
            if (containsGlobals(kv.first.get())) return true;
            if (containsGlobals(kv.second.get())) return true;
        }
    }
    
    return false;
}

void RewriteGlobalsVisitor::visitBoolConst(const BoolConst &node) {
    // No-op: actual rewriting happens in rewriteExpr()
}

void RewriteGlobalsVisitor::visitBinaryOpExpr(const BinaryOpExpr &node) {
    // No-op: actual rewriting happens in rewriteExpr()
}

void RewriteGlobalsVisitor::visitUnaryOpExpr(const UnaryOpExpr &node) {
    // No-op: actual rewriting happens in rewriteExpr()
}

/* ============================================================
 * MAIN ENTRY POINT: visitProgram
 * ============================================================ */

void RewriteGlobalsVisitor::visitProgram(const Program& p) {
    // STEP 1: Detect globals from init statements
    globals.clear();
    tmpCounters.clear();
    newStmts.clear();
    
    for (const auto& stmt : p.statements) {
        if (isInitAssign(stmt.get())) {
            const Assign* as = dynamic_cast<const Assign*>(stmt.get());
            const Var* lhs = dynamic_cast<const Var*>(as->left.get());
            globals.insert(lhs->name);
            tmpCounters[lhs->name] = 0;  // Initialize counter
        }
    }
    
    cout << "[RewriteGlobalsVisitor] Detected " << globals.size() << " globals: ";
    for (const auto& g : globals) cout << g << " ";
    cout << endl;
    
    // STEP 2: Insert reset() call
    {
        vector<unique_ptr<Expr>> noArgs;
        auto resetCall = make_unique<FuncCall>("reset", move(noArgs));
        newStmts.push_back(
            make_unique<Assign>(
                make_unique<Var>("_"),
                move(resetCall)
            )
        );
    }
    
    // STEP 3: Process all non-init statements
    for (const auto& stmt : p.statements) {
        // Skip init statements
        if (isInitAssign(stmt.get())) {
            continue;
        }
        
        // Dispatch to statement rewriters
        this->visit(stmt.get());
    }
    
    // STEP 4: Create final program
    rewrittenProgram = make_unique<Program>(move(newStmts));
    
    cout << "[RewriteGlobalsVisitor] Generated " << newStmts.size() 
         << " statements in rewritten program" << endl;
}

/* ============================================================
 * STATEMENT VISITORS
 * ============================================================ */

void RewriteGlobalsVisitor::visitAssign(const Assign& s) {
    rewriteAssignStmt(s);
}

void RewriteGlobalsVisitor::visitAssume(const Assume& s) {
    // Rewrite the condition expression
    auto result = rewriteExpr(s.expr.get());
    
    // Add hoisted statements first
    for (auto& stmt : result.hoistedStmts) {
        newStmts.push_back(move(stmt));
    }
    
    // Then add the assume with rewritten condition
    newStmts.push_back(
        make_unique<Assume>(move(result.expr))
    );
}

void RewriteGlobalsVisitor::visitAssert(const Assert& s) {
    // Rewrite the condition expression
    auto result = rewriteExpr(s.expr.get());
    
    // Add hoisted statements first
    for (auto& stmt : result.hoistedStmts) {
        newStmts.push_back(move(stmt));
    }
    
    // Then add the assert with rewritten condition
    newStmts.push_back(
        make_unique<Assert>(move(result.expr))
    );
}

/* ============================================================
 * ASSIGNMENT REWRITING
 * ============================================================ */

void RewriteGlobalsVisitor::rewriteAssignStmt(const Assign& s) {
    // Detect if this is a global map update
    auto updateInfo = detectMapUpdate(s.left.get(), s.right.get());
    
    if (updateInfo.isMapUpdate) {
        // Case: G[k] = v
        emitMapUpdate(
            updateInfo.globalName,
            move(updateInfo.key),
            move(updateInfo.value)
        );
    } else if (!updateInfo.globalName.empty()) {
        // Case: G = expr
        emitMapReplace(
            updateInfo.globalName,
            move(updateInfo.value)
        );
    } else {
        // Case: Regular assignment (not a global)
        // Rewrite RHS
        auto rhsResult = rewriteExpr(s.right.get());
        
        // Add hoisted statements
        for (auto& stmt : rhsResult.hoistedStmts) {
            newStmts.push_back(move(stmt));
        }
        
        // Clone LHS
        CloneVisitor cloner;
        auto lhsClone = cloner.cloneExpr(s.left.get());
        
        // Add assignment
        newStmts.push_back(
            make_unique<Assign>(
                move(lhsClone),
                move(rhsResult.expr)
            )
        );
    }
}

/* ============================================================
 * DETECT MAP UPDATE
 * ============================================================ */

RewriteGlobalsVisitor::MapUpdateInfo
RewriteGlobalsVisitor::detectMapUpdate(const Expr* lhs, const Expr* rhs) {
    MapUpdateInfo info;
    info.globalName = "";
    info.isMapUpdate = false;
    
    // Case 1: Direct global assignment (G = expr)
    if (const Var* v = dynamic_cast<const Var*>(lhs)) {
        if (globals.count(v->name)) {
            info.globalName = v->name;
            info.isMapUpdate = false;
            
            // Rewrite RHS
            auto rhsResult = rewriteExpr(rhs);
            info.value = move(rhsResult.expr);
            
            // Hoist statements from RHS rewrite
            // (Will be added by caller)
            // For now, we'll handle this in emitMapReplace
            
            return info;
        }
    }
    
    // Case 2: Map index assignment (G[k] = v)
    // This comes as: [] (G, k) on LHS
    if (const FuncCall* fc = dynamic_cast<const FuncCall*>(lhs)) {
        if (fc->name == "[]" && fc->args.size() == 2) {
            const Expr* base = fc->args[0].get();
            const Expr* keyExpr = fc->args[1].get();
            
            // Check if base is a global
            if (const Var* baseVar = dynamic_cast<const Var*>(base)) {
                if (globals.count(baseVar->name)) {
                    info.globalName = baseVar->name;
                    info.isMapUpdate = true;
                    
                    // Clone key and value
                    CloneVisitor cloner;
                    info.key = cloner.cloneExpr(keyExpr);
                    info.value = cloner.cloneExpr(rhs);
                    
                    return info;
                }
            }
        }
    }
    
    return info;
}

/* ============================================================
 * EMIT MAP UPDATE: G[k] = v
 * ============================================================ */

void RewriteGlobalsVisitor::emitMapUpdate(
    const string& globalName,
    unique_ptr<Expr> key,
    unique_ptr<Expr> value
) {
    string tmpName = freshTemp(globalName);
    
    // STEP 1: tmp := get_G()
    {
        vector<unique_ptr<Expr>> noArgs;
        auto getCall = make_unique<FuncCall>("get_" + globalName, move(noArgs));
        
        newStmts.push_back(
            make_unique<Assign>(
                make_unique<Var>(tmpName),
                move(getCall)
            )
        );
    }
    
    // STEP 2: tmp[k] := v
    {
        vector<unique_ptr<Expr>> indexArgs;
        indexArgs.push_back(make_unique<Var>(tmpName));
        indexArgs.push_back(move(key));
        
        auto indexExpr = make_unique<FuncCall>("[]", move(indexArgs));
        
        newStmts.push_back(
            make_unique<Assign>(
                move(indexExpr),
                move(value)
            )
        );
    }
    
    // STEP 3: _ := set_G(tmp)
    {
        vector<unique_ptr<Expr>> setArgs;
        setArgs.push_back(make_unique<Var>(tmpName));
        
        auto setCall = make_unique<FuncCall>("set_" + globalName, move(setArgs));
        
        newStmts.push_back(
            make_unique<Assign>(
                make_unique<Var>("_"),
                move(setCall)
            )
        );
    }
    
    cout << "[RewriteGlobalsVisitor] Emitted map update for " << globalName << endl;
}

/* ============================================================
 * EMIT MAP REPLACE: G = expr
 * ============================================================ */

void RewriteGlobalsVisitor::emitMapReplace(
    const string& globalName,
    unique_ptr<Expr> expr
) {
    // Rewrite the expression first
    auto exprResult = rewriteExpr(expr.get());
    
    // Add hoisted statements
    for (auto& stmt : exprResult.hoistedStmts) {
        newStmts.push_back(move(stmt));
    }
    
    // _ := set_G(expr')
    vector<unique_ptr<Expr>> setArgs;
    setArgs.push_back(move(exprResult.expr));
    
    auto setCall = make_unique<FuncCall>("set_" + globalName, move(setArgs));
    
    newStmts.push_back(
        make_unique<Assign>(
            make_unique<Var>("_"),
            move(setCall)
        )
    );
    
    cout << "[RewriteGlobalsVisitor] Emitted map replace for " << globalName << endl;
}

/* ============================================================
 * EXPRESSION REWRITING (WITH HOISTING)
 * ============================================================ */

RewriteGlobalsVisitor::RewriteResult
RewriteGlobalsVisitor::rewriteExpr(const Expr* e) {
    if (!e) {
        RewriteResult result;
        result.expr = nullptr;
        return result;
    }
    
    // Dispatch based on expression type
    if (auto v = dynamic_cast<const Var*>(e)) {
        return rewriteVar(v);
    }
    if (auto f = dynamic_cast<const FuncCall*>(e)) {
        return rewriteFuncCall(f);
    }
    if (auto n = dynamic_cast<const Num*>(e)) {
        return rewriteNum(n);
    }
    if (auto s = dynamic_cast<const String*>(e)) {
        return rewriteString(s);
    }
    if (auto t = dynamic_cast<const Tuple*>(e)) {
        return rewriteTuple(t);
    }
    if (auto s = dynamic_cast<const Set*>(e)) {
        return rewriteSet(s);
    }
    if (auto m = dynamic_cast<const Map*>(e)) {
        return rewriteMap(m);
    }
    if (auto bc = dynamic_cast<const BoolConst*>(e)) {
        return rewriteBoolConst(bc);
    }
    if (auto binop = dynamic_cast<const BinaryOpExpr*>(e)) {
        return rewriteBinaryOpExpr(binop);
    }
    if (auto unop = dynamic_cast<const UnaryOpExpr*>(e)) {
        return rewriteUnaryOpExpr(unop);
    }
    
    
    // Default: clone as-is
    CloneVisitor cloner;
    RewriteResult result;
    result.expr = cloner.cloneExpr(e);
    return result;
}

/* ============================================================
 * REWRITE VAR
 * ============================================================ */

RewriteGlobalsVisitor::RewriteResult
RewriteGlobalsVisitor::rewriteVar(const Var* v) {
    RewriteResult result;
    
    // If it's a global, hoist get_G() call
    if (globals.count(v->name)) {
        string tmpName = freshTemp(v->name);
        
        // Hoist: tmp := get_G()
        vector<unique_ptr<Expr>> noArgs;
        auto getCall = make_unique<FuncCall>("get_" + v->name, move(noArgs));
        
        result.hoistedStmts.push_back(
            make_unique<Assign>(
                make_unique<Var>(tmpName),
                move(getCall)
            )
        );
        
        // Return tmp
        result.expr = make_unique<Var>(tmpName);
    } else {
        // Not a global, clone as-is
        result.expr = make_unique<Var>(v->name);
    }
    
    return result;
}

/* ============================================================
 * REWRITE FUNCCALL
 * ============================================================ */

RewriteGlobalsVisitor::RewriteResult
RewriteGlobalsVisitor::rewriteFuncCall(const FuncCall* f) {
    // Special cases
    if (f->name == "[]") {
        return rewriteMapAccess(f);
    }
    if (f->name == "dom") {
        return rewriteDom(f);
    }
    
    // General function call: rewrite all arguments
    RewriteResult result;
    vector<unique_ptr<Expr>> newArgs;
    
    for (const auto& arg : f->args) {
        auto argResult = rewriteExpr(arg.get());
        
        // Collect hoisted statements
        for (auto& stmt : argResult.hoistedStmts) {
            result.hoistedStmts.push_back(move(stmt));
        }
        
        // Collect rewritten argument
        newArgs.push_back(move(argResult.expr));
    }
    
    // Create new function call with rewritten args
    result.expr = make_unique<FuncCall>(f->name, move(newArgs));
    
    return result;
}

/* ============================================================
 * REWRITE MAP ACCESS: G[k]
 * ============================================================ */

RewriteGlobalsVisitor::RewriteResult
RewriteGlobalsVisitor::rewriteMapAccess(const FuncCall* f) {
    RewriteResult result;
    
    if (f->args.size() != 2) {
        // Malformed [], shouldn't happen
        CloneVisitor cloner;
        result.expr = cloner.cloneExpr(f);
        return result;
    }
    
    const Expr* base = f->args[0].get();
    const Expr* key = f->args[1].get();
    
    // Check if base is a global
    if (const Var* baseVar = dynamic_cast<const Var*>(base)) {
        if (globals.count(baseVar->name)) {
            // Hoist: tmp := get_G()
            string tmpName = freshTemp(baseVar->name);
            
            vector<unique_ptr<Expr>> noArgs;
            auto getCall = make_unique<FuncCall>("get_" + baseVar->name, move(noArgs));
            
            result.hoistedStmts.push_back(
                make_unique<Assign>(
                    make_unique<Var>(tmpName),
                    move(getCall)
                )
            );
            
            // Rewrite key
            auto keyResult = rewriteExpr(key);
            for (auto& stmt : keyResult.hoistedStmts) {
                result.hoistedStmts.push_back(move(stmt));
            }
            
            // Return: tmp[key']
            vector<unique_ptr<Expr>> indexArgs;
            indexArgs.push_back(make_unique<Var>(tmpName));
            indexArgs.push_back(move(keyResult.expr));
            
            result.expr = make_unique<FuncCall>("[]", move(indexArgs));
            return result;
        }
    }
    
    // Not a global, rewrite base and key recursively
    auto baseResult = rewriteExpr(base);
    auto keyResult = rewriteExpr(key);
    
    for (auto& stmt : baseResult.hoistedStmts) {
        result.hoistedStmts.push_back(move(stmt));
    }
    for (auto& stmt : keyResult.hoistedStmts) {
        result.hoistedStmts.push_back(move(stmt));
    }
    
    vector<unique_ptr<Expr>> indexArgs;
    indexArgs.push_back(move(baseResult.expr));
    indexArgs.push_back(move(keyResult.expr));
    
    result.expr = make_unique<FuncCall>("[]", move(indexArgs));
    return result;
}

/* ============================================================
 * REWRITE DOM: dom(G)
 * ============================================================ */

RewriteGlobalsVisitor::RewriteResult
RewriteGlobalsVisitor::rewriteDom(const FuncCall* f) {
    RewriteResult result;
    
    if (f->args.size() != 1) {
        CloneVisitor cloner;
        result.expr = cloner.cloneExpr(f);
        return result;
    }
    
    const Expr* base = f->args[0].get();
    
    // Check if base is a global
    if (const Var* baseVar = dynamic_cast<const Var*>(base)) {
        if (globals.count(baseVar->name)) {
            // Hoist: tmp := get_G()
            string tmpName = freshTemp(baseVar->name);
            
            vector<unique_ptr<Expr>> noArgs;
            auto getCall = make_unique<FuncCall>("get_" + baseVar->name, move(noArgs));
            
            result.hoistedStmts.push_back(
                make_unique<Assign>(
                    make_unique<Var>(tmpName),
                    move(getCall)
                )
            );
            
            // Return: dom(tmp)
            vector<unique_ptr<Expr>> domArgs;
            domArgs.push_back(make_unique<Var>(tmpName));
            
            result.expr = make_unique<FuncCall>("dom", move(domArgs));
            return result;
        }
    }
    
    // Not a global, rewrite argument
    auto argResult = rewriteExpr(base);
    
    for (auto& stmt : argResult.hoistedStmts) {
        result.hoistedStmts.push_back(move(stmt));
    }
    
    vector<unique_ptr<Expr>> domArgs;
    domArgs.push_back(move(argResult.expr));
    
    result.expr = make_unique<FuncCall>("dom", move(domArgs));
    return result;
}

/* ============================================================
 * REWRITE NUM, STRING (Literals - No Rewrite Needed)
 * ============================================================ */

RewriteGlobalsVisitor::RewriteResult
RewriteGlobalsVisitor::rewriteNum(const Num* n) {
    RewriteResult result;
    result.expr = make_unique<Num>(n->value);
    return result;
}

RewriteGlobalsVisitor::RewriteResult
RewriteGlobalsVisitor::rewriteString(const String* s) {
    RewriteResult result;
    result.expr = make_unique<String>(s->value);
    return result;
}

/* ============================================================
 * REWRITE TUPLE
 * ============================================================ */

RewriteGlobalsVisitor::RewriteResult
RewriteGlobalsVisitor::rewriteTuple(const Tuple* t) {
    RewriteResult result;
    vector<unique_ptr<Expr>> newElements;
    
    for (const auto& elem : t->exprs) {
        auto elemResult = rewriteExpr(elem.get());
        
        for (auto& stmt : elemResult.hoistedStmts) {
            result.hoistedStmts.push_back(move(stmt));
        }
        
        newElements.push_back(move(elemResult.expr));
    }
    
    result.expr = make_unique<Tuple>(move(newElements));
    return result;
}

/* ============================================================
 * REWRITE SET
 * ============================================================ */

RewriteGlobalsVisitor::RewriteResult
RewriteGlobalsVisitor::rewriteSet(const Set* s) {
    RewriteResult result;
    vector<unique_ptr<Expr>> newElements;
    
    for (const auto& elem : s->elements) {
        auto elemResult = rewriteExpr(elem.get());
        
        for (auto& stmt : elemResult.hoistedStmts) {
            result.hoistedStmts.push_back(move(stmt));
        }
        
        newElements.push_back(move(elemResult.expr));
    }
    
    result.expr = make_unique<Set>(move(newElements));
    return result;
}

RewriteGlobalsVisitor::RewriteResult RewriteGlobalsVisitor::rewriteBoolConst(const BoolConst* bc) {
    RewriteResult result;
    result.expr = make_unique<BoolConst>(bc->value);
    return result;
}

RewriteGlobalsVisitor::RewriteResult RewriteGlobalsVisitor::rewriteBinaryOpExpr(const BinaryOpExpr* binop) {
    RewriteResult leftResult = rewriteExpr(binop->left.get());
    RewriteResult rightResult = rewriteExpr(binop->right.get());
    
    RewriteResult result;
    for (auto& stmt : leftResult.hoistedStmts) {
        result.hoistedStmts.push_back(std::move(stmt));
    }
    for (auto& stmt : rightResult.hoistedStmts) {
        result.hoistedStmts.push_back(std::move(stmt));
    }
    
    result.expr = make_unique<BinaryOpExpr>(
        binop->op,
        std::move(leftResult.expr),
        std::move(rightResult.expr)
    );
    
    return result;
}

RewriteGlobalsVisitor::RewriteResult RewriteGlobalsVisitor::rewriteUnaryOpExpr(const UnaryOpExpr* unop) {
    RewriteResult operandResult = rewriteExpr(unop->operand.get());
    
    RewriteResult result;
    for (auto& stmt : operandResult.hoistedStmts) {
        result.hoistedStmts.push_back(std::move(stmt));
    }
    
    result.expr = make_unique<UnaryOpExpr>(
        unop->op,
        std::move(operandResult.expr)
    );
    
    return result;
}

/* ============================================================
 * REWRITE MAP
 * ============================================================ */

RewriteGlobalsVisitor::RewriteResult
RewriteGlobalsVisitor::rewriteMap(const Map* m) {
    RewriteResult result;
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> newPairs;
    
    for (const auto& kv : m->value) {
        // Rewrite key (should be Var, but rewrite anyway)
        auto keyResult = rewriteExpr(kv.first.get());
        for (auto& stmt : keyResult.hoistedStmts) {
            result.hoistedStmts.push_back(move(stmt));
        }
        
        // Rewrite value
        auto valResult = rewriteExpr(kv.second.get());
        for (auto& stmt : valResult.hoistedStmts) {
            result.hoistedStmts.push_back(move(stmt));
        }
        
        // Key must be Var for Map AST
        Var* keyVar = dynamic_cast<Var*>(keyResult.expr.get());
        if (!keyVar) {
            throw runtime_error("Map key must be Var after rewrite");
        }
        
        unique_ptr<Var> keyOwned(static_cast<Var*>(keyResult.expr.release()));
        
        newPairs.push_back({
            move(keyOwned),
            move(valResult.expr)
        });
    }
    
    result.expr = make_unique<Map>(move(newPairs));
    return result;
}
