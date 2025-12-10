
// TestGenDriver.hpp

#pragma once

#include <string>

#include <map>

#include "TestGenSymbolicVisitor.hpp"

#include "TestGenSymbolicEnv.hpp"

#include "../ast.hpp"

// Forward declarations

class Spec;

struct SymbolTable;

struct TypeMap;
struct WebAppConfig;

class TestGenDriver {

public:

    struct TestResult {

        bool satisfiable;

        std::map<std::string, std::string> concreteValues;

        std::string smtContent;

        std::string z3Output;

        std::string errorMessage;

    };

    

    static TestResult generateConcreteTestCase(const Program& testGenProgram, const WebAppConfig& config, bool debugMode = false, bool executeAPIs = false);

    static void demonstrateTestGenSymbolicEngine();

private:

    static void writeFile(const std::string& filename, const std::string& content);

    static std::string runZ3(const std::string& smtFile);

    static std::map<std::string, std::string> parseZ3Model(const std::string& z3Output, const TestGenSymbolicEnv& env);

    // inside class TestGenDriver { ... private: section

    // rewriteATC: replace InputStmt placeholders with concrete values from L (consumes from L front).
    // Returns a new Program on success; on error returns nullptr and sets error flag.
    static std::unique_ptr<Program> rewriteATC(const Program &t,
                                               std::vector<std::string> &L,
                                               bool &error);

    // genCTC: iterative driver that rewrites ATC, runs symex to collect constraints, calls Z3 (runZ3)
    // and produces concrete test case (Program). If solver returns unsat or error, returns current program or nullptr.
    static std::unique_ptr<Program> genCTC(std::unique_ptr<Program> t, const WebAppConfig& config,
                                           std::vector<std::string> initialL = {},
                                           bool debugMode = false, bool executeAPIs = false);


};

