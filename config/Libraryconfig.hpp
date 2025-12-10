#pragma once
#include "../WebAppConfig.hpp"
#include "../ModularTestGen.hpp"

// Forward declarations
#include "../library_apispec_examples/heavy_examples/1.hpp"
#include "../library_apispec_examples/heavy_examples/2.hpp"
#include "../library_apispec_examples/heavy_examples/3.hpp"
#include "../library_apispec_examples/heavy_examples/4.hpp"
#include "../library_apispec_examples/heavy_examples/5.hpp"

/**
 * Library Management Application Configuration
 * 
 * Global Variables:
 * - U: (string -> string) map  // Users: userId -> password
 * - S: (string -> tuple) map   // Students: studentId -> (name, data)
 * - T: (string -> string) map  // Tokens: token -> userId
 * - B: (string -> Book) map    // Books: bookCode -> book data
 * - R: set of requests (implicit)
 * - BS: set of book-student records (implicit)
 */
class LibraryAppConfig {
public:
    static WebAppConfig create() {
        return WebAppConfigBuilder("library")
            .withDescription("Library management system with student and book operations")
            
            // Core globals
            .addMapGlobal("U", "string", "string")  // userId -> password
            .addMapGlobal("S", "string", "tuple")   // studentId -> student data
            .addMapGlobal("T", "string", "string")  // token -> userId
            .addMapGlobal("B", "string", "book")    // bookCode -> book
            
            // Initialize as empty
            .initializeGlobal("U", "{}")
            .initializeGlobal("S", "{}")
            .initializeGlobal("T", "{}")
            .initializeGlobal("B", "{}")
            
            // Functions
            .addFunction("register_admin", {"string", "string"}, HTTPResponseCode::CREATED_201)
            .addFunction("login", {"string", "string"}, HTTPResponseCode::CREATED_201, {"string"})
            .addFunction("getStudent", {"string", "string"}, HTTPResponseCode::OK_200, {"student"})
            .addFunction("getAllStudents", {"string"}, HTTPResponseCode::OK_200, {"list"})
            .addFunction("deleteStudent", {"string", "string"}, HTTPResponseCode::OK_200)
            .addFunction("saveStudent", {"string", "student"}, HTTPResponseCode::OK_200)
            .addFunction("updateStudent", {"string", "student"}, HTTPResponseCode::OK_200)
            .addFunction("getAllBooks", {"string"}, HTTPResponseCode::OK_200, {"list"})
            .addFunction("getBook", {"string", "string"}, HTTPResponseCode::OK_200, {"book"})
            .addFunction("deleteBook", {"string", "string"}, HTTPResponseCode::OK_200)
            .addFunction("saveBook", {"string", "book"}, HTTPResponseCode::OK_200)
            
            // Test API (getters/setters for global state)
            .addTestAPI("U", "get_U", "set_U")
            .addTestAPI("S", "get_S", "set_S")
            .addTestAPI("T", "get_T", "set_T")
            .addTestAPI("B", "get_B", "set_B")
            
            // Test Scenarios
            .addTestScenario(
                "library_scenario_1",
                "Get Student by ID",
                {"getStudent"},
                "heavyexample1"
            )
            .addTestScenario(
                "library_scenario_2",
                "Get All Students",
                {"getAllStudents"},
                "heavyexample2"
            )
            .addTestScenario(
                "library_scenario_3",
                "Delete Student",
                {"deleteStudent"},
                "heavyexample3"
            )
            .addTestScenario(
                "library_scenario_4",
                "Save Student Details",
                {"saveStudent"},
                "heavyexample4"
            )
            .addTestScenario(
                "library_scenario_5",
                "Update Student",
                {"updateStudent"},
                "heavyexample5"
            )
            .build();
    }
};

// Register all library builders
REGISTER_API_BUILDER(heavyexample1, heavyexample1);
REGISTER_API_BUILDER(heavyexample2, heavyexample2);
REGISTER_API_BUILDER(heavyexample3, heavyexample3);
REGISTER_API_BUILDER(heavyexample4, heavyexample4);
REGISTER_API_BUILDER(heavyexample5, heavyexample5);
