// TestGenSymbolicVisitor.hpp
#pragma once
#include "../ASTVis.hpp"
#include "../ast.hpp"
#include "TestGenSymbolicEnv.hpp"
#include "../APIFunction.hpp"
#include <string>
#include <vector>
#include <unordered_set>  

/*
 * Symbolic visitor for TestGen output
 * Handles TestGen function names: AND, OR, inMapVerify, equals, etc.
 */
class TestGenSymbolicVisitor : public ASTVisitor {
public:
    explicit TestGenSymbolicVisitor(TestGenSymbolicEnv& env, APIFunctionFactory* factory = nullptr, bool debugMode = false, bool executeAPIs = false)
        : env(env), functionFactory(factory), debugMode(debugMode), executeAPIs(executeAPIs) {}

    // Main statement visitors
    void visit(const Assign& n)       override;
    void visit(const FuncCallStmt& n) override;
    void visit(const Program& n)      override;
    
    // To handle new statement types 
    void visit(const AssumeStmt &s) override;
    void visit(const AssertStmt &s) override;
    void visit(const InputStmt &s) override;

    // Expression visitors (used by smtOf function)
    void visit(const Var& n)      override;
    void visit(const FuncCall& n) override;
    void visit(const Num& n)      override;
    void visit(const String& n)   override;

    // Unused visitors (stubbed out)
    void visit(const Set&)        override {}
    void visit(const Map&)        override {}
    void visit(const Tuple&)      override {}
    void visit(const TypeConst&)  override {}
    void visit(const FuncType&)   override {}
    void visit(const MapType&)    override {}
    void visit(const TupleType&)  override {}
    void visit(const SetType&)    override {}
    void visit(const Decl&)       override {}
    void visit(const FuncDecl&)   override {}
    void visit(const APIcall&)    override {}
    void visit(const API&)        override {}
    void visit(const Response&)   override {}
    void visit(const Init&)       override {}
    void visit(const Spec&)       override {}

    void setDebug(bool debug) { debugMode = debug; }

    void setFunctionFactory(APIFunctionFactory* factory) {
        functionFactory = factory; 
    }

    // Convert TestGen expression to SMT-LIB string
    std::string exprToSMT(const Expr& expr);

    //  isReady checks 
    bool isReadyStmt(const Stmt& s) const;
    bool isReadyExpr(const Expr& e) const;
    bool isSymbolicExpr(const Expr& e) const;

    //some more METHODS
    bool isConcreteExpr(const Expr& e) const;
    void executeAPI(
        const std::string& apiName, 
        const std::vector<std::unique_ptr<Expr>>& argExprs,
        const std::vector<std::unique_ptr<Expr>>& originalArgs//oroginal arguments before any binding
    );

private:
    TestGenSymbolicEnv& env;
    APIFunctionFactory* functionFactory; 
    bool debugMode = false;
    bool executeAPIs = false;
    bool stopExecution = false; //flag to stop further execution
    
    // Helper for recursive SMT conversion
    std::string convertExprToSMT(const Expr& expr);

    //Helper to check if function is API (external) or native (helper)
    bool isAPI(const std::string& fname) const;
    // cloneExpr helper
    std::unique_ptr<Expr> cloneExpr(const Expr* expr) const;

    void updateStateAfterAPI(
        const std::string& apiName,
        //const std::vector<std::unique_ptr<Expr>>& argExprs,
        const std::vector<std::unique_ptr<Expr>>& originalArgs,
        int statusCode
    );
    
    //Native helper functions (evaluated symbolically, always ready)
    const std::unordered_set<std::string> nativeHelpers = {
        "AND", "OR", "NOT", "=", "equals", 
        "in", "inMapVerify", "mapAccess", "[]",
        "dom", "getId", "getRoles", "contains",
        "add_to_set", "not_empty", "len",
        "assume", "assert", "input"
    };
};
