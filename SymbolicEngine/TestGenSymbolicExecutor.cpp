// TestGenSymbolicExecutor.cpp
#include "TestGenSymbolicExecutor.hpp"
#include <iostream>
#include <stdexcept>


// The executor will rely on env to hold/read bindings so rewriteATC / genCTC later can read them.

using namespace std;

static unique_ptr<Expr> cloneExprOrNull(const unique_ptr<Expr> &e) {
    if (!e) return nullptr;
    return e->clone();
}

// -------------------- isAPI helper --------------------
bool TestGenSymbolicExecutor::isAPI(const std::string &fname) const {
    // If fname is not one of nativeHelpers, treat it as API for now.
    // You can change this to consult Spec/function table later.
    if (nativeHelpers.find(fname) != nativeHelpers.end()) return false;
    // common assume/assert strings handled elsewhere
    if (fname == "assume" || fname == "assert") return false;
    return true;
}

// -------------------- isSymbolicExpr --------------------
bool TestGenSymbolicExecutor::isSymbolicExpr(const Expr &e, const TestGenSymbolicEnv &env) const {
    switch (e.expressionType) {
        case ExpressionType::NUM:
        case ExpressionType::STRING:
            return false;
        case ExpressionType::VAR: {
    const Var &v = static_cast<const Var&>(e);
    // If env has a binding (either concrete or symbolic), inspect it
    if (env.hasExpr(v.name)) {
        auto b = env.getExprClone(v.name);
        if (!b) return true; // treat unknown binding as symbolic
        // If bound to a NUM/STRING it's concrete -> not symbolic
        return !(b->expressionType == ExpressionType::NUM ||
                 b->expressionType == ExpressionType::STRING);
    }
    // No binding -> symbolic (input/unbound)
    return true;
}


        case ExpressionType::SET: {
            const Set &s = static_cast<const Set&>(e);
            for (auto &elem : s.elements) if (isSymbolicExpr(*elem, env)) return true;
            return false;
        }
        case ExpressionType::TUPLE: {
            const Tuple &t = static_cast<const Tuple&>(e);
            for (auto &elem : t.expr) if (isSymbolicExpr(*elem, env)) return true;
            return false;
        }
        case ExpressionType::FUNCTIONCALL_EXPR: {
            // if any arg is symbolic => function call is symbolic (unless helper simplifies)
            const FuncCall &fc = static_cast<const FuncCall&>(e);
            // native helpers assumed to be evaluable symbolically (they accept symbolic args).
            if (!isAPI(fc.name)) {
                // still symbolic if args symbolic? many helpers operate symbolically,
                // but we treat helper calls as non-symbolic results only if all args are non-symbolic.
                for (auto &a : fc.args) if (isSymbolicExpr(*a, env)) return true;
                return false;
            } else {
                // API function => result is symbolic unless its args produce concrete result.
                for (auto &a : fc.args) if (!isSymbolicExpr(*a, env)) return false; // if any arg concrete return false? (slide says API requires concrete args -> see isReady)
                // conservative: treat API result as symbolic
                return true;
            }
        }
        default:
            return true;
    }
}

// -------------------- isReadyExpr --------------------
bool TestGenSymbolicExecutor::isReadyExpr(const Expr &e, const TestGenSymbolicEnv &env) const {
    switch (e.expressionType) {
        case ExpressionType::NUM:
        case ExpressionType::STRING:
            return true;

        case ExpressionType::VAR: {
            const Var &v = static_cast<const Var&>(e);
            // ready only if env has a concrete (non-symbolic) binding
            if (!env.hasExpr(v.name)) return false; // unbound input var => not ready
            auto bound = env.getExprClone(v.name);
            if (!bound) return false;
            return (bound->expressionType == ExpressionType::NUM || bound->expressionType == ExpressionType::STRING);
        }

        case ExpressionType::SET: {
            const Set &s = static_cast<const Set&>(e);
            for (auto &elem : s.elements) if (!isReadyExpr(*elem, env)) return false;
            return true;
        }

        case ExpressionType::TUPLE: {
            const Tuple &t = static_cast<const Tuple&>(e);
            for (auto &elem : t.expr) if (!isReadyExpr(*elem, env)) return false;
            return true;
        }

        case ExpressionType::FUNCTIONCALL_EXPR: {
            const FuncCall &fc = static_cast<const FuncCall&>(e);
            if (isAPI(fc.name)) {
        // SEMANTICS: API call is ready ONLY IF ALL arguments are ready.
        for (auto &arg : fc.args) {
            if (!isReadyExpr(*arg, env))
                return false;
        }
        return true;
            } else {
                // helper - assume always ready (we can evaluate symbolically)
                return true;
            }
        }

        default:
            return true;
    }
}

// -------------------- isReadyStmt --------------------
bool TestGenSymbolicExecutor::isReadyStmt(const Stmt &s, const TestGenSymbolicEnv &env) const {
    switch (s.statementType) {
        case StatementType::ASSIGN: {
            const Assign &a = static_cast<const Assign&>(s);
            return isReadyExpr(*a.right, env);
        }
        case StatementType::ASSUME: {
            const AssumeStmt &as = static_cast<const AssumeStmt&>(s);
            return isReadyExpr(*as.condition, env);
        }
        case StatementType::ASSERT: {
            const AssertStmt &as = static_cast<const AssertStmt&>(s);
            return isReadyExpr(*as.condition, env);
        }
        case StatementType::FUNCTIONCALL_STMT: {
            const FuncCallStmt &fcs = static_cast<const FuncCallStmt&>(s);
            if (!fcs.call) return true;
            const FuncCall &fc = *fcs.call;
            if (isAPI(fc.name)) {
                // ready if ANY argument ready (as per slide)
                for (auto &arg : fc.args) if (isReadyExpr(*arg, env)) return true;
                return false;
            } else {
                // helper call - ready
                return true;
            }
        }
        default:
            return true;
    }
}

// -------------------- evaluateExpr (SYMEVAL) --------------------
std::unique_ptr<Expr> TestGenSymbolicExecutor::evaluateExpr(const Expr &e, TestGenSymbolicEnv &env) {
    switch (e.expressionType) {
        case ExpressionType::NUM:
        case ExpressionType::STRING:
            return e.clone();

        case ExpressionType::VAR: {
            const Var &v = static_cast<const Var&>(e);
            if (env.hasExpr(v.name)) {
                // return cloned binding (could be concrete or symbolic expression)
                auto b = env.getExprClone(v.name);
                if (b) return b;
            }
            // not bound => create fresh symbolic variable (Var with fresh name) and bind it
            string sym = env.createFreshSymbol("in_" + v.name);
            auto newVar = make_unique<Var>(sym);
            env.bindExpr(v.name, newVar->clone()); // bind original variable name -> the symbolic var expr
            return newVar;
        }

        case ExpressionType::SET: {
            const Set &s = static_cast<const Set&>(e);
            vector<unique_ptr<Expr>> elems;
            elems.reserve(s.elements.size());
            for (auto &el : s.elements) {
                elems.push_back(evaluateExpr(*el, env));
            }
            return make_unique<Set>(std::move(elems));
        }

        case ExpressionType::TUPLE: {
            const Tuple &t = static_cast<const Tuple&>(e);
            vector<unique_ptr<Expr>> elems;
            elems.reserve(t.expr.size());
            for (auto &el : t.expr) {
                elems.push_back(evaluateExpr(*el, env));
            }
            return make_unique<Tuple>(std::move(elems));
        }

        case ExpressionType::FUNCTIONCALL_EXPR: {
            const FuncCall &fc = static_cast<const FuncCall&>(e);

            // Evaluate args (SYMEVAL recursively)
            vector<unique_ptr<Expr>> evaluatedArgs;
            evaluatedArgs.reserve(fc.args.size());
            for (auto &a : fc.args) evaluatedArgs.push_back(evaluateExpr(*a, env));

            if (!isAPI(fc.name)) {
                // Native helper: form a FuncCall node with evaluated args and return it
                return make_unique<FuncCall>(fc.name, std::move(evaluatedArgs));
            } else {
                // API function: we cannot fully execute it here (external). But if any arg is concrete
                // then we may produce a partially-evaluated call. To follow slide: return FuncCall with evaluated args.
                // If you want to convert to concrete result when all args concrete, you can add logic here.
                return make_unique<FuncCall>(fc.name, std::move(evaluatedArgs));
            }
        }

        default:
            // conservative fallback
            return e.clone();
    }
}

// -------------------- Statement handlers --------------------
void TestGenSymbolicExecutor::handleAssign(Assign &asg, TestGenSymbolicEnv &env, vector<unique_ptr<Expr>> &pathConstraints) {
    // left is Var
    if (!asg.left) return;
    auto rhsEval = evaluateExpr(*asg.right, env);
    // bind left var name to rhsEval clone in env
    env.bindExpr(asg.left->name, rhsEval->clone());
}

void TestGenSymbolicExecutor::handleAssume(AssumeStmt &assume, TestGenSymbolicEnv &env, vector<unique_ptr<Expr>> &pathConstraints) {
    // Evaluate condition and add to path constraints
    auto cond = evaluateExpr(*assume.condition, env);
    pathConstraints.push_back(std::move(cond));
    // Optionally also store the assume as binding or constraint string in env (env.addConstraint)
    // If env supports SMT constraints as strings, you could convert cond->toString() and add.
}

void TestGenSymbolicExecutor::handleAssert(AssertStmt &asserts, TestGenSymbolicEnv &env, vector<unique_ptr<Expr>> &pathConstraints) {
    auto cond = evaluateExpr(*asserts.condition, env);
    pathConstraints.push_back(std::move(cond));
    // assertions can be treated same as assume for path constraint collection; driver may treat them specially.
}

void TestGenSymbolicExecutor::handleFuncCallStmt(FuncCallStmt &fcs, TestGenSymbolicEnv &env, vector<unique_ptr<Expr>> &pathConstraints) {
    if (!fcs.call) return;
    // If it's an assume/assert encoded as function call (sometimes parser produces assume(...) as FuncCall),
    // handle specially:
    if (fcs.call->name == "assume" && !fcs.call->args.empty()) {
        auto cond = evaluateExpr(*fcs.call->args[0], env);
        pathConstraints.push_back(std::move(cond));
        return;
    }
    if (fcs.call->name == "assert" && !fcs.call->args.empty()) {
        auto cond = evaluateExpr(*fcs.call->args[0], env);
        pathConstraints.push_back(std::move(cond));
        return;
    }

    // For other function calls:
    // - If helper (native), we evaluate args and optionally compute a result to throw into some target variable (if function returns assigned to a variable).
    // - If API call, we evaluate args and leave the function call expression as result (symbolic).
    vector<unique_ptr<Expr>> evalArgs;
    for (auto &a : fcs.call->args) evalArgs.push_back(evaluateExpr(*a, env));

    // No direct side-effect binding here (unless API returns mapped to variable via assignment in AST).
    // If you support FunctionFactory to actually "execute" some functions (e.g., dom, inMap) you can call that here.
}

// -------------------- executePartial (SYMEX main loop) --------------------
vector<unique_ptr<Expr>> TestGenSymbolicExecutor::executePartial(Program &prog, TestGenSymbolicEnv &env) {
    vector<unique_ptr<Expr>> C; // path constraints
    // Walk statements sequentially and stop when encountering the first NOT-ready statement (per slide)
    for (auto &stmtPtr : prog.statements) {
        if (!stmtPtr) continue;
        Stmt &s = *stmtPtr;
        if (isReadyStmt(s, env)) {
            // execute symbolically
            switch (s.statementType) {
                case StatementType::ASSIGN:
                    handleAssign(static_cast<Assign&>(s), env, C);
                    break;
                case StatementType::ASSUME:
                    handleAssume(static_cast<AssumeStmt&>(s), env, C);
                    break;
                case StatementType::ASSERT:
                    handleAssert(static_cast<AssertStmt&>(s), env, C);
                    break;
                case StatementType::FUNCTIONCALL_STMT:
                    handleFuncCallStmt(static_cast<FuncCallStmt&>(s), env, C);
                    break;
                default:
                    break;
            }
            // continue to next statement
        } else {
            // statement not ready: break out (SYMEX stops here and returns collected constraints)
            break;
        }
    }
    return C;
}
