#include <iostream>
#include <cassert>
#include <memory>
#include <fstream>
#include <string>
#include <vector>

#include "jsCodeGenerator/jsCodeGen.h"
#include "ast.hpp"
#include "algo.hpp"
#include "symbol_table.hpp"
#include "WebAppConfig.hpp"
#include "ModularTestGen.hpp"

// Include all app configurations
#include "config/Tourismconfig.hpp"
#include "config/Pesufoodsconfig.hpp"
#include "config/Libraryconfig.hpp"
#include "config/Restaurantconfig.hpp"

// Conditional include for symbolic engine
#ifdef USE_SYMBOLIC_ENGINE
#include "SymbolicEngine/TestGenDriver.hpp"
#endif

using namespace std;

/**
 * Command-line interface for running tests
 */
class TestGenCLI {
public:
    static void printUsage(const char* programName) {
        cout << "Usage: " << programName << " [options]\n\n";
        cout << "Options:\n";
        cout << "  --list-apps                List all available web applications\n";
        cout << "  --list-scenarios <app>     List all test scenarios for an app\n";
        cout << "  --app <name>               Select web application to test\n";
        cout << "  --scenario <id>            Select specific test scenario (optional)\n";
        cout << "  --all-scenarios            Run all scenarios for the selected app\n";
        cout << "  --symbolic                 Run symbolic execution (requires symbolic engine)\n";
        cout << "  --execute                  Execute API calls during symbolic execution\n";
        cout << "  --help                     Display this help message\n\n";
        cout << "Examples:\n";
        cout << "  " << programName << " --list-apps\n";
        cout << "  " << programName << " --app tourism --scenario tourism_scenario_1\n";
        cout << "  " << programName << " --app pesu_foods --all-scenarios --symbolic\n";
        cout << "  " << programName << " --app library --scenario library_scenario_2\n";
    }
    
    static WebAppRegistry createRegistry() {
        WebAppRegistry registry;
        
        // Register all applications
        registry.registerApp(TourismAppConfig::create());
        registry.registerApp(PesuFoodsAppConfig::create());
        registry.registerApp(LibraryAppConfig::create());
        registry.registerApp(RestaurantAppConfig::create());
        
        return registry;
    }
    
    static void listApps(const WebAppRegistry& registry) {
        cout << "\nAvailable Web Applications: \n";
        auto apps = registry.listApps();
        for (const auto& app : apps) {
            auto* config = registry.getConfig(app);
            cout << "  - " << app;
            if (config && !config->description.empty()) {
                cout << ": " << config->description;
            }
            cout << "\n";
        }
        cout << "\n";
    }
    
    static void listScenarios(const WebAppRegistry& registry, const string& appName) {
        auto* config = registry.getConfig(appName);
        if (!config) {
            cerr << "Error: Application '" << appName << "' not found\n";
            return;
        }
        
        cout << "\nTest Scenarios for " << appName << " \n";
        for (const auto& scenario : config->testScenarios) {
            cout << "  - " << scenario.id << ": " << scenario.description << "\n";
            cout << "    APIs: ";
            for (size_t i = 0; i < scenario.apiSequence.size(); ++i) {
                if (i > 0) cout << " -> ";
                cout << scenario.apiSequence[i];
            }
            cout << "\n\n";
        }
    }
};

/**
 * Main test runner
 */
void runTest(const WebAppRegistry& registry, 
             const string& appName, 
             const string& scenarioId,
             bool runSymbolic,
             bool executeAPIs) {
    
    cout << "\n Running Test for " << appName;
    if (!scenarioId.empty()) {
        cout << " (scenario: " << scenarioId << ")";
    }
    cout << " \n\n";
    
    // Get configuration
    auto* config = registry.getConfig(appName);
    if (!config) {
        cerr << "Error: Application '" << appName << "' not found\n";
        return;
    }
    
    // Generate specification and symbol table
    auto [spec, symtable] = ModularTestGen::generateSpec(*config, scenarioId);
    
    // Print specification
    PrintVisitor visitor;
    cout << "= SPECIFICATION =" << endl;
    spec->accept(visitor);
    
    // Generate TestGen program
    Program testGenProgram = convert(spec.get(), symtable);
    cout << "\n TESTGEN PROGRAM! " << endl;
    cout << "Generated program with " << testGenProgram.statements.size() << " statements" << endl;
    testGenProgram.accept(visitor);
    
#ifdef USE_SYMBOLIC_ENGINE
    if (runSymbolic) {
        // Run symbolic execution
        cout << "\n=== TESTGEN SYMBOLIC EXECUTION ===" << endl;
        
        auto result = TestGenDriver::generateConcreteTestCase(testGenProgram, *config, true, executeAPIs);
        
        if (result.satisfiable) {
            cout << "\n✓ SUCCESS: Generated concrete test case!" << endl;
            
            cout << "\n=== CONCRETE TEST INPUTS ===" << endl;
            for (const auto& [varName, value] : result.concreteValues) {
                if (!value.empty()) {
                    cout << "  " << varName << " = \"" << value << "\"" << endl;
                }
            }
            
            // Write results to files
            ofstream smtFile("testgen_output_" + appName + "_" + scenarioId + ".smt2");
            smtFile << result.smtContent;
            smtFile.close();
            
            ofstream resultsFile("concrete_test_values_" + appName + "_" + scenarioId + ".txt");
            resultsFile << "Concrete Test Case for " << appName << " - " << scenarioId << "\n";
            resultsFile << "\n\n";
            for (const auto& [varName, value] : result.concreteValues) {
                if (!value.empty()) {
                    resultsFile << varName << " = " << value << "\n";
                }
            }
            resultsFile.close();
            
            cout << "\n=== FILES WRITTEN ===" << endl;
            cout << "- testgen_output_" << appName << "_" << scenarioId << ".smt2" << endl;
            cout << "- concrete_test_values_" << appName << "_" << scenarioId << ".txt" << endl;
            
        } else {
            cout << "\n✗ FAILED: Could not generate concrete test case" << endl;
            cout << "Error: " << result.errorMessage << endl;
        }
    }
#else
    if (runSymbolic) {
        cout << "\n=== SYMBOLIC ENGINE NOT AVAILABLE ===" << endl;
        cout << "To enable symbolic execution:" << endl;
        cout << "  make symbolic" << endl;
    }
#endif
    
    // Generate JavaScript code (always available)
    cout << "\n=== JAVASCRIPT GENERATION ===" << endl;
    ExpoSECodeGenerator ecg;
    string code = ecg.generateCode(testGenProgram);
    
    // Write to file
    ofstream jsFile("testgen_output_" + appName + "_" + scenarioId + ".js");
    jsFile << code;
    jsFile.close();
    
    cout << "JavaScript code written to: testgen_output_" << appName << "_" << scenarioId << ".js\n";
    
    // Clean up symbol table
    for (auto* child : symtable.children) {
        delete child;
    }
}

int main(int argc, char** argv) {
    try {
        // Create registry
        auto registry = TestGenCLI::createRegistry();
        
        // Parse command line arguments
        if (argc == 1) {
            TestGenCLI::printUsage(argv[0]);
            return 0;
        }
        
        string appName;
        string scenarioId;
        bool runSymbolic = false;
        bool allScenarios = false;
        bool executeAPIs = false;
        
        for (int i = 1; i < argc; ++i) {
            string arg = argv[i];
            
            if (arg == "--help") {
                TestGenCLI::printUsage(argv[0]);
                return 0;
            } else if (arg == "--list-apps") {
                TestGenCLI::listApps(registry);
                return 0;
            } else if (arg == "--list-scenarios") {
                if (i + 1 < argc) {
                    TestGenCLI::listScenarios(registry, argv[++i]);
                    return 0;
                } else {
                    cerr << "Error: --list-scenarios requires an app name\n";
                    return 1;
                }
            } else if (arg == "--app") {
                if (i + 1 < argc) {
                    appName = argv[++i];
                } else {
                    cerr << "Error: --app requires an application name\n";
                    return 1;
                }
            } else if (arg == "--scenario") {
                if (i + 1 < argc) {
                    scenarioId = argv[++i];
                } else {
                    cerr << "Error: --scenario requires a scenario ID\n";
                    return 1;
                }
            } else if (arg == "--all-scenarios") {
                allScenarios = true;
            } else if (arg == "--symbolic") {
                runSymbolic = true;
            } else if (arg == "--execute") {
                executeAPIs = true;
            } else {
                cerr << "Error: Unknown option: " << arg << "\n";
                TestGenCLI::printUsage(argv[0]);
                return 1;
            }
        }
        
        // Run tests
        if (appName.empty()) {
            cerr << "Error: No application specified (use --app)\n";
            TestGenCLI::printUsage(argv[0]);
            return 1;
        }
        
        auto* config = registry.getConfig(appName);
        if (!config) {
            cerr << "Error: Application '" << appName << "' not found\n";
            cerr << "Use --list-apps to see available applications\n";
            return 1;
        }
        
        if (allScenarios) {
            // Run all scenarios
            for (const auto& scenario : config->testScenarios) {
                runTest(registry, appName, scenario.id, runSymbolic, executeAPIs);
                cout << "\n" << string(80, '=') << "\n";
            }
        } else if (!scenarioId.empty()) {
            // Run specific scenario
            runTest(registry, appName, scenarioId, runSymbolic, executeAPIs);
        } else {
            cerr << "Error: No scenario specified (use --scenario or --all-scenarios)\n";
            return 1;
        }
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
