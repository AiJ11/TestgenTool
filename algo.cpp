#include "algo.hpp"
#include "clonevisitor.hh"

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <stdexcept>

using namespace std;

/* ============================================================
 * Helper logic
 * ============================================================ */

unique_ptr<Expr> convert1(
    const unique_ptr<Expr>& expr,
    SymbolTable* symtable,
    const string& suffix
) {
    if (!expr) return nullptr;

    // -------- Variable --------
    if (auto var = dynamic_cast<Var*>(expr.get())) {
        string name = var->name;
        if (symtable && symtable->hasKey(&name)) {
            return make_unique<Var>(name + suffix);
        }
        return make_unique<Var>(name);
    }

    // -------- Function Call --------
    if (auto func = dynamic_cast<FuncCall*>(expr.get())) {
        vector<unique_ptr<Expr>> args;
        for (const auto& arg : func->args) {
            args.push_back(convert1(arg, symtable, suffix));
        }
        return make_unique<FuncCall>(func->name, move(args));
    }

    // -------- Number --------
    if (auto num = dynamic_cast<Num*>(expr.get())) {
        return make_unique<Num>(num->value);
    }

    // -------- Set --------
    if (auto set = dynamic_cast<Set*>(expr.get())) {
        vector<unique_ptr<Expr>> elems;
        for (const auto& e : set->elements) {
            elems.push_back(convert1(e, symtable, suffix));
        }
        return make_unique<Set>(move(elems));
    }

    // -------- Tuple --------
    if (auto tup = dynamic_cast<Tuple*>(expr.get())) {
        vector<unique_ptr<Expr>> elems;
        for (const auto& e : tup->exprs) {
            elems.push_back(convert1(e, symtable, suffix));
        }
        return make_unique<Tuple>(move(elems));
    }

    return nullptr;
}

void addthedashexpr(
    const unique_ptr<Expr>& expr,
    set<string>& res
) {
    if (!expr) return;

    if (auto func = dynamic_cast<FuncCall*>(expr.get())) {
        if (func->name == "'") {
            if (auto v = dynamic_cast<Var*>(func->args[0].get())) {
                res.insert(v->name);
            }
            return;
        }
        for (const auto& arg : func->args) {
            addthedashexpr(arg, res);
        }
    }
}

unique_ptr<Expr> removethedashexpr(
    const unique_ptr<Expr>& expr,
    set<string>& res,
    int flag
) {
    if (!expr) return nullptr;

    if (auto var = dynamic_cast<Var*>(expr.get())) {
        if (flag) return make_unique<Var>(var->name);
        if (res.count(var->name)) {
            return make_unique<Var>(var->name + "_old");
        }
        return make_unique<Var>(var->name);
    }

    if (auto func = dynamic_cast<FuncCall*>(expr.get())) {
        if (func->name == "'") {
            return removethedashexpr(func->args[0], res, 1);
        }
        vector<unique_ptr<Expr>> args;
        for (const auto& arg : func->args) {
            args.push_back(removethedashexpr(arg, res));
        }
        return make_unique<FuncCall>(func->name, move(args));
    }

    return nullptr;
}

/* ============================================================
 * Input collection
 * ============================================================ */

unique_ptr<Stmt> makeInputStmt(unique_ptr<Var> var) {
    return make_unique<Assign>(
        move(var),
        make_unique<FuncCall>("input", vector<unique_ptr<Expr>>{})
    );
}

void getInputVars(
    const unique_ptr<Expr>& expr,
    vector<unique_ptr<Expr>>& inputVars,
    const string& suffix,
    SymbolTable* symtable,
    TypeMap& localTM
) {
    if (!expr) return;

    // -------- Variable --------
    if (auto v = dynamic_cast<Var*>(expr.get())) {
        string name = v->name;

        // Free variable → input
        if (!symtable || !symtable->hasKey(&name)) {
            string renamed = name + suffix;
            inputVars.push_back(make_unique<Var>(renamed));

            // (Optional for now) record in local type map
            // localTM.setValue(renamed, inferredType);
        }
        return;
    }

    // -------- Function Call --------
    if (auto f = dynamic_cast<FuncCall*>(expr.get())) {
        for (const auto& arg : f->args) {
            getInputVars(arg, inputVars, suffix, symtable, localTM);
        }
        return;
    }

    // -------- Set --------
    if (auto s = dynamic_cast<Set*>(expr.get())) {
        for (const auto& e : s->elements) {
            getInputVars(e, inputVars, suffix, symtable, localTM);
        }
        return;
    }

    // -------- Tuple --------
    if (auto t = dynamic_cast<Tuple*>(expr.get())) {
        for (const auto& e : t->exprs) {
            getInputVars(e, inputVars, suffix, symtable, localTM);
        }
        return;
    }

    // -------- Map --------
    if (auto m = dynamic_cast<Map*>(expr.get())) {

        for (const auto& kv : m->value) {
            // key is Var
            string keyName = kv.first->name;
            if (!symtable || !symtable->hasKey(&keyName)) {
                inputVars.push_back(
                    make_unique<Var>(keyName + suffix)
                );
            }

            // value expression
            getInputVars(kv.second, inputVars, suffix, symtable, localTM);
        }
        return;
    }
}


/* ============================================================
 * Global initialization
 * ============================================================ */

vector<unique_ptr<Stmt>> genInit(
    const Spec& spec,
    SymbolTable* globalST,
    TypeMap* globalTM
) {
    vector<unique_ptr<Stmt>> stmts;
    CloneVisitor cloner;

    for (const auto& init : spec.init) {

        // Allocate a stable key
        string* key = new string(init->varName);

        // Register global variable
        globalST->addMapping(key, nullptr);
        globalTM->setValue(*key, nullptr);

        // Emit: var := <init-expr>
        stmts.push_back(
            make_unique<Assign>(
                make_unique<Var>(*key),
                cloner.cloneExpr(init->expr.get())
            )
        );
    }
    return stmts;
}



/* ============================================================
 * ATC construction
 * ============================================================ */

Program buildATCFromBlockSequence(
    const vector<const API*>& blockSeq
) {
    vector<unique_ptr<Stmt>> stmts;
    SymbolTable globalST(nullptr);
    TypeMap globalTM(nullptr);

    // Global init
    // (caller guarantees spec init already handled if needed)

    for (size_t i = 0; i < blockSeq.size(); i++) {
        const API* api = blockSeq[i];
        string idx = to_string(i);

        SymbolTable blockST(&globalST);
        TypeMap blockTM(&globalTM);

        CloneVisitor cloner;

        auto pre  = api->pre
            ? cloner.cloneExpr(api->pre.get())
            : nullptr;

        auto post = api->response.ResponseExpr
            ? cloner.cloneExpr(api->response.ResponseExpr.get())
            : nullptr;

        auto call = cloner.cloneExpr(api->call->call.get());

        // -------- INPUTS --------
        vector<unique_ptr<Expr>> inputs;

        auto* callExpr = dynamic_cast<FuncCall*>(call.get());
        if (!callExpr) {
            throw runtime_error("API call must be FuncCall");
        }

        for (const auto& arg : callExpr->args) {
            getInputVars(arg, inputs, idx, &blockST, blockTM);
        }

        for (auto& in : inputs) {
            auto* v = dynamic_cast<Var*>(in.get());
            stmts.push_back(makeInputStmt(make_unique<Var>(v->name)));
        }

        // -------- RENAME --------
        auto pre1  = pre  ? convert1(pre,  &blockST, idx) : nullptr;
        auto call1 = convert1(call, &blockST, idx);
        auto post1 = post ? convert1(post, &blockST, idx) : nullptr;

        // -------- PRIMED VARS --------
        set<string> primed;
        if (post1) addthedashexpr(post1, primed);

        // -------- SNAPSHOT --------
        for (const auto& v : primed) {
            stmts.push_back(
                make_unique<Assign>(
                    make_unique<Var>(v + "_old"),
                    make_unique<Var>(v)
                )
            );
        }

        // -------- ASSUME --------
        if (pre1) {
            stmts.push_back(make_unique<Assume>(move(pre1)));
        }

        // -------- CALL --------
        stmts.push_back(
            make_unique<Assign>(
                make_unique<Var>("_"),
                move(call1)
            )
        );

        // -------- ASSERT --------
        if (post1) {
            post1 = removethedashexpr(post1, primed);
            stmts.push_back(make_unique<Assert>(move(post1)));
        }
    }

    return Program(move(stmts));
}

/* ============================================================
 * genATC
 * ============================================================ */

Program genATC(
    const Spec& spec,
    const vector<string>& testString
) {
    vector<const API*> blocks;

    for (const auto& name : testString) {
        bool found = false;
        for (const auto& b : spec.blocks) {
            if (b->name == name) {
                blocks.push_back(b.get());
                found = true;
                break;
            }
        }
        if (!found) {
            throw runtime_error("Block not found: " + name);
        }
    }

    SymbolTable globalST(nullptr);
    TypeMap globalTM(nullptr);

    vector<unique_ptr<Stmt>> stmts;

    //  1. Global init first
    auto initStmts = genInit(spec, &globalST, &globalTM);
    for (auto& s : initStmts) {
        stmts.push_back(move(s));
    }

    //  2. API blocks
    Program body = buildATCFromBlockSequence(blocks);
    for (auto& s : body.statements) {
        stmts.push_back(move(const_cast<unique_ptr<Stmt>&>(s)));
    }

    return Program(move(stmts));
}
