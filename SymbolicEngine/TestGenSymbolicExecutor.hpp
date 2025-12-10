// TestGenSymbolicExecutor.hpp
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include "TestGenSymbolicEnv.hpp"

class TestGenSymbolicExecutor {
public:
    TestGenSymbolicExecutor() = default;

    // Execute program until a "not ready" statement is encountered.
    // Returns a vector of path constraint expressions (each a unique_ptr clone).
    // The executor will store variable bindings into env (via env.bindExpr / env.bindSymbolic).
    std::vector<std::unique_ptr<Expr>> executePartial(Program &prog, TestGenSymbolicEnv &env);

    // Small helpers (public for tests/debug)
    bool isReadyStmt(const Stmt &s, const TestGenSymbolicEnv &env) const;
    bool isReadyExpr(const Expr &e, const TestGenSymbolicEnv &env) const;
    bool isSymbolicExpr(const Expr &e, const TestGenSymbolicEnv &env) const;

    // Evaluate an expression in the current env. Returns a clone of resulting Expr.
    // If expression depends on unknown input, it will create a fresh symbolic Var via env.createFreshSymbol()
    // and return that Var.
    std::unique_ptr<Expr> evaluateExpr(const Expr &e, TestGenSymbolicEnv &env);

private:
    // statement handlers
    void handleAssign(Assign &asg, TestGenSymbolicEnv &env, std::vector<std::unique_ptr<Expr>> &pathConstraints);
    void handleFuncCallStmt(FuncCallStmt &fcs, TestGenSymbolicEnv &env, std::vector<std::unique_ptr<Expr>> &pathConstraints);
    void handleAssume(AssumeStmt &assume, TestGenSymbolicEnv &env, std::vector<std::unique_ptr<Expr>> &pathConstraints);
    void handleAssert(AssertStmt &asserts, TestGenSymbolicEnv &env, std::vector<std::unique_ptr<Expr>> &pathConstraints);

    // helper to decide if a function name denotes an API (as per slide)
    bool isAPI(const std::string &fname) const;

    // helper: set of known native helper functions (update as needed)
    std::unordered_set<std::string> nativeHelpers = {
        "equals", "inMap", "mapAccess", "dom", "getId", "getRoles", "contains",
        "AND", "OR", "NOT", "len", "str.len"
    };
};
