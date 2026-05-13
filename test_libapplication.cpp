#include <iostream>
#include <memory>
#include <map>
#include <functional>
#include "ast.hh"
#include "printvisitor.hh"
#include "algo.hpp"
#include "tester/tester.hh"
#include "env.hh"
#include "see/restaurantfunctionfactory.hh"
#include "see/ecommercefunctionfactory.hh"
#include "see/libraryfunctionfactory.hh"
#include "see/tripvaultfunctionfactory.hh"
#include "see/ghostsocketfunctionfactory.hh"
#include "see/serveezfunctionfactory.hh"
#include "see/see.hh"

// Import webapp-specific specs
#include "specs/RestaurantSpec.hpp"
#include "specs/EcommerceSpec.hpp"
#include "specs/LibrarySpec.hpp"
#include "specs/TripVaultSpec.hpp"
#include "specs/GhostSocketSpec.hpp"
#include "specs/ServeezSpec.hpp"

using namespace std;

// ============================================
// TEST EXECUTION MODES
// ============================================

enum class TestMode
{
    ORIGINAL,     // Just genATC (no rewrite)
    REWRITE_ONLY, // genATC + RewriteGlobalsVisitor (no backend)
    FULL_PIPELINE // Complete: genATC + Rewrite + SEE + Backend
};

// ============================================
// TEST EXECUTOR LIBRARY
// ============================================

class LibraryTestExecutor
{
private:
    TestMode mode;
    string backendUrl;

public:
    LibraryTestExecutor(TestMode m, const string &url = "http://localhost:8080")
        : mode(m), backendUrl(url) {}

    void runTest(
        const string &testName,
        unique_ptr<Spec> spec,
        const vector<string> &testSequence)
    {
        cout << "\n========================================" << endl;
        cout << "TEST: " << testName << endl;
        cout << "MODE: " << getModeString() << endl;
        cout << "DEPTH: " << testSequence.size() << " API calls" << endl;
        cout << "========================================\n"
             << endl;

        try
        {
            switch (mode)
            {
            case TestMode::ORIGINAL:
                runOriginal(std::move(spec), testSequence);
                break;
            case TestMode::REWRITE_ONLY:
                runRewriteOnly(std::move(spec), testSequence);
                break;
            case TestMode::FULL_PIPELINE:
                runFullPipeline(std::move(spec), testSequence);
                break;
            }
            cout << "\n✓ " << testName << " COMPLETE!\n"
                 << endl;
        }
        catch (const runtime_error &e)
        {
            string errorMsg = e.what();
            if (errorMsg.find("UNSAT") != string::npos)
            {
                cout << "\n⊘ " << testName << " UNSAT: Preconditions not satisfiable\n"
                     << endl;
            }
            else
            {
                cout << "\n✗ " << testName << " FAILED: " << errorMsg << "\n"
                     << endl;
            }
        }
        catch (const exception &e)
        {
            cout << "\n✗ " << testName << " FAILED: " << e.what() << "\n"
                 << endl;
        }
    }

private:
    string getModeString()
    {
        switch (mode)
        {
        case TestMode::ORIGINAL:
            return "Original (No Rewrite)";
        case TestMode::REWRITE_ONLY:
            return "Rewrite Only (No Backend)";
        case TestMode::FULL_PIPELINE:
            return "Full Pipeline (With Backend)";
        }
        return "Unknown";
    }

    void runOriginal(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        Program atc = genATC(*spec, ts);
        PrintVisitor printer;
        printer.visitProgram(atc);
    }

    void runRewriteOnly(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        auto factory = make_unique<Library::LibraryFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);
    }

    void runFullPipeline(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        SymbolTable *symbolTable = new SymbolTable(nullptr);

        auto factory = make_unique<Library::LibraryFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);

        vector<Expr *> inputVars;
        ValueEnvironment *valueEnv = new ValueEnvironment(nullptr);

        unique_ptr<Program> ctc = tester.generateCTC(
            std::move(testApiATC),
            inputVars,
            valueEnv);

        if (!ctc)
        {
            cout << "\n[RESULT] UNSAT - Test preconditions cannot be satisfied" << endl;
            delete symbolTable;
            delete valueEnv;
            throw runtime_error("UNSAT: Preconditions not satisfiable");
        }

        cout << "\n[FINAL CTC]" << endl;
        PrintVisitor printer;
        printer.visitProgram(*ctc);

        delete symbolTable;
        delete valueEnv;
    }
};

// ============================================
// TEST EXECUTOR
// ============================================

class TestExecutor
{
private:
    TestMode mode;
    string backendUrl;

public:
    TestExecutor(TestMode m, const string &url = "http://localhost:5002")
        : mode(m), backendUrl(url) {}

    void runTest(
        const string &testName,
        unique_ptr<Spec> spec,
        const vector<string> &testSequence)
    {
        cout << "\n========================================" << endl;
        cout << "TEST: " << testName << endl;
        cout << "MODE: " << getModeString() << endl;
        cout << "DEPTH: " << testSequence.size() << " API calls" << endl;
        cout << "========================================\n"
             << endl;

        try
        {
            switch (mode)
            {
            case TestMode::ORIGINAL:
                runOriginal(std::move(spec), testSequence);
                break;
            case TestMode::REWRITE_ONLY:
                runRewriteOnly(std::move(spec), testSequence);
                break;
            case TestMode::FULL_PIPELINE:
                runFullPipeline(std::move(spec), testSequence);
                break;
            }
            cout << "\n✓ " << testName << " COMPLETE!\n"
                 << endl;
        }
        catch (const runtime_error &e)
        {
            string errorMsg = e.what();
            // Check if this is an UNSAT error
            if (errorMsg.find("UNSAT") != string::npos)
            {
                cout << "\n⊘ " << testName << " UNSAT: Preconditions not satisfiable\n"
                     << endl;
            }
            else
            {
                cout << "\n✗ " << testName << " FAILED: " << errorMsg << "\n"
                     << endl;
            }
        }
        catch (const exception &e)
        {
            cout << "\n✗ " << testName << " FAILED: " << e.what() << "\n"
                 << endl;
        }
    }

private:
    string getModeString()
    {
        switch (mode)
        {
        case TestMode::ORIGINAL:
            return "Original (No Rewrite)";
        case TestMode::REWRITE_ONLY:
            return "Rewrite Only (No Backend)";
        case TestMode::FULL_PIPELINE:
            return "Full Pipeline (With Backend)";
        }
        return "Unknown";
    }

    void runOriginal(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        Program atc = genATC(*spec, ts);
        PrintVisitor printer;
        printer.visitProgram(atc);
    }

    void runRewriteOnly(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        auto factory = make_unique<RestaurantFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);
    }

    void runFullPipeline(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        SymbolTable *symbolTable = new SymbolTable(nullptr);

        auto factory = make_unique<RestaurantFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);

        vector<Expr *> inputVars;
        ValueEnvironment *valueEnv = new ValueEnvironment(nullptr);

        unique_ptr<Program> ctc = tester.generateCTC(
            std::move(testApiATC),
            inputVars,
            valueEnv);

        if (!ctc)
        {
            cout << "\n[RESULT] UNSAT - Test preconditions cannot be satisfied" << endl;
            delete symbolTable;
            delete valueEnv;
            throw runtime_error("UNSAT: Preconditions not satisfiable");
        }

        cout << "\n[FINAL CTC]" << endl;
        PrintVisitor printer;
        printer.visitProgram(*ctc);

        delete symbolTable;
        delete valueEnv;
    }
};

// ============================================
// ECOMMERCE TEST EXECUTOR
// ============================================

class EcommerceTestExecutor
{
private:
    TestMode mode;
    string backendUrl;

public:
    EcommerceTestExecutor(TestMode m, const string &url = "http://localhost:3000")
        : mode(m), backendUrl(url) {}

    void runTest(
        const string &testName,
        unique_ptr<Spec> spec,
        const vector<string> &testSequence)
    {
        cout << "\n========================================" << endl;
        cout << "TEST: " << testName << endl;
        cout << "MODE: " << getModeString() << endl;
        cout << "DEPTH: " << testSequence.size() << " API calls" << endl;
        cout << "========================================\n"
             << endl;

        try
        {
            switch (mode)
            {
            case TestMode::ORIGINAL:
                runOriginal(std::move(spec), testSequence);
                break;
            case TestMode::REWRITE_ONLY:
                runRewriteOnly(std::move(spec), testSequence);
                break;
            case TestMode::FULL_PIPELINE:
                runFullPipeline(std::move(spec), testSequence);
                break;
            }
            cout << "\n✓ " << testName << " COMPLETE!\n"
                 << endl;
        }
        catch (const runtime_error &e)
        {
            string errorMsg = e.what();
            if (errorMsg.find("UNSAT") != string::npos)
            {
                cout << "\n⊘ " << testName << " UNSAT: Preconditions not satisfiable\n"
                     << endl;
            }
            else
            {
                cout << "\n✗ " << testName << " FAILED: " << errorMsg << "\n"
                     << endl;
            }
        }
        catch (const exception &e)
        {
            cout << "\n✗ " << testName << " FAILED: " << e.what() << "\n"
                 << endl;
        }
    }

private:
    string getModeString()
    {
        switch (mode)
        {
        case TestMode::ORIGINAL:
            return "Original (No Rewrite)";
        case TestMode::REWRITE_ONLY:
            return "Rewrite Only (No Backend)";
        case TestMode::FULL_PIPELINE:
            return "Full Pipeline (With Backend)";
        }
        return "Unknown";
    }

    void runOriginal(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        Program atc = genATC(*spec, ts);
        PrintVisitor printer;
        printer.visitProgram(atc);
    }

    void runRewriteOnly(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        auto factory = make_unique<Ecommerce::EcommerceFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);
    }

    void runFullPipeline(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        SymbolTable *symbolTable = new SymbolTable(nullptr);

        auto factory = make_unique<Ecommerce::EcommerceFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);

        vector<Expr *> inputVars;
        ValueEnvironment *valueEnv = new ValueEnvironment(nullptr);

        unique_ptr<Program> ctc = tester.generateCTC(
            std::move(testApiATC),
            inputVars,
            valueEnv);

        if (!ctc)
        {
            cout << "\n[RESULT] UNSAT - Test preconditions cannot be satisfied" << endl;
            delete symbolTable;
            delete valueEnv;
            throw runtime_error("UNSAT: Preconditions not satisfiable");
        }

        cout << "\n[FINAL CTC]" << endl;
        PrintVisitor printer;
        printer.visitProgram(*ctc);

        delete symbolTable;
        delete valueEnv;
    }
};

// ============================================
// TRIPVAULT TEST EXECUTOR
// ============================================

class TripVaultTestExecutor
{
private:
    TestMode mode;
    string backendUrl;

public:
    TripVaultTestExecutor(TestMode m, const string &url = "http://localhost:4001")
        : mode(m), backendUrl(url) {}

    void runTest(
        const string &testName,
        unique_ptr<Spec> spec,
        const vector<string> &testSequence)
    {
        cout << "\n========================================" << endl;
        cout << "TEST: " << testName << endl;
        cout << "MODE: " << getModeString() << endl;
        cout << "DEPTH: " << testSequence.size() << " API calls" << endl;
        cout << "========================================\n" << endl;

        try
        {
            switch (mode)
            {
            case TestMode::ORIGINAL:
                runOriginal(std::move(spec), testSequence);
                break;
            case TestMode::REWRITE_ONLY:
                runRewriteOnly(std::move(spec), testSequence);
                break;
            case TestMode::FULL_PIPELINE:
                runFullPipeline(std::move(spec), testSequence);
                break;
            }
            cout << "\n✓ " << testName << " COMPLETE!\n" << endl;
        }
        catch (const runtime_error &e)
        {
            string errorMsg = e.what();
            if (errorMsg.find("UNSAT") != string::npos)
            {
                cout << "\n⊘ " << testName << " UNSAT: Preconditions not satisfiable\n" << endl;
            }
            else
            {
                cout << "\n✗ " << testName << " FAILED: " << errorMsg << "\n" << endl;
            }
        }
        catch (const exception &e)
        {
            cout << "\n✗ " << testName << " FAILED: " << e.what() << "\n" << endl;
        }
    }

private:
    string getModeString()
    {
        switch (mode)
        {
        case TestMode::ORIGINAL:    return "Original (No Rewrite)";
        case TestMode::REWRITE_ONLY: return "Rewrite Only (No Backend)";
        case TestMode::FULL_PIPELINE: return "Full Pipeline (With Backend)";
        }
        return "Unknown";
    }

    void runOriginal(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        Program atc = genATC(*spec, ts);
        PrintVisitor printer;
        printer.visitProgram(atc);
    }

    void runRewriteOnly(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        auto factory = make_unique<TripVault::TripVaultFunctionFactory>(backendUrl);
        Tester tester(factory.get());
        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);
    }

    void runFullPipeline(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        SymbolTable *symbolTable = new SymbolTable(nullptr);

        auto factory = make_unique<TripVault::TripVaultFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);

        vector<Expr *> inputVars;
        ValueEnvironment *valueEnv = new ValueEnvironment(nullptr);

        unique_ptr<Program> ctc = tester.generateCTC(
            std::move(testApiATC),
            inputVars,
            valueEnv);

        if (!ctc)
        {
            cout << "\n[RESULT] UNSAT - Test preconditions cannot be satisfied" << endl;
            delete symbolTable;
            delete valueEnv;
            throw runtime_error("UNSAT: Preconditions not satisfiable");
        }

        cout << "\n[FINAL CTC]" << endl;
        PrintVisitor printer;
        printer.visitProgram(*ctc);

        delete symbolTable;
        delete valueEnv;
    }
};

// ============================================
// 25 TRIPVAULT TESTS (21 SAT + 4 UNSAT)
// ============================================

namespace TripVaultTests
{

// SAT-01: Register + Login (Depth=2)
void test01_registerLogin(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 01: Register → Login (Depth=2)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk"});
}

// SAT-02: Register + Login + Create Trip (Depth=3)
void test02_createTrip(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 02: Register → Login → Create Trip (Depth=3)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk"});
}

// SAT-03: Register + Login + Create Trip + Get Trips (Depth=4)
void test03_getUserTrips(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 03: Register → Login → Create Trip → Get Trips (Depth=4)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk", "getUserTripsOk"});
}

// SAT-04: Update Trip (Depth=4)
void test04_updateTrip(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 04: Register → Login → Create Trip → Update Trip (Depth=4)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk", "updateTripAdminOk"});
}

// SAT-05: Delete Trip (Depth=4)
void test05_deleteTrip(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 05: Register → Login → Create Trip → Delete Trip (Depth=4)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk", "deleteTripOk"});
}

// SAT-06: Two Users → Create Trip → Add Member (Depth=6)
void test06_addMember(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 06: Two Users → Create Trip → Add Member (Depth=6)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk",
         "registerUser2Ok", "loginUser2Ok", "addMemberOk"});
}

// SAT-07: Two Users → Create Trip → Join By Invite (Depth=6)
void test07_joinByInvite(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 07: Two Users → Create Trip → Join By Invite (Depth=6)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk",
         "registerUser2Ok", "loginUser2Ok", "joinByInviteOk"});
}

// SAT-08: Create Expense (Depth=4)
void test08_createExpense(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 08: Register → Login → Create Trip → Create Expense (Depth=4)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk", "createExpenseOk"});
}

// SAT-09: Get Expenses (Depth=5)
void test09_getExpenses(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 09: Create Trip → Create Expense → Get Expenses (Depth=5)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk", "createExpenseOk", "getExpensesOk"});
}

// SAT-10: Delete Expense (Depth=5)
void test10_deleteExpense(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 10: Create Trip → Create Expense → Delete Expense (Depth=5)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk", "createExpenseOk", "deleteExpenseOk"});
}

// SAT-11: Create Proposal (Depth=4)
void test11_createProposal(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 11: Create Trip → Create Proposal (Depth=4)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk", "createProposalOk"});
}

// SAT-12: Get Proposals (Depth=5)
void test12_getProposals(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 12: Create Trip → Create Proposal → Get Proposals (Depth=5)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk", "createProposalOk", "getProposalsOk"});
}

// SAT-13: Delete Proposal (Depth=5)
void test13_deleteProposal(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 13: Create Trip → Create Proposal → Delete Proposal (Depth=5)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk", "createProposalOk", "deleteProposalOk"});
}

// SAT-14: Multiple Expenses (Depth=6)
void test14_multipleExpenses(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 14: Create Trip → Create 3 Expenses (Depth=6)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk",
         "createExpenseOk", "createExpenseOk", "createExpenseOk"});
}

// SAT-15: Multiple Trips (Depth=5)
void test15_multipleTrips(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 15: Register → Login → Create 2 Trips → Get Trips (Depth=5)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk", "createTripOk", "getUserTripsOk"});
}

// SAT-16: Expense + Proposal (Depth=5)
void test16_expenseAndProposal(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 16: Create Trip → Create Expense → Create Proposal (Depth=5)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk", "createExpenseOk", "createProposalOk"});
}

// SAT-17: Multi-User Add Member Then Create Expense (Depth=7)
void test17_memberCreatesExpense(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 17: Two Users → Add Member → Create Expense (Depth=7)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk",
         "registerUser2Ok", "loginUser2Ok", "addMemberOk", "createExpenseOk"});
}

// SAT-18: Full Trip Lifecycle (Depth=7)
void test18_fullTripLifecycle(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 18: Full Lifecycle: Create→Expense→Proposal→Delete Both (Depth=7)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk",
         "createExpenseOk", "createProposalOk",
         "deleteExpenseOk", "deleteProposalOk"});
}

// SAT-19: Join Then Create Expense (Depth=7)
void test19_joinAndCreateExpense(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 19: Multi-User Join → Create Expense (Depth=7)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk",
         "registerUser2Ok", "loginUser2Ok", "joinByInviteOk", "createExpenseOk"});
}

// SAT-20: Delete and Re-create Expense (Depth=6)
void test20_deleteAndRecreateExpense(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 20: Create Expense → Delete → Re-create (Depth=6)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk",
         "createExpenseOk", "deleteExpenseOk", "createExpenseOk"});
}

// SAT-21: Multiple Proposals (Depth=6)
void test21_multipleProposals(TripVaultTestExecutor &executor)
{
    executor.runTest("[SAT] Test 21: Create Trip → Create 2 Proposals → Get All (Depth=6)",
        makeTripVaultSpec(),
        {"registerUserOk", "loginUserOk", "createTripOk",
         "createProposalOk", "createProposalOk", "getProposalsOk"});
}

// UNSAT-22: Login Without Register (Depth=1)
void test22_loginWithoutRegister(TripVaultTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 22: Login Without Register (Depth=1)",
        makeTripVaultSpec(),
        {"loginUserOk"});
}

// UNSAT-23: Create Trip Without Login (Depth=1)
void test23_createTripWithoutLogin(TripVaultTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 23: Create Trip Without Login (Depth=1)",
        makeTripVaultSpec(),
        {"createTripOk"});
}

// UNSAT-24: Delete Expense Without Auth (Depth=1)
void test24_deleteExpenseWithoutAuth(TripVaultTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 24: Delete Expense Without Auth (Depth=1)",
        makeTripVaultSpec(),
        {"deleteExpenseOk"});
}

// UNSAT-25: Delete Trip Without Login (Depth=1)
void test25_deleteTripWithoutAuth(TripVaultTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 25: Delete Trip Without Login (Depth=1)",
        makeTripVaultSpec(),
        {"deleteTripOk"});
}

} // namespace TripVaultTests

// ============================================
// GHOSTSOCKET TEST EXECUTOR
// ============================================

class GhostSocketTestExecutor
{
private:
    TestMode mode;
    string backendUrl;

public:
    GhostSocketTestExecutor(TestMode m, const string &url = "http://localhost:4002")
        : mode(m), backendUrl(url) {}

    void runTest(
        const string &testName,
        unique_ptr<Spec> spec,
        const vector<string> &testSequence)
    {
        cout << "\n========================================" << endl;
        cout << "TEST: " << testName << endl;
        cout << "MODE: " << getModeString() << endl;
        cout << "DEPTH: " << testSequence.size() << " API calls" << endl;
        cout << "========================================\n" << endl;

        try
        {
            switch (mode)
            {
            case TestMode::ORIGINAL:
                runOriginal(std::move(spec), testSequence);
                break;
            case TestMode::REWRITE_ONLY:
                runRewriteOnly(std::move(spec), testSequence);
                break;
            case TestMode::FULL_PIPELINE:
                runFullPipeline(std::move(spec), testSequence);
                break;
            }
            cout << "\n✓ " << testName << " COMPLETE!\n" << endl;
        }
        catch (const runtime_error &e)
        {
            string errorMsg = e.what();
            if (errorMsg.find("UNSAT") != string::npos)
            {
                cout << "\n⊘ " << testName << " UNSAT: Preconditions not satisfiable\n" << endl;
            }
            else
            {
                cout << "\n✗ " << testName << " FAILED: " << errorMsg << "\n" << endl;
            }
        }
        catch (const exception &e)
        {
            cout << "\n✗ " << testName << " FAILED: " << e.what() << "\n" << endl;
        }
    }

private:
    string getModeString()
    {
        switch (mode)
        {
        case TestMode::ORIGINAL:     return "Original (No Rewrite)";
        case TestMode::REWRITE_ONLY: return "Rewrite Only (No Backend)";
        case TestMode::FULL_PIPELINE: return "Full Pipeline (With Backend)";
        }
        return "Unknown";
    }

    void runOriginal(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        Program atc = genATC(*spec, ts);
        PrintVisitor printer;
        printer.visitProgram(atc);
    }

    void runRewriteOnly(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        auto factory = make_unique<GhostSocket::GhostSocketFunctionFactory>(backendUrl);
        Tester tester(factory.get());
        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);
    }

    void runFullPipeline(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        SymbolTable *symbolTable = new SymbolTable(nullptr);

        auto factory = make_unique<GhostSocket::GhostSocketFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);

        vector<Expr *> inputVars;
        ValueEnvironment *valueEnv = new ValueEnvironment(nullptr);

        unique_ptr<Program> ctc = tester.generateCTC(
            std::move(testApiATC),
            inputVars,
            valueEnv);

        if (!ctc)
        {
            cout << "\n[RESULT] UNSAT - Test preconditions cannot be satisfied" << endl;
            delete symbolTable;
            delete valueEnv;
            throw runtime_error("UNSAT: Preconditions not satisfiable");
        }

        cout << "\n[FINAL CTC]" << endl;
        PrintVisitor printer;
        printer.visitProgram(*ctc);

        delete symbolTable;
        delete valueEnv;
    }
};

// ============================================
// 25 GHOSTSOCKET TESTS (21 SAT + 4 UNSAT)
// ============================================

namespace GhostSocketTests
{

// SAT-01: Register user only
void test01_registerUser(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 01: Register User (Depth=1)",
        makeGhostSocketSpec(),
        {"registerUserOk"});
}

// SAT-02: Register two users
void test02_registerTwoUsers(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 02: Register Two Users (Depth=2)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerUser2Ok"});
}

// SAT-03: Register user + device
void test03_registerDevice(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 03: Register User + Device (Depth=2)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerDeviceOk"});
}

// SAT-04: Get my devices
void test04_getMyDevices(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 04: Register → Device → GetMyDevices (Depth=3)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerDeviceOk", "getMyDevicesOk"});
}

// SAT-05: Get device info (owner has access)
void test05_getDeviceInfo(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 05: Register → Device → GetDeviceInfo (Depth=3)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerDeviceOk", "getDeviceInfoOk"});
}

// SAT-06: Create session (201 check)
void test06_createSession(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 06: Register → Device → CreateSession (Depth=3)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerDeviceOk", "createSessionOk"});
}

// SAT-07: Full join session flow
void test07_joinSession(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 07: Two Users → Device → Session → Join (Depth=5)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerUser2Ok", "registerDeviceOk", "createSessionOk", "joinSessionOk"});
}

// SAT-08: Create session + get sessions list
void test08_getSessions(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 08: Register → Device → Session → GetSessions (Depth=4)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerDeviceOk", "createSessionOk", "getSessionsOk"});
}

// SAT-09: Create session + terminate
void test09_terminateSession(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 09: Register → Device → Session → Terminate (Depth=4)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerDeviceOk", "createSessionOk", "terminateSessionOk"});
}

// SAT-10: Delete device (owner)
void test10_deleteDevice(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 10: Register → Device → DeleteDevice (Depth=3)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerDeviceOk", "deleteDeviceOk"});
}

// SAT-11: Join + get other devices
void test11_getOtherDevices(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 11: Two Users → Device → Session → Join → GetOtherDevices (Depth=6)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerUser2Ok", "registerDeviceOk", "createSessionOk", "joinSessionOk", "getOtherDevicesOk"});
}

// SAT-12: Update permissions
void test12_updatePermissions(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 12: Register → Device → Session → UpdatePermissions (Depth=4)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerDeviceOk", "createSessionOk", "updatePermissionsOk"});
}

// SAT-13: Forbidden - device info without device (no D entry)
void test13_deviceInfoForbidden(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 13: GetDeviceInfo Forbidden (no device, Depth=1)",
        makeGhostSocketSpec(),
        {"getDeviceInfoForbiddenErr"});
}

// SAT-14: Forbidden - create session without device ownership
void test14_createSessionForbidden(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 14: CreateSession Forbidden (no device, Depth=1)",
        makeGhostSocketSpec(),
        {"createSessionForbiddenErr"});
}

// SAT-15: Session not found when joining
void test15_joinSessionNotFound(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 15: JoinSession NotFound (no session, Depth=1)",
        makeGhostSocketSpec(),
        {"joinSessionNotFoundErr"});
}

// SAT-16: Terminate session forbidden (no session)
void test16_terminateSessionForbidden(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 16: TerminateSession Forbidden (no session, Depth=1)",
        makeGhostSocketSpec(),
        {"terminateSessionForbiddenErr"});
}

// SAT-17: Full lifecycle: register → device → session → join → terminate
void test17_fullSessionLifecycle(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 17: Full Session Lifecycle (Depth=6)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerUser2Ok", "registerDeviceOk", "createSessionOk", "joinSessionOk", "terminateSessionOk"});
}

// SAT-18: Get devices + create session
void test18_devicesAndSession(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 18: Register → Device → GetMyDevices → Session → GetSessions (Depth=5)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerDeviceOk", "getMyDevicesOk", "createSessionOk", "getSessionsOk"});
}

// SAT-19: Device info + create session
void test19_deviceInfoAndSession(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 19: Register → Device → DeviceInfo → CreateSession (Depth=4)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerDeviceOk", "getDeviceInfoOk", "createSessionOk"});
}

// SAT-20: Join then update permissions
void test20_joinAndUpdatePermissions(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 20: Two Users → Device → Session → Join → UpdatePermissions (Depth=6)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerUser2Ok", "registerDeviceOk", "createSessionOk", "joinSessionOk", "updatePermissionsOk"});
}

// SAT-21: Session list and terminate
void test21_sessionListAndTerminate(GhostSocketTestExecutor &executor)
{
    executor.runTest("[SAT] Test 21: Register → Device → Session → GetSessions → Terminate (Depth=5)",
        makeGhostSocketSpec(),
        {"registerUserOk", "registerDeviceOk", "createSessionOk", "getSessionsOk", "terminateSessionOk"});
}

// UNSAT-22: Create session alone (no device in D)
void test22_createSessionNoDevice(GhostSocketTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 22: CreateSession alone — no device (Expected UNSAT)",
        makeGhostSocketSpec(),
        {"createSessionOk"});
}

// UNSAT-23: Join session alone (no session in S)
void test23_joinSessionNoSession(GhostSocketTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 23: JoinSession alone — no session (Expected UNSAT)",
        makeGhostSocketSpec(),
        {"joinSessionOk"});
}

// UNSAT-24: Terminate session alone (no session in S)
void test24_terminateNoSession(GhostSocketTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 24: TerminateSession alone — no session (Expected UNSAT)",
        makeGhostSocketSpec(),
        {"terminateSessionOk"});
}

// UNSAT-25: Get device info alone (no device in D)
void test25_deviceInfoNoDevice(GhostSocketTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 25: GetDeviceInfo alone — no device (Expected UNSAT)",
        makeGhostSocketSpec(),
        {"getDeviceInfoOk"});
}

} // namespace GhostSocketTests

// ============================================
// SERVEEZ TEST EXECUTOR
// ============================================

class ServeezTestExecutor
{
private:
    TestMode mode;
    string backendUrl;

public:
    ServeezTestExecutor(TestMode m, const string &url = "http://localhost:8083")
        : mode(m), backendUrl(url) {}

    void runTest(
        const string &testName,
        unique_ptr<Spec> spec,
        const vector<string> &testSequence)
    {
        cout << "\n========================================" << endl;
        cout << "TEST: " << testName << endl;
        cout << "MODE: " << getModeString() << endl;
        cout << "DEPTH: " << testSequence.size() << " API calls" << endl;
        cout << "========================================\n" << endl;

        try
        {
            switch (mode)
            {
            case TestMode::ORIGINAL:
                runOriginal(std::move(spec), testSequence);
                break;
            case TestMode::REWRITE_ONLY:
                runRewriteOnly(std::move(spec), testSequence);
                break;
            case TestMode::FULL_PIPELINE:
                runFullPipeline(std::move(spec), testSequence);
                break;
            }
            cout << "\n✓ " << testName << " COMPLETE!\n" << endl;
        }
        catch (const runtime_error &e)
        {
            string errorMsg = e.what();
            if (errorMsg.find("UNSAT") != string::npos)
            {
                cout << "\n⊘ " << testName << " UNSAT: Preconditions not satisfiable\n" << endl;
            }
            else
            {
                cout << "\n✗ " << testName << " FAILED: " << errorMsg << "\n" << endl;
            }
        }
        catch (const exception &e)
        {
            cout << "\n✗ " << testName << " FAILED: " << e.what() << "\n" << endl;
        }
    }

private:
    string getModeString()
    {
        switch (mode)
        {
        case TestMode::ORIGINAL:     return "Original (No Rewrite)";
        case TestMode::REWRITE_ONLY: return "Rewrite Only (No Backend)";
        case TestMode::FULL_PIPELINE: return "Full Pipeline (With Backend)";
        }
        return "Unknown";
    }

    void runOriginal(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        Program atc = genATC(*spec, ts);
        PrintVisitor printer;
        printer.visitProgram(atc);
    }

    void runRewriteOnly(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        auto factory = make_unique<Serveez::ServeezFunctionFactory>(backendUrl);
        Tester tester(factory.get());
        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);
    }

    void runFullPipeline(unique_ptr<Spec> spec, const vector<string> &ts)
    {
        SymbolTable *symbolTable = new SymbolTable(nullptr);

        auto factory = make_unique<Serveez::ServeezFunctionFactory>(backendUrl);
        Tester tester(factory.get());

        unique_ptr<Program> testApiATC = tester.generateATC(std::move(spec), ts);

        vector<Expr *> inputVars;
        ValueEnvironment *valueEnv = new ValueEnvironment(nullptr);

        unique_ptr<Program> ctc = tester.generateCTC(
            std::move(testApiATC),
            inputVars,
            valueEnv);

        if (!ctc)
        {
            cout << "\n[RESULT] UNSAT - Test preconditions cannot be satisfied" << endl;
            delete symbolTable;
            delete valueEnv;
            throw runtime_error("UNSAT: Preconditions not satisfiable");
        }

        cout << "\n[FINAL CTC]" << endl;
        PrintVisitor printer;
        printer.visitProgram(*ctc);

        delete symbolTable;
        delete valueEnv;
    }
};

// ============================================
// 25 SERVEEZ TESTS (21 SAT + 4 UNSAT)
// ============================================

namespace ServeezTests
{

// SAT-01: Register a USER
void test01_registerUser(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 01: Register USER (Depth=1)",
        makeServeezSpec(),
        {"registerUserOk"});
}

// SAT-02: Register a PROVIDER
void test02_registerProvider(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 02: Register PROVIDER (Depth=1)",
        makeServeezSpec(),
        {"registerProviderOk"});
}

// SAT-03: Register an ADMIN
void test03_registerAdmin(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 03: Register ADMIN (Depth=1)",
        makeServeezSpec(),
        {"registerAdminOk"});
}

// SAT-04: Admin creates a category
void test04_createCategory(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 04: Admin → CreateCategory (Depth=2)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk"});
}

// SAT-05: Provider creates a listing
void test05_createListing(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 05: Admin → Category → Provider → Listing (Depth=4)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk"});
}

// SAT-06: Public get all listings
void test06_getListings(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 06: Admin → Category → Provider → Listing → GetListings (Depth=5)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "getListingsOk"});
}

// SAT-07: Public get listing by ID
void test07_getListingById(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 07: Admin → Category → Provider → Listing → GetListingById (Depth=5)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "getListingByIdOk"});
}

// SAT-08: User creates a booking
void test08_createBooking(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 08: Admin → Category → Provider → Listing → User → Booking (Depth=6)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "registerUserOk", "createBookingOk"});
}

// SAT-09: User gets own bookings
void test09_getMyBookings(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 09: ... → Booking → GetMyBookings (Depth=7)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "registerUserOk", "createBookingOk", "getMyBookingsOk"});
}

// SAT-10: Provider confirms booking
void test10_confirmBooking(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 10: ... → Booking → ConfirmBooking (Depth=7)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "registerUserOk", "createBookingOk", "confirmBookingOk"});
}

// SAT-11: Provider completes booking (after confirm)
void test11_completeBooking(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 11: ... → Booking → Confirm → Complete (Depth=8)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "registerUserOk", "createBookingOk", "confirmBookingOk", "completeBookingOk"});
}

// SAT-12: User reviews after complete
void test12_createReview(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 12: ... → Confirm → Complete → Review (Depth=9)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "registerUserOk", "createBookingOk", "confirmBookingOk", "completeBookingOk", "svzCreateReviewOk"});
}

// SAT-13: Get listing reviews (public)
void test13_getListingReviews(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 13: ... → Review → GetListingReviews (Depth=10)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "registerUserOk", "createBookingOk", "confirmBookingOk", "completeBookingOk", "svzCreateReviewOk", "getListingReviewsOk"});
}

// SAT-14: User cancels a booking
void test14_cancelBooking(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 14: ... → Booking → CancelBooking (Depth=7)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "registerUserOk", "createBookingOk", "cancelBookingOk"});
}

// SAT-15: Unauth listing creation rejected
void test15_createListingUnauth(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 15: Admin → Category → CreateListingUnauth (Depth=3)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "createListingUnauthErr"});
}

// SAT-16: Provider cannot create booking (role enforcement)
void test16_createBookingAsProvider(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 16: Admin → Category → Provider → Listing → CreateBookingAsProvider (Depth=5)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "createBookingAsProviderErr"});
}

// SAT-17: Two listings from same provider
void test17_twoListings(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 17: Admin → Category → Provider → Listing1 → Listing2 (Depth=5)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "createListingOk"});
}

// SAT-18: Full lifecycle: register + list + book + confirm + complete + review
void test18_fullLifecycle(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 18: Full booking lifecycle (Depth=8)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "registerUserOk", "createBookingOk", "confirmBookingOk", "completeBookingOk"});
}

// SAT-19: Multiple users book same listing
void test19_multipleBookings(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 19: Two users book same listing (Depth=8)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "registerUserOk", "createBookingOk", "registerUserOk", "createBookingOk"});
}

// SAT-20: Booking + get bookings + cancel
void test20_bookAndCancel(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 20: Book → GetMyBookings → Cancel (Depth=8)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "registerUserOk", "createBookingOk", "getMyBookingsOk", "cancelBookingOk"});
}

// SAT-21: Get listing by ID then book
void test21_getListingThenBook(ServeezTestExecutor &executor)
{
    executor.runTest("[SAT] Test 21: Listing → GetById → Book → Confirm (Depth=8)",
        makeServeezSpec(),
        {"registerAdminOk", "createCategoryOk", "registerProviderOk", "createListingOk", "getListingByIdOk", "registerUserOk", "createBookingOk", "confirmBookingOk"});
}

// UNSAT-22: createCategory alone — no ADMIN
void test22_createCategoryNoAdmin(ServeezTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 22: createCategory alone — no admin (Expected UNSAT)",
        makeServeezSpec(),
        {"createCategoryOk"});
}

// UNSAT-23: createListing alone — no PROVIDER, no C
void test23_createListingNoProvider(ServeezTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 23: createListing alone — no provider/category (Expected UNSAT)",
        makeServeezSpec(),
        {"createListingOk"});
}

// UNSAT-24: createBooking alone — no USER, no L
void test24_createBookingNoUser(ServeezTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 24: createBooking alone — no user/listing (Expected UNSAT)",
        makeServeezSpec(),
        {"createBookingOk"});
}

// UNSAT-25: confirmBooking alone — no PROVIDER, no B
void test25_confirmBookingNone(ServeezTestExecutor &executor)
{
    executor.runTest("[UNSAT] Test 25: confirmBooking alone — no booking (Expected UNSAT)",
        makeServeezSpec(),
        {"confirmBookingOk"});
}

} // namespace ServeezTests

// ============================================
// 25 COMPREHENSIVE RESTAURANT TESTS
// ============================================

namespace RestaurantTests
{

    // ========================================
    // DEPTH 1: Single API Call Tests
    // ========================================

    void test01_registerLogin(TestExecutor &executor)
    {
        executor.runTest(
            "Test 01: Register Customer-->Login (Depth=2)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginCustomerOk"});
    }

    void test02_loginFailure(TestExecutor &executor)
    {
        executor.runTest(
            "Test 02: loginCustomerErr (Depth=1)",
            makeRestaurantSpec(), {"loginCustomerErr"}
            // {"loginCustomerOk"} // Should fail: user not in U
        );
    }

    void test03_browseOnly(TestExecutor &executor)
    {
        executor.runTest(
            "Test 03: registerCustomerOk → loginWrongPasswordErr (Depth=2)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginWrongPasswordErr"}
            // {"browseRestaurantsOk"} // Public API, no auth needed
        );
    }

    // ========================================
    // DEPTH 2: Two API Call Tests
    // ========================================

    void test04_Login(TestExecutor &executor)
    {
        executor.runTest(
            "Test 04: loginCustomerOk (Depth=1, Expected UNSAT)",
            makeRestaurantSpec(),
            {"loginCustomerOk"}
            // {"loginOk"} // should be UNSAT
        );
    }

    void test05_registerOwnerAndLogin(TestExecutor &executor)
    {
        executor.runTest(
            "Test 05: registerCustomerOk → loginCustomerOk → addToCartRestaurantOk (Depth=3, Expected UNSAT)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginCustomerOk", "addToCartRestaurantOk"} // unsat
            // {"registerOwnerOk", "loginOk"} // should be SAT
        );
    }

    void test06_registerAgentAndLogin(TestExecutor &executor)
    {
        executor.runTest(
            "Test 06: registerOwnerOk → loginOwnerOk → createRestaurantOk → placeOrderOk (Depth=4, Expected UNSAT)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "placeOrderOk"}
            // {"registerAgentOk", "loginOk"} // should be SAT
        );
    }

    // ========================================
    // DEPTH 4: Three API Call Tests
    // ========================================

    void test07_loginBrowseView(TestExecutor &executor)
    {
        executor.runTest(
            "Test 07: registerOwnerOk → loginOwnerOk → createRestaurantOk → registerCustomerOk → loginCustomerOk → browseRestaurantsOk → viewMenuOk (Depth=7)",
            makeRestaurantSpec(),
            {"registerOwnerOk",     // 1. Owner registers
             "loginOwnerOk",        // 2. Owner logs in
             "createRestaurantOk",  // 3. Owner creates restaurant
             "registerCustomerOk",  // 4. Customer registers
             "loginCustomerOk",     // 5. Customer logs in
             "browseRestaurantsOk", // 6. Customer browses
             "viewMenuOk"}          // should be SAT
        );
    }

    void test08_loginAndAddToCart(TestExecutor &executor)
    {
        executor.runTest(
            "Test 08: registerOwnerOk → loginOwnerOk → createRestaurantOk → addMenuItemOk → registerCustomerOk → loginCustomerOk → browseRestaurantsOk → viewMenuOk → addToCartRestaurantOk (Depth=9)",
            makeRestaurantSpec(),
            {"registerOwnerOk",       // 1. Owner registers
             "loginOwnerOk",          // 2. Owner logs in
             "createRestaurantOk",    // 3. Owner creates restaurant
             "addMenuItemOk",         // 4. Owner adds menu item
             "registerCustomerOk",    // 5. Customer registers
             "loginCustomerOk",       // 6. Customer logs in
             "browseRestaurantsOk",   // 7. Customer browses
             "viewMenuOk",            // 8. Customer views menu
             "addToCartRestaurantOk"} // 9. Customer adds to cart
        );
    }

    void test09_loginAndReview(TestExecutor &executor)
    {
        executor.runTest(
            "Test 09: registerOwnerOk → loginOwnerOk → createRestaurantOk → addMenuItemOk → registerCustomerOk → loginCustomerOk → browseRestaurantsOk → viewMenuOk → addToCartRestaurantOk → placeOrderOk (Depth=10)",
            makeRestaurantSpec(),
            {"registerOwnerOk",     // 1. Owner registers
             "loginOwnerOk",        // 2. Owner logs in
             "createRestaurantOk",  // 3. Owner creates restaurant
             "addMenuItemOk",       // 4. Owner adds menu item
             "registerCustomerOk",  // 5. Customer registers
             "loginCustomerOk",     // 6. Customer logs in
             "browseRestaurantsOk", // 7. Customer browses
             "viewMenuOk",          // 8. Customer views menu
             "addToCartRestaurantOk",
             "placeOrderOk"});
    }

    void test10_reviewWithoutLogin(TestExecutor &executor)
    {
        executor.runTest(
            "Test 10: Full Order Lifecycle with Review (Depth=19)",
            makeRestaurantSpec(),
            {"registerOwnerOk",          // 1. Owner registers
             "loginOwnerOk",             // 2. Owner logs in
             "createRestaurantOk",       // 3. Owner creates restaurant
             "addMenuItemOk",            // 4. Owner adds menu item
             "registerCustomerOk",       // 5. Customer registers
             "loginCustomerOk",          // 6. Customer logs in
             "browseRestaurantsOk",      // 7. Customer browses
             "viewMenuOk",               // 8. Customer views menu
             "addToCartRestaurantOk",    // 9. Customer adds to cart
             "placeOrderOk",             // 10. Customer places order (status: placed)
             "registerAgentOk",          // 11. Delivery agent registers
             "loginAgentOk",             // 12. Delivery agent logs in
             "updateOrderStatusOwnerOk", // 13. Owner accepts order (status: accepted)
             "updateOrderStatusOwnerOk", // 14. Owner marks preparing (status: preparing)
             "updateOrderStatusOwnerOk", // 15. Owner marks ready (status: ready)
             "assignOrderOk",            // 16. Owner assigns delivery agent
             "updateOrderStatusAgentOk", // 17. Agent picks up (status: picked_up)
             "updateOrderStatusAgentOk", // 18. Agent delivers (status: delivered)
             "leaveReviewOk"});
    }

    void test11_fullCustomerOrder(TestExecutor &executor)
    {
        executor.runTest(
            "Test 11: registerOwnerOk → loginOwnerOk → createRestaurantOk (Depth=3)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk"}
            // {"registerCustomerOk", "loginOk", "addToCartOk", "placeOrderOk"} // should fail no browse restaurants
        );
    }

    void test12_ownerCreateRestaurant(TestExecutor &executor)
    {
        executor.runTest(
            "Test 12: registerOwnerOk → loginOwnerOk → createRestaurantOk → addMenuItemOk → addMenuItemOk → addMenuItemOk (Depth=6)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk",
             "addMenuItemOk", "addMenuItemOk", "addMenuItemOk"}
            // {"registerOwnerOk", "loginOk", "createRestaurantOk", "addMenuItemOk"}
        );
    }

    void test13_cartWithoutItems(TestExecutor &executor)
    {
        executor.runTest(
            "Test 13: registerOwnerOk → loginOwnerOk → createRestaurantOk → addMenuItemOk → registerCustomerOk → loginCustomerOk → browseRestaurantsOk → viewMenuOk → addToCartRestaurantOk (Depth=9)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartRestaurantOk"}
            // {"registerCustomerOk", "loginOk", "placeOrderErr"} // Should fail: cart empty
        );
    }

    // ========================================
    // DEPTH 5: Five API Call Tests
    // ========================================

    void test14_customerFullWorkflow(TestExecutor &executor)
    {
        executor.runTest(
            "Test 14: registerOwnerOk → loginOwnerOk → createRestaurantOk → addMenuItemOk → registerCustomerOk → loginCustomerOk → browseRestaurantsOk → viewMenuOk → addToCartRestaurantOk → placeOrderOk (Depth=10)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartRestaurantOk", "placeOrderOk"}
            // {"registerCustomerOk", "loginOk", "browseRestaurantsOk", "addToCartRestaurantOk", "placeOrderOk"}
        );
    }

    void test15_ownerFullSetup(TestExecutor &executor)
    {
        executor.runTest(
            "Test 15: registerOwnerOk → loginOwnerOk → createRestaurantOk → addMenuItemOk → registerCustomerOk → loginCustomerOk → browseRestaurantsOk → viewMenuOk → addToCartRestaurantOk → placeOrderOk → updateOrderStatusOwnerOk x3 (Depth=13)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartRestaurantOk", "placeOrderOk",
             "updateOrderStatusOwnerOk", // accepted
             "updateOrderStatusOwnerOk", // preparing
             "updateOrderStatusOwnerOk"}
            // {"registerOwnerOk", "loginOk", "createRestaurantOk", "addMenuItemOk", "addMenuItemOk"}
        );
    }

    void test16_agentAssignOrder(TestExecutor &executor)
    {
        executor.runTest(
            "Test 16: registerCustomerOk → registerCustomerOk (Depth=2, Expected UNSAT - Duplicate Registration)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "registerOwnerOk"}
            //{"registerAgentOk", "loginOk", "placeOrderOk", "assignOrderOk", "updateOrderStatusAgentOk"} // delivery agent can't place order
        );
    }

    // ========================================
    // DEPTH 6: Six API Call Tests
    // ========================================

    void test17_ownerManageOrder(TestExecutor &executor)
    {
        executor.runTest(
            "Test 17: registerOwnerOk → loginOwnerOk → createRestaurantOk → addMenuItemOk → registerCustomerOk → loginCustomerOk → browseRestaurantsOk → viewMenuOk → addToCartRestaurantOk → placeOrderOk → registerAgentOk → loginAgentOk → updateOrderStatusAgentOk (Depth=13, Expected UNSAT)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartRestaurantOk", "placeOrderOk",
             "registerAgentOk", "loginAgentOk",
             "updateOrderStatusAgentOk"}
            //{"registerOwnerOk", "loginOk", "createRestaurantOk", "placeOrderOk"} // should be UNSAT, owner can't place order
        );
    }

    void test18_multipleCartAdditions(TestExecutor &executor)
    {
        executor.runTest(
            "Test 18: loginCustomerOk → browseRestaurantsOk → addToCartRestaurantOk x3 → placeOrderOk (Depth=6, Expected UNSAT - No Registration)",
            makeRestaurantSpec(),
            {"loginCustomerOk", "browseRestaurantsOk", "addToCartRestaurantOk", "addToCartRestaurantOk", "addToCartRestaurantOk", "placeOrderOk"} // should return unsat as no registration
        );
    }

    void test19_wrongRoleAccess(TestExecutor &executor)
    {
        executor.runTest(
            "Test 19: registerCustomerOk → loginCustomerOk → createRestaurantCustomerErr (Depth=3, Expected UNSAT - Customer Can't Create Restaurant)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginCustomerOk", "createRestaurantCustomerErr"});
    }

    // ========================================
    // DEPTH 7: Seven API Call Tests
    // ========================================

    void test20_fullLifecycle(TestExecutor &executor)
    {
        executor.runTest(
            "Test 20: registerCustomerOk → loginCustomerOk → browseRestaurantsOk → viewMenuOk → addToCartRestaurantOk → placeOrderOk → leaveReviewOk (Depth=7, Expected UNSAT)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartRestaurantOk", "placeOrderOk", "leaveReviewOk"});
    }

    void test21_ownerCompleteFlow(TestExecutor &executor)
    {
        executor.runTest(
            "Test 21: registerOwnerOk → loginOwnerOk → createRestaurantOk → addMenuItemOk x3 (Depth=6)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk",
             "addMenuItemOk", "addMenuItemOk", "addMenuItemOk"});
    }

    

    void test22_complexOrderManagement(TestExecutor &executor)
    {
        executor.runTest(
            "Test 23: registerOwnerOk → loginOwnerOk → createRestaurantOk → addMenuItemOk → registerCustomerOk → loginCustomerOk → browseRestaurantsOk → viewMenuOk → addToCartRestaurantOk → placeOrderOk (Depth=10)",
            makeRestaurantSpec(),
            {"registerOwnerOk",       // 1. Owner registers
             "loginOwnerOk",          // 2. Owner logs in
             "createRestaurantOk",    // 3. Owner creates restaurant
             "addMenuItemOk",         // 4. Owner adds menu item
             "registerCustomerOk",    // 5. Customer registers
             "loginCustomerOk",       // 6. Customer logs in
             "browseRestaurantsOk",   // 7. Customer browses
             "viewMenuOk",            // 8. Customer views menu
             "addToCartRestaurantOk", // 9. Customer adds to cart
             "placeOrderOk"});
    }

    void test23_invalidSequence(TestExecutor &executor)
    {
        executor.runTest(
            "Test 24: registerCustomerOk → loginCustomerOk → leaveReviewOk (Depth=3, Expected UNSAT - Review Before Order)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "loginCustomerOk", "leaveReviewOk"}
            // Should fail: can't review restaurant you haven't ordered from
        );
    }

    void test24_deepWorkflow(TestExecutor &executor)
    {
        executor.runTest(
            "Test 25: registerOwnerOk → loginOwnerOk → createRestaurantOk → addMenuItemOk x5 → updateOrderStatusOwnerOk x2 (Depth=10, Expected UNSAT)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk",
             "addMenuItemOk", "addMenuItemOk", "addMenuItemOk",
             "addMenuItemOk", "addMenuItemOk", "updateOrderStatusOwnerOk", "updateOrderStatusOwnerOk"});
    }

    void test25_registerCustomerDuplicate(TestExecutor &executor)
    {
        executor.runTest(
            "Test 26: registerCustomerOk → registerCustomerOk (Depth=2, Expected UNSAT - Duplicate Email)",
            makeRestaurantSpec(),
            {"registerCustomerOk", "registerCustomerOk"}); // edge case should return unsat
    }
    
}

// ============================================
// LIBRARY TESTS
// ============================================

namespace LibraryTests
{

    // ========================================
    // DEPTH 1: Single API Call Tests
    // ========================================

    void test01_getAllBooks(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 01: Get All Books (Depth=1)",
            makeLibrarySpec(),
            {"getAllBooksOk"});
    }

    void test02_getAllStudents(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 02: Get All Students (Depth=1)",
            makeLibrarySpec(),
            {"getAllStudentsOk"});
    }

    void test03_saveBook(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 03: Save Book (Depth=1)",
            makeLibrarySpec(),
            {"saveBookOk"});
    }

    void test04_saveStudent(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 04: Save Student (Depth=1)",
            makeLibrarySpec(),
            {"saveStudentOk"});
    }

    // ========================================
    // DEPTH 2: Two API Call Tests
    // ========================================

    void test05_saveAndGetBook(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 05: Save Book → Get Book (Depth=2)",
            makeLibrarySpec(),
            {"saveBookOk", "getBookByCodeOk"});
    }

    void test06_saveAndGetStudent(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 06: Save Student → Get Student (Depth=2)",
            makeLibrarySpec(),
            {"saveStudentOk", "getStudentByIdOk"});
    }

    void test07_saveBookTwice(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 07: Save Two Books (Depth=2)",
            makeLibrarySpec(),
            {"saveBookOk", "saveBookOk"});
    }

    void test08_getBookNotFound(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 08: Get Book Not Found (Depth=1)",
            makeLibrarySpec(),
            {"getBookByCodeErr"});
    }

    // ========================================
    // DEPTH 3: Three API Call Tests
    // ========================================

    void test09_bookCRUD(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 09: Book CRUD - Save → Update → Delete (Depth=3)",
            makeLibrarySpec(),
            {"saveBookOk", "updateBookOk", "deleteBookOk"});
    }

    void test10_studentCRUD(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 10: Student CRUD - Save → Update → Delete (Depth=3)",
            makeLibrarySpec(),
            {"saveStudentOk", "updateStudentOk", "deleteStudentOk"});
    }

    void test11_createBookAndStudent(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 11: Create Book and Student (Depth=2)",
            makeLibrarySpec(),
            {"saveBookOk", "saveStudentOk"});
    }

    // ========================================
    // DEPTH 4+: Request/Loan Flow Tests
    // ========================================

    void test12_createRequest(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 12: Create Request Flow (Depth=3)",
            makeLibrarySpec(),
            {"saveBookOk", "saveStudentOk", "saveRequestOk"});
    }

    void test13_acceptRequest(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 13: Accept Request Flow (Depth=4)",
            makeLibrarySpec(),
            {"saveBookOk", "saveStudentOk", "saveRequestOk", "acceptRequestOk"});
    }

    void test14_fullBorrowReturn(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 14: Full Borrow/Return Lifecycle (Depth=5)",
            makeLibrarySpec(),
            {"saveBookOk", "saveStudentOk", "saveRequestOk", "acceptRequestOk", "returnBookOk"});
    }

    void test15_directLoan(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 15: Direct Loan Creation (Depth=3)",
            makeLibrarySpec(),
            {"saveBookOk", "saveStudentOk", "saveLoanOk"});
    }

    void test16_rejectRequest(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 16: Reject Request (Depth=4)",
            makeLibrarySpec(),
            {"saveBookOk", "saveStudentOk", "saveRequestOk", "deleteRequestOk"});
    }

    // ========================================
    // NEGATIVE TESTS (Expected UNSAT)
    // ========================================

    void test17_requestWithoutBook(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 17: Request Without Book (Should be UNSAT)",
            makeLibrarySpec(),
            {"saveStudentOk", "saveRequestOk"}); // No book - should fail
    }

    void test18_requestWithoutStudent(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 18: Request Without Student (Should be UNSAT)",
            makeLibrarySpec(),
            {"saveBookOk", "saveRequestOk"}); // No student - should fail
    }

    void test19_acceptWithoutRequest(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 19: Accept Without Request (Should be UNSAT)",
            makeLibrarySpec(),
            {"acceptRequestOk"}); // No request - should fail
    }

    void test20_returnWithoutLoan(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 20: Return Without Loan (Should be UNSAT)",
            makeLibrarySpec(),
            {"returnBookOk"}); // No loan - should fail
    }

    // ========================================
    // COMPLEX WORKFLOWS
    // ========================================

    void test21_multipleBooks(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 21: Multiple Books (Depth=5)",
            makeLibrarySpec(),
            {"saveBookOk", "saveBookOk", "saveBookOk", "getAllBooksOk", "getBookByCodeOk"});
    }

    void test22_multipleStudents(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 22: Multiple Students (Depth=5)",
            makeLibrarySpec(),
            {"saveStudentOk", "saveStudentOk", "saveStudentOk", "getAllStudentsOk", "getStudentByIdOk"});
    }

    void test23_multipleBorrowings(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 23: Multiple Borrowings (Depth=7)",
            makeLibrarySpec(),
            {"saveBookOk", "saveBookOk", "saveStudentOk",
             "saveRequestOk", "saveRequestOk",
             "acceptRequestOk", "acceptRequestOk"});
    }

    void test24_fullLibraryWorkflow(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 24: Full Library Workflow (Depth=8)",
            makeLibrarySpec(),
            {"saveBookOk", "saveBookOk",
             "saveStudentOk", "saveStudentOk",
             "saveRequestOk", "acceptRequestOk",
             "returnBookOk", "getAllLoansOk"});
    }

    void test25_complexScenario(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "Test 25: Complex Multi-User Scenario (Depth=10)",
            makeLibrarySpec(),
            {"saveBookOk", "saveBookOk", "saveBookOk",
             "saveStudentOk", "saveStudentOk",
             "saveRequestOk", "saveRequestOk",
             "acceptRequestOk",
             "returnBookOk",
             "getAllRequestsOk"});
    }
}

// ============================================
// E-COMMERCE COMPREHENSIVE TESTS (30 tests: 21 SAT, 9 UNSAT)
// ============================================

namespace EcommerceTests
{
    // ╔════════════════════════════════════════════════════════════╗
    // ║  SAT TESTS (21 tests = 70%)                                ║
    // ╚════════════════════════════════════════════════════════════╝

    // SAT-01: Register Buyer (Depth=1)
    void test01_registerBuyer(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 01: Register Buyer (Depth=1)",
            makeEcommerceSpec(),
            {"registerBuyerOk"});
    }

    // SAT-02: Register Seller (Depth=1)
    void test02_registerSeller(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 02: Register Seller (Depth=1)",
            makeEcommerceSpec(),
            {"registerSellerOk"});
    }

    // SAT-03: Browse Products - Public API (Depth=1)
    void test03_browseProducts(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 03: Browse All Products - Public (Depth=1)",
            makeEcommerceSpec(),
            {"getAllProductsOk"});
    }

    // SAT-04: Register Buyer → Login (Depth=2)
    void test04_buyerRegisterLogin(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 04: Register Buyer → Login (Depth=2)",
            makeEcommerceSpec(),
            {"registerBuyerOk", "loginBuyerOk"});
    }

    // SAT-05: Register Seller → Login (Depth=2)
    void test05_sellerRegisterLogin(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 05: Register Seller → Login (Depth=2)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk"});
    }

    // SAT-06: Seller Creates Product (Depth=3)
    void test06_sellerCreateProduct(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 06: Seller Creates Product (Depth=3)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk"});
    }

    // SAT-07: Seller Creates Multiple Products (Depth=5)
    void test07_sellerCreateMultipleProducts(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 07: Seller Creates 3 Products (Depth=5)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk",
             "createProductOk", "createProductOk", "createProductOk"});
    }

    // SAT-08: Seller Creates → Updates Product (Depth=4)
    void test08_sellerUpdateProduct(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 08: Seller Creates → Updates Product (Depth=4)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk", "updateProductOk"});
    }

    // SAT-09: Seller Creates → Deletes Product (Depth=4)
    void test09_sellerDeleteProduct(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 09: Seller Creates → Deletes Product (Depth=4)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk", "deleteProductOk"});
    }

    // SAT-10: Seller Views Own Inventory (Depth=4)
    void test10_sellerViewInventory(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 10: Seller Creates → Views Inventory (Depth=4)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk", "getSellerProductsOk"});
    }

    // SAT-11: Seller creates product → Buyer browses (Depth=6)
    void test11_multiUserBrowse(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 11: Seller Setup → Buyer Browses (Depth=6)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk", "getAllProductsOk"});
    }

    // SAT-12: Seller creates product → Buyer adds to cart (Depth=7)
    void test12_multiUserAddToCart(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 12: Seller Setup → Buyer Adds to Cart (Depth=7)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk"});
    }

    // SAT-13: Seller creates product → Buyer views cart (Depth=8)
    void test13_multiUserViewCart(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 13: Seller Setup → Buyer Adds & Views Cart (Depth=8)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "getCartOk"});
    }

    // SAT-14: Seller creates product → Buyer creates order (Depth=8)
    void test14_multiUserCreateOrder(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 14: Seller Setup → Buyer Creates Order (Depth=8)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk"});
    }

    // SAT-15: Full order flow → Buyer views orders (Depth=9)
    void test15_multiUserViewOrders(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 15: Full Order Flow → Buyer Views Orders (Depth=9)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk", "getBuyerOrdersOk"});
    }

    // SAT-16: Full order flow → Seller views orders (Depth=9)
    void test16_sellerViewsOrders(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 16: Full Order → Seller Views Orders (Depth=9)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk", "getSellerOrdersOk"});
    }

    // SAT-17: Full flow → Buyer creates review (Depth=9)
    void test17_multiUserCreateReview(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 17: Full Order → Buyer Creates Review (Depth=9)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk", "createReviewOk"});
    }

    // SAT-18: Complete E-Commerce Journey (Depth=12)
    void test18_completeEcommerceFlow(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 18: Complete E-Commerce Flow (Depth=12)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "getAllProductsOk", "addToCartOk", "addToCartOk",
             "createOrderOk", "getBuyerOrdersOk", "createReviewOk"});
    }

    // SAT-19: Multiple Orders by Same Buyer (Depth=12)
    void test19_multipleOrders(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 19: Buyer Places Multiple Orders (Depth=12)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk",
             "createProductOk", "createProductOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk",
             "addToCartOk", "createOrderOk",
             "getBuyerOrdersOk"});
    }

    // SAT-20: Seller Full Product Management (Depth=10)
    void test20_sellerFullManagement(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 20: Seller Full Product Management (Depth=10)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk",
             "createProductOk", "createProductOk", "createProductOk",
             "updateProductOk", "updateProductOk",
             "deleteProductOk",
             "getSellerProductsOk", "getSellerOrdersOk"});
    }

    // SAT-21: Deep E-Commerce Workflow (Depth=15)
    void test21_deepWorkflow(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[SAT] Test 21: Deep E-Commerce Workflow (Depth=15)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk",
             "createProductOk", "createProductOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "getAllProductsOk", "addToCartOk", "addToCartOk",
             "createOrderOk", "createReviewOk",
             "addToCartOk", "createOrderOk", "createReviewOk"});
    }

    // ╔════════════════════════════════════════════════════════════╗
    // ║  UNSAT TESTS (9 tests = 30%)                               ║
    // ╚════════════════════════════════════════════════════════════╝

    // UNSAT-01: Login without registration (Depth=1)
    void test22_loginWithoutRegister(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 22: Buyer Login Without Registration (Depth=1)",
            makeEcommerceSpec(),
            {"loginBuyerOk"});
    }

    // UNSAT-02: Seller login without registration (Depth=1)
    void test23_sellerLoginWithoutRegister(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 23: Seller Login Without Registration (Depth=1)",
            makeEcommerceSpec(),
            {"loginSellerOk"});
    }

    // UNSAT-03: Duplicate registration (Depth=2)
    void test24_duplicateRegistration(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 24: Duplicate Buyer Registration (Depth=2)",
            makeEcommerceSpec(),
            {"registerBuyerOk", "registerBuyerOk"});
    }

    // UNSAT-04: Buyer tries to create product (Depth=3)
    void test25_buyerCannotCreateProduct(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 25: Buyer Cannot Create Product (Depth=3)",
            makeEcommerceSpec(),
            {"registerBuyerOk", "loginBuyerOk", "createProductOk"});
    }

    // UNSAT-05: Seller tries to add to cart (Depth=3)
    void test26_sellerCannotAddToCart(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 26: Seller Cannot Add to Cart (Depth=3)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "addToCartOk"});
    }

    // UNSAT-06: Seller tries to create order (Depth=4)
    void test27_sellerCannotCreateOrder(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 27: Seller Cannot Create Order (Depth=4)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk", "createOrderOk"});
    }

    // UNSAT-07: Buyer adds to cart without product existing (Depth=3)
    void test28_addToCartNoProduct(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 28: Add to Cart - No Product Exists (Depth=3)",
            makeEcommerceSpec(),
            {"registerBuyerOk", "loginBuyerOk", "addToCartOk"});
    }

    // UNSAT-08: Create order without items in cart (Depth=6)
    void test29_createOrderEmptyCart(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 29: Create Order - Empty Cart (Depth=6)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "createOrderOk"});
    }

    // UNSAT-09: Create review without placing order (Depth=7)
    void test30_reviewWithoutOrder(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[UNSAT] Test 30: Create Review - No Order Placed (Depth=7)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createReviewOk"});
    }
}

// ============================================
// BUG DETECTION TESTS (8 Mutation Bugs)
// Backend: http://localhost:5002
// ============================================

namespace BugTests
{
    // B1: auth.js:73 — register returns 200 instead of 201
    // Spec postcondition checks STATE (U', Roles'), NOT the return status code.
    // Factory accepts any 2xx → state IS updated → test currently PASSES.
    // Detection requires adding _result==201 to the register postcondition.
    void bugB1_registerStatusCode(TestExecutor &executor)
    {
        executor.runTest(
            "[BUG B1] POST /api/auth/register returns 200 instead of 201 (Depth=1)",
            makeRestaurantSpec(),
            {"registerCustomerOk"});
    }

    // B2: restaurants.js:31 — createRestaurant returns 200 instead of 201
    // Factory strictly checks resp.statusCode==201. With bug: returns Num(200).
    // Spec POST asserts: _result in dom(R') — 200 is NOT a valid restaurantId → ASSERTION FAILS.
    void bugB2_createRestaurantStatusCode(TestExecutor &executor)
    {
        executor.runTest(
            "[BUG B2] POST /api/restaurants returns 200 instead of 201 (Depth=3)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk"});
    }

    // B3: menu.js:41 — addMenuItem returns 200 instead of 201
    // Factory strictly checks resp.statusCode==201. With bug: returns Num(200).
    // Spec POST asserts: _result in dom(M') — FAILS (cascade: B2 detected first at step 3).
    void bugB3_addMenuItemStatusCode(TestExecutor &executor)
    {
        executor.runTest(
            "[BUG B3] POST /api/menu returns 200 instead of 201 (Depth=4, B2 cascade expected)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk"});
    }

    // B4: orders.js:41 — finalAmount missing delivery fee
    // totalAmount + tax instead of totalAmount + deliveryFee + tax.
    // Detection: placeOrderOk returns orderId, then checkOrderAmountOk calls
    // /api/test/check_order_amount/:orderId which verifies finalAmount server-side.
    // With B4: finalAmount is wrong → endpoint returns 400 → _result=400 ≠ 1 → ASSERTION FAILS.
    void bugB4_orderAmountMissingDeliveryFee(TestExecutor &executor)
    {
        executor.runTest(
            "[BUG B4] Order finalAmount missing deliveryFee — checkOrderAmountOk (Depth=11)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartRestaurantOk", "placeOrderOk",
             "checkOrderAmountOk"});
    }

    // B5: orders.js:82 — Cart.findOneAndDelete commented out (cart not cleared)
    // After placing order, backend no longer deletes the cart document.
    // Spec POST asserts: customerEmail not_in dom(C') — backend C still has the cart → ASSERTION FAILS.
    // (Cascade: B2 fails first at step 3 before reaching placeOrderOk)
    void bugB5_cartNotClearedAfterOrder(TestExecutor &executor)
    {
        executor.runTest(
            "[BUG B5] Cart not cleared after placing order (Depth=10, B2 cascade expected)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartRestaurantOk", "placeOrderOk"});
    }

    // B6: cart.js:129 — quantity <= 0 check changed to quantity < 0
    // PUT /api/cart/:itemId with quantity:0 now succeeds (200) instead of returning 400.
    // Detection: addToCartQuantityZeroErr calls PUT /api/cart/:itemId with quantity=0.
    // Spec asserts _result==400. With B6: backend returns 200 → 200≠400 → ASSERTION FAILS.
    void bugB6_cartQuantityBoundary(TestExecutor &executor)
    {
        executor.runTest(
            "[BUG B6] Cart accepts quantity=0 — addToCartQuantityZeroErr (Depth=10)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartRestaurantOk",
             "addToCartQuantityZeroErr"});
    }

    // B7: reviews.js:40 — status !== 'delivered' changed to status !== 'cancelled'
    // Only cancelled orders can now be reviewed; delivered orders get 400.
    // Detection requires a test that brings the order to 'delivered' status then calls leaveReviewOk.
    // Spec POST asserts: _result in dom(Rev') — with bug, leaveReview returns 400 → ASSERTION FAILS.
    // (Cascade: B2 fails first at step 3 in this full workflow)
    void bugB7_reviewGuardInverted(TestExecutor &executor)
    {
        executor.runTest(
            "[BUG B7] Review rejected for delivered orders — full delivery flow (Depth=18, B2 cascade expected)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartRestaurantOk", "placeOrderOk",
             "updateOrderStatusOwnerOk",  // pending -> accepted
             "updateOrderStatusOwnerOk",  // accepted -> preparing
             "updateOrderStatusOwnerOk",  // preparing -> out_for_delivery
             "registerAgentOk", "loginAgentOk", "assignOrderOk",
             "updateOrderStatusAgentOk",  // out_for_delivery -> delivered
             "leaveReviewOk"});
    }

    // B8: models/Cart.js:43 — total + (item.price * item.quantity) changed to total + item.price
    // Cart totalAmount ignores quantity — 3 items at Rs100 each shows Rs100 instead of Rs300.
    // Detection: checkCartTotalOk calls /api/test/check_cart_total/:email which
    // computes expected=sum(price*qty) and compares with stored totalAmount.
    // With B8: totalAmount is wrong → endpoint returns 400 → _result=400≠1 → ASSERTION FAILS.
    void bugB8_cartTotalIgnoresQuantity(TestExecutor &executor)
    {
        executor.runTest(
            "[BUG B8] Cart totalAmount ignores quantity — checkCartTotalOk (Depth=10)",
            makeRestaurantSpec(),
            {"registerOwnerOk", "loginOwnerOk", "createRestaurantOk", "addMenuItemOk",
             "registerCustomerOk", "loginCustomerOk", "browseRestaurantsOk", "viewMenuOk",
             "addToCartRestaurantOk",
             "checkCartTotalOk"});
    }
}

// ============================================
// E-COMMERCE BUG DETECTION TESTS (8 Mutation Bugs)
// Backend: http://localhost:3000
// ============================================

namespace EcomBugTests
{
    // EB1: authController.js — register returns 200 instead of 201
    // registerBuyerOk postcondition now includes _result == 201.
    // With bug: factory returns Num(200) → 200 ≠ 201 → ASSERTION FAILS.
    void bugEB1_registerStatusCode(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[ECOM BUG EB1] POST /api/auth/register returns 200 instead of 201 (Depth=1)",
            makeEcommerceSpec(),
            {"registerBuyerOk"});
    }

    // EB2: productController.js — createProduct returns 200 instead of 201
    // Factory checks resp.statusCode == 201; with bug returns "" → in(_result, dom(P')) fails.
    void bugEB2_createProductStatusCode(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[ECOM BUG EB2] POST /api/products returns 200 instead of 201 (Depth=3)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk"});
    }

    // EB3: orderController.js — createOrder returns 200 instead of 201
    // Factory checks resp.statusCode == 201; with bug returns "" → in(_result, dom(O')) fails.
    void bugEB3_createOrderStatusCode(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[ECOM BUG EB3] POST /api/orders returns 200 instead of 201 (Depth=7)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk"});
    }

    // EB4: orderController.js — totalAmount ignores item.quantity
    // itemTotal = item.product.price instead of item.product.price * item.quantity.
    // checkOrderTotalOk calls /api/test/check_order_total/:orderId which recomputes
    // expected = sum(items[].price * items[].quantity) and compares with totalAmount.
    // With EB4: wrong totalAmount → endpoint returns 400 → _result = 400 ≠ 1 → ASSERTION FAILS.
    void bugEB4_orderTotalIgnoresQuantity(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[ECOM BUG EB4] Order totalAmount ignores quantity — checkOrderTotalOk (Depth=8)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk",
             "checkOrderTotalOk"});
    }

    // EB5: orderController.js — Cart.findOneAndDelete commented out (cart not cleared)
    // createOrderOk postcondition includes: buyerEmail not_in dom(C').
    // With EB5: cart still exists after order → get_C returns cart → not_in fails → ASSERTION FAILS.
    void bugEB5_cartNotClearedAfterOrder(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[ECOM BUG EB5] Cart not cleared after order — createOrderOk (Depth=7)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk"});
    }

    // EB6: cartController.js — quantity > product.quantity changed to quantity >= product.quantity
    // addToCartMaxStockOk factory fetches current stock and adds EXACTLY that quantity.
    // Correct backend (>): exact-stock add is allowed → 200.
    // Buggy backend (>=): exact-stock add rejected → 400 ≠ 200 → ASSERTION FAILS.
    void bugEB6_cartBoundaryCondition(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[ECOM BUG EB6] Cart rejects exact-stock quantity — addToCartMaxStockOk (Depth=6)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartMaxStockOk"});
    }

    // EB7: reviewController.js — if (!order) changed to if (order) (inverted guard)
    // With bug: review returns 400 for all valid-order purchases.
    // createReviewOk postcondition: in(_result, dom(Rev')).
    // Factory returns "" on non-201 → "" not in dom(Rev') → ASSERTION FAILS.
    void bugEB7_reviewGuardInverted(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[ECOM BUG EB7] Review guard inverted — createReviewOk (Depth=8)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "addToCartOk", "createOrderOk", "createReviewOk"});
    }

    // EB8: productController.js — deleteProduct removes seller ownership check
    // deleteProductByBuyerErr uses buyer's token to delete a seller's product.
    // Correct: { _id, seller } query fails for buyer → 404.
    // Buggy: { _id } only → product deleted → 200 ≠ 404 → ASSERTION FAILS.
    void bugEB8_deleteProductAuthBypass(EcommerceTestExecutor &executor)
    {
        executor.runTest(
            "[ECOM BUG EB8] Product delete missing seller auth — deleteProductByBuyerErr (Depth=6)",
            makeEcommerceSpec(),
            {"registerSellerOk", "loginSellerOk", "createProductOk",
             "registerBuyerOk", "loginBuyerOk",
             "deleteProductByBuyerErr"});
    }
}

// ============================================
// LIBRARY BUG DETECTION TESTS (8 Mutation Bugs)
// Backend: http://localhost:8080
// ============================================

namespace LibraryBugTests
{
    // LB1: RequestController.java:167 — requestService.deleteRequestbyId() commented out
    // accept() creates loan but never deletes the original request from DB.
    // acceptRequestOk postcondition: in(_result, dom(Loans')) AND not_in(requestId, dom(Req')).
    // With LB1: request still in DB → get_Req() returns it → not_in fails → ASSERTION FAILS.
    void bugLB1_requestNotDeletedAfterAccept(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "[LIB BUG LB1] Request not deleted after accept — acceptRequestOk (Depth=4)",
            makeLibrarySpec(),
            {"saveBookOk", "saveStudentOk", "saveRequestOk", "acceptRequestOk"});
    }

    // LB2: BookService.java:43 — deleteBookByCode returns book without calling deleteById
    // deleteBook endpoint returns the book (HTTP 200) but never removes it from DB.
    // deleteBookOk postcondition: not_in(bookCode, dom(B')).
    // With LB2: book still in DB → get_B() returns it → not_in fails → ASSERTION FAILS.
    void bugLB2_bookNotDeleted(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "[LIB BUG LB2] Book not deleted — deleteBookOk (Depth=2)",
            makeLibrarySpec(),
            {"saveBookOk", "deleteBookOk"});
    }

    // LB3: RequestController.java:147 — doesRequestOverlap guard inverted
    // With LB3: accept throws UnavailableForGivenDatesException even when no overlap exists.
    // acceptRequestOk postcondition: in(_result, dom(Loans')).
    // Bug makes POST /bookStudent/accept throw 500 → factory returns Num(500).
    // AcceptRequestFunc returns Num(500) (non 200/201) → not a valid loanId → in fails → ASSERTION FAILS.
    void bugLB3_overlapCheckInverted(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "[LIB BUG LB3] Overlap check inverted — accept rejects valid requests (Depth=4)",
            makeLibrarySpec(),
            {"saveBookOk", "saveStudentOk", "saveRequestOk", "acceptRequestOk"});
    }

    // LB4: BookStudentService.java:70 — deleteById returns findBookStudentById instead of deleteBookStudentById
    // returnBook endpoint returns the loan (HTTP 200) but never removes it from DB.
    // returnBookOk postcondition: not_in(loanId, dom(Loans')).
    // With LB4: loan still in DB → get_Loans() returns it → not_in fails → ASSERTION FAILS.
    void bugLB4_loanNotDeleted(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "[LIB BUG LB4] Loan not deleted on return — returnBookOk (Depth=5)",
            makeLibrarySpec(),
            {"saveBookOk", "saveStudentOk", "saveRequestOk", "acceptRequestOk", "returnBookOk"});
    }

    // LB5: StudentService.java:50 — deleteStudentById replaced with getStudentById
    // deleteStudent endpoint returns student (HTTP 200) but never removes from DB.
    // deleteStudentOk postcondition: not_in(studentId, dom(S')).
    // With LB5: student still in DB → get_S() returns it → not_in fails → ASSERTION FAILS.
    void bugLB5_studentNotDeleted(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "[LIB BUG LB5] Student not deleted — deleteStudentOk (Depth=2)",
            makeLibrarySpec(),
            {"saveStudentOk", "deleteStudentOk"});
    }

    // LB6: BookService.java — updateBook deletes instead of updating
    // After updateBook call, book is gone from DB.
    // updateBookOk postcondition: bookCode in dom(B').
    // With LB6: book deleted → get_B() doesn't return it → in fails → ASSERTION FAILS.
    void bugLB6_updateDeletesBook(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "[LIB BUG LB6] Update deletes book instead of updating (Depth=2)",
            makeLibrarySpec(),
            {"saveBookOk", "updateBookOk"});
    }

    // LB7: StudentService.java — saveStudent returns student without saving to DB
    // saveStudent returns student object with id=0 (not persisted).
    // saveStudentOk postcondition: in(_result, dom(S')).
    // SaveStudentFunc gets id=0 → returns String("0") → get_S() fetches DB → "0" not there → ASSERTION FAILS.
    void bugLB7_studentNotSaved(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "[LIB BUG LB7] Student not saved to DB — saveStudentOk (Depth=1)",
            makeLibrarySpec(),
            {"saveStudentOk"});
    }

    // LB8: BookService.java — addBook returns new Book() without saving to DB
    // saveBook returns empty Book with bookCode=0 (not persisted).
    // saveBookOk postcondition: in(_result, dom(B')).
    // SaveBookFunc gets bookCode=0 → returns String("0") → get_B() fetches DB → "0" not there → ASSERTION FAILS.
    void bugLB8_bookNotSaved(LibraryTestExecutor &executor)
    {
        executor.runTest(
            "[LIB BUG LB8] Book not saved to DB — saveBookOk (Depth=1)",
            makeLibrarySpec(),
            {"saveBookOk"});
    }
}

// ============================================
// MAIN FUNCTION - TEST SELECTION
// ============================================

int main(int argc, char* argv[])
{
    // backend = "restaurant" | "ecommerce" | "library"  (default: library)
    string backend = (argc > 1) ? string(argv[1]) : "library";

    try
    {
        // ========================================
        // CONFIGURATION
        // ========================================

        // Choose test mode
        // TestMode mode = TestMode::REWRITE_ONLY;
        // TestMode mode = TestMode::ORIGINAL;
        TestMode mode = TestMode::FULL_PIPELINE; // Needs backend running!

        // ========================================
        // BACKEND-SELECTED BUG DETECTION
        // ========================================
        if (backend == "restaurant")
        {
            string restUrl = "http://localhost:5002";
            TestExecutor executor(mode, restUrl);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  TESTGEN - RESTAURANT TEST SUITE       ║" << endl;
            cout << "║  Total Tests: 25                       ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
            RestaurantTests::test01_registerLogin(executor);
            RestaurantTests::test02_loginFailure(executor);
            RestaurantTests::test03_browseOnly(executor);
            RestaurantTests::test04_Login(executor);
            RestaurantTests::test05_registerOwnerAndLogin(executor);
            RestaurantTests::test06_registerAgentAndLogin(executor);
            RestaurantTests::test07_loginBrowseView(executor);
            RestaurantTests::test08_loginAndAddToCart(executor);
            RestaurantTests::test09_loginAndReview(executor);
            RestaurantTests::test10_reviewWithoutLogin(executor);
            RestaurantTests::test11_fullCustomerOrder(executor);
            RestaurantTests::test12_ownerCreateRestaurant(executor);
            RestaurantTests::test13_cartWithoutItems(executor);
            RestaurantTests::test14_customerFullWorkflow(executor);
            RestaurantTests::test15_ownerFullSetup(executor);
            RestaurantTests::test16_agentAssignOrder(executor);
            RestaurantTests::test17_ownerManageOrder(executor);
            RestaurantTests::test18_multipleCartAdditions(executor);
            RestaurantTests::test19_wrongRoleAccess(executor);
            RestaurantTests::test20_fullLifecycle(executor);
            RestaurantTests::test21_ownerCompleteFlow(executor);
            RestaurantTests::test22_complexOrderManagement(executor);
            RestaurantTests::test23_invalidSequence(executor);
            RestaurantTests::test24_deepWorkflow(executor);
            RestaurantTests::test25_registerCustomerDuplicate(executor);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  ALL RESTAURANT TESTS COMPLETE         ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
        }
        else if (backend == "ecommerce")
        {
            string ecomUrl = "http://localhost:3000";
            EcommerceTestExecutor ecomExecutor(mode, ecomUrl);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  TESTGEN - E-COMMERCE TEST SUITE       ║" << endl;
            cout << "║  Total Tests: 30 (21 SAT, 9 UNSAT)     ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
            EcommerceTests::test01_registerBuyer(ecomExecutor);
            EcommerceTests::test02_registerSeller(ecomExecutor);
            EcommerceTests::test03_browseProducts(ecomExecutor);
            EcommerceTests::test04_buyerRegisterLogin(ecomExecutor);
            EcommerceTests::test05_sellerRegisterLogin(ecomExecutor);
            EcommerceTests::test06_sellerCreateProduct(ecomExecutor);
            EcommerceTests::test07_sellerCreateMultipleProducts(ecomExecutor);
            EcommerceTests::test08_sellerUpdateProduct(ecomExecutor);
            EcommerceTests::test09_sellerDeleteProduct(ecomExecutor);
            EcommerceTests::test10_sellerViewInventory(ecomExecutor);
            EcommerceTests::test11_multiUserBrowse(ecomExecutor);
            EcommerceTests::test12_multiUserAddToCart(ecomExecutor);
            EcommerceTests::test13_multiUserViewCart(ecomExecutor);
            EcommerceTests::test14_multiUserCreateOrder(ecomExecutor);
            EcommerceTests::test15_multiUserViewOrders(ecomExecutor);
            EcommerceTests::test16_sellerViewsOrders(ecomExecutor);
            EcommerceTests::test17_multiUserCreateReview(ecomExecutor);
            EcommerceTests::test18_completeEcommerceFlow(ecomExecutor);
            EcommerceTests::test19_multipleOrders(ecomExecutor);
            EcommerceTests::test20_sellerFullManagement(ecomExecutor);
            EcommerceTests::test21_deepWorkflow(ecomExecutor);
            EcommerceTests::test22_loginWithoutRegister(ecomExecutor);
            EcommerceTests::test23_sellerLoginWithoutRegister(ecomExecutor);
            EcommerceTests::test24_duplicateRegistration(ecomExecutor);
            EcommerceTests::test25_buyerCannotCreateProduct(ecomExecutor);
            EcommerceTests::test26_sellerCannotAddToCart(ecomExecutor);
            EcommerceTests::test27_sellerCannotCreateOrder(ecomExecutor);
            EcommerceTests::test28_addToCartNoProduct(ecomExecutor);
            EcommerceTests::test29_createOrderEmptyCart(ecomExecutor);
            EcommerceTests::test30_reviewWithoutOrder(ecomExecutor);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  ALL ECOMMERCE TESTS COMPLETE          ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
        }
        else if (backend == "ghostsocket")
        {
            string gsUrl = "http://localhost:4002";
            GhostSocketTestExecutor gsExecutor(mode, gsUrl);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  TESTGEN - GHOSTSOCKET TEST SUITE      ║" << endl;
            cout << "║  Total Tests: 25 (21 SAT, 4 UNSAT)    ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
            GhostSocketTests::test01_registerUser(gsExecutor);
            GhostSocketTests::test02_registerTwoUsers(gsExecutor);
            GhostSocketTests::test03_registerDevice(gsExecutor);
            GhostSocketTests::test04_getMyDevices(gsExecutor);
            GhostSocketTests::test05_getDeviceInfo(gsExecutor);
            GhostSocketTests::test06_createSession(gsExecutor);
            GhostSocketTests::test07_joinSession(gsExecutor);
            GhostSocketTests::test08_getSessions(gsExecutor);
            GhostSocketTests::test09_terminateSession(gsExecutor);
            GhostSocketTests::test10_deleteDevice(gsExecutor);
            GhostSocketTests::test11_getOtherDevices(gsExecutor);
            GhostSocketTests::test12_updatePermissions(gsExecutor);
            GhostSocketTests::test13_deviceInfoForbidden(gsExecutor);
            GhostSocketTests::test14_createSessionForbidden(gsExecutor);
            GhostSocketTests::test15_joinSessionNotFound(gsExecutor);
            GhostSocketTests::test16_terminateSessionForbidden(gsExecutor);
            GhostSocketTests::test17_fullSessionLifecycle(gsExecutor);
            GhostSocketTests::test18_devicesAndSession(gsExecutor);
            GhostSocketTests::test19_deviceInfoAndSession(gsExecutor);
            GhostSocketTests::test20_joinAndUpdatePermissions(gsExecutor);
            GhostSocketTests::test21_sessionListAndTerminate(gsExecutor);
            GhostSocketTests::test22_createSessionNoDevice(gsExecutor);
            GhostSocketTests::test23_joinSessionNoSession(gsExecutor);
            GhostSocketTests::test24_terminateNoSession(gsExecutor);
            GhostSocketTests::test25_deviceInfoNoDevice(gsExecutor);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  ALL GHOSTSOCKET TESTS COMPLETE        ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
        }
        else if (backend == "serveez")
        {
            string svUrl = "http://localhost:8083";
            ServeezTestExecutor svExecutor(mode, svUrl);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  TESTGEN - SERVEEZ TEST SUITE          ║" << endl;
            cout << "║  Total Tests: 25 (21 SAT, 4 UNSAT)    ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
            ServeezTests::test01_registerUser(svExecutor);
            ServeezTests::test02_registerProvider(svExecutor);
            ServeezTests::test03_registerAdmin(svExecutor);
            ServeezTests::test04_createCategory(svExecutor);
            ServeezTests::test05_createListing(svExecutor);
            ServeezTests::test06_getListings(svExecutor);
            ServeezTests::test07_getListingById(svExecutor);
            ServeezTests::test08_createBooking(svExecutor);
            ServeezTests::test09_getMyBookings(svExecutor);
            ServeezTests::test10_confirmBooking(svExecutor);
            ServeezTests::test11_completeBooking(svExecutor);
            ServeezTests::test12_createReview(svExecutor);
            ServeezTests::test13_getListingReviews(svExecutor);
            ServeezTests::test14_cancelBooking(svExecutor);
            ServeezTests::test15_createListingUnauth(svExecutor);
            ServeezTests::test16_createBookingAsProvider(svExecutor);
            ServeezTests::test17_twoListings(svExecutor);
            ServeezTests::test18_fullLifecycle(svExecutor);
            ServeezTests::test19_multipleBookings(svExecutor);
            ServeezTests::test20_bookAndCancel(svExecutor);
            ServeezTests::test21_getListingThenBook(svExecutor);
            ServeezTests::test22_createCategoryNoAdmin(svExecutor);
            ServeezTests::test23_createListingNoProvider(svExecutor);
            ServeezTests::test24_createBookingNoUser(svExecutor);
            ServeezTests::test25_confirmBookingNone(svExecutor);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  ALL SERVEEZ TESTS COMPLETE            ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
        }
        else if (backend == "tripvault")
        {
            string tvUrl = "http://localhost:4001";
            TripVaultTestExecutor tvExecutor(mode, tvUrl);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  TESTGEN - TRIPVAULT TEST SUITE        ║" << endl;
            cout << "║  Total Tests: 25 (21 SAT, 4 UNSAT)    ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
            TripVaultTests::test01_registerLogin(tvExecutor);
            TripVaultTests::test02_createTrip(tvExecutor);
            TripVaultTests::test03_getUserTrips(tvExecutor);
            TripVaultTests::test04_updateTrip(tvExecutor);
            TripVaultTests::test05_deleteTrip(tvExecutor);
            TripVaultTests::test06_addMember(tvExecutor);
            TripVaultTests::test07_joinByInvite(tvExecutor);
            TripVaultTests::test08_createExpense(tvExecutor);
            TripVaultTests::test09_getExpenses(tvExecutor);
            TripVaultTests::test10_deleteExpense(tvExecutor);
            TripVaultTests::test11_createProposal(tvExecutor);
            TripVaultTests::test12_getProposals(tvExecutor);
            TripVaultTests::test13_deleteProposal(tvExecutor);
            TripVaultTests::test14_multipleExpenses(tvExecutor);
            TripVaultTests::test15_multipleTrips(tvExecutor);
            TripVaultTests::test16_expenseAndProposal(tvExecutor);
            TripVaultTests::test17_memberCreatesExpense(tvExecutor);
            TripVaultTests::test18_fullTripLifecycle(tvExecutor);
            TripVaultTests::test19_joinAndCreateExpense(tvExecutor);
            TripVaultTests::test20_deleteAndRecreateExpense(tvExecutor);
            TripVaultTests::test21_multipleProposals(tvExecutor);
            TripVaultTests::test22_loginWithoutRegister(tvExecutor);
            TripVaultTests::test23_createTripWithoutLogin(tvExecutor);
            TripVaultTests::test24_deleteExpenseWithoutAuth(tvExecutor);
            TripVaultTests::test25_deleteTripWithoutAuth(tvExecutor);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  ALL TRIPVAULT TESTS COMPLETE          ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
        }
        else // library (default)
        {
            string libUrl = "http://localhost:8080";
            LibraryTestExecutor libExecutor(mode, libUrl);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  TESTGEN - LIBRARY TEST SUITE          ║" << endl;
            cout << "║  Total Tests: 25                       ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
            LibraryTests::test01_getAllBooks(libExecutor);
            LibraryTests::test02_getAllStudents(libExecutor);
            LibraryTests::test03_saveBook(libExecutor);
            LibraryTests::test04_saveStudent(libExecutor);
            LibraryTests::test05_saveAndGetBook(libExecutor);
            LibraryTests::test06_saveAndGetStudent(libExecutor);
            LibraryTests::test07_saveBookTwice(libExecutor);
            LibraryTests::test08_getBookNotFound(libExecutor);
            LibraryTests::test09_bookCRUD(libExecutor);
            LibraryTests::test10_studentCRUD(libExecutor);
            LibraryTests::test11_createBookAndStudent(libExecutor);
            LibraryTests::test12_createRequest(libExecutor);
            LibraryTests::test13_acceptRequest(libExecutor);
            LibraryTests::test14_fullBorrowReturn(libExecutor);
            LibraryTests::test15_directLoan(libExecutor);
            LibraryTests::test16_rejectRequest(libExecutor);
            LibraryTests::test17_requestWithoutBook(libExecutor);
            LibraryTests::test18_requestWithoutStudent(libExecutor);
            LibraryTests::test19_acceptWithoutRequest(libExecutor);
            LibraryTests::test20_returnWithoutLoan(libExecutor);
            LibraryTests::test21_multipleBooks(libExecutor);
            LibraryTests::test22_multipleStudents(libExecutor);
            LibraryTests::test23_multipleBorrowings(libExecutor);
            LibraryTests::test24_fullLibraryWorkflow(libExecutor);
            LibraryTests::test25_complexScenario(libExecutor);
            cout << "\n╔════════════════════════════════════════╗" << endl;
            cout << "║  ALL LIBRARY TESTS COMPLETE            ║" << endl;
            cout << "╚════════════════════════════════════════╝\n" << endl;
        }

        // ---- Restaurant Bug Tests (commented out) ----
        // string backendUrl = "http://localhost:5002";
        // TestExecutor executor(mode, backendUrl);
        // BugTests::bugB1_registerStatusCode(executor);
        // BugTests::bugB2_createRestaurantStatusCode(executor);
        // BugTests::bugB3_addMenuItemStatusCode(executor);
        // BugTests::bugB4_orderAmountMissingDeliveryFee(executor);
        // BugTests::bugB5_cartNotClearedAfterOrder(executor);
        // BugTests::bugB6_cartQuantityBoundary(executor);
        // BugTests::bugB7_reviewGuardInverted(executor);
        // BugTests::bugB8_cartTotalIgnoresQuantity(executor);

        // ---- Original 25 Restaurant Tests (commented out) ----
        // cout << "\n╔════════════════════════════════════════╗" << endl; // for restaurant
        // cout << "║  TESTGEN - RESTAURANT TEST SUITE      ║" << endl; // for restaurant
        // cout << "║  Total Tests: 25                       ║" << endl; // for restaurant
        // cout << "╚════════════════════════════════════════╝\n"        // for restaurant
        //     << endl; // for restaurant
        // cout << "\n=== DEPTH TESTS ===" << endl; // for restaurant
        // RestaurantTests::test01_registerLogin(executor);
        // RestaurantTests::test02_loginFailure(executor);
        // RestaurantTests::test03_browseOnly(executor);

        // RestaurantTests::test04_Login(executor);
        // RestaurantTests::test05_registerOwnerAndLogin(executor);
        //  RestaurantTests::test06_registerAgentAndLogin(executor);

        // RestaurantTests::test07_loginBrowseView(executor);
        // RestaurantTests::test08_loginAndAddToCart(executor);
        // RestaurantTests::test09_loginAndReview(executor);
        // RestaurantTests::test10_reviewWithoutLogin(executor);

        // RestaurantTests::test11_fullCustomerOrder(executor);
        // RestaurantTests::test12_ownerCreateRestaurant(executor);
        // RestaurantTests::test13_cartWithoutItems(executor);

         // cout << "\n=== DEPTH 5 TESTS ===" << endl;
        // RestaurantTests::test14_customerFullWorkflow(executor);
        // RestaurantTests::test15_ownerFullSetup(executor);
        // RestaurantTests::test16_agentAssignOrder(executor);

         // cout << "\n=== DEPTH 6 TESTS ===" << endl;
        // RestaurantTests::test17_ownerManageOrder(executor);
        // RestaurantTests::test18_multipleCartAdditions(executor);
        // RestaurantTests::test19_wrongRoleAccess(executor);

         // cout << "\n=== DEPTH 7 TESTS ===" << endl;
        // RestaurantTests::test20_fullLifecycle(executor);
        // RestaurantTests::test21_ownerCompleteFlow(executor);

         // cout << "\n===More TESTS ===" << endl;
         // RestaurantTests::test22_complexOrderManagement(executor);
         // RestaurantTests::test23_invalidSequence(executor);
         // RestaurantTests::test24_deepWorkflow(executor);
         // RestaurantTests::test25_registerCustomerDuplicate(executor);

         // ========================================
         // E-COMMERCE TESTS
         // ========================================

         // cout << "\n╔════════════════════════════════════════╗" << endl; //uncomment for ecom
         // cout << "║  TESTGEN - E-COMMERCE TEST SUITE       ║" << endl; //uncomment for ecom
         // cout << "║  Total Tests: 30 (21 SAT, 9 UNSAT)     ║" << endl; //uncomment for ecom
         // cout << "╚════════════════════════════════════════╝\n" //uncomment for ecom
         //      << endl; //uncomment for ecom

         // E-commerce backend URL
         // string ecommerceBackendUrl = "http://localhost:3000"; //uncomment for ecom
         // EcommerceTestExecutor ecommerceExecutor(mode, ecommerceBackendUrl); //uncomment for ecom

         // === SAT TESTS ===
         // EcommerceTests::test01_registerBuyer(ecommerceExecutor);
         // EcommerceTests::test02_registerSeller(ecommerceExecutor);
         // EcommerceTests::test03_browseProducts(ecommerceExecutor);
         // EcommerceTests::test04_buyerRegisterLogin(ecommerceExecutor);
         // EcommerceTests::test05_sellerRegisterLogin(ecommerceExecutor);
         // EcommerceTests::test06_sellerCreateProduct(ecommerceExecutor);
         // EcommerceTests::test07_sellerCreateMultipleProducts(ecommerceExecutor);
         // EcommerceTests::test08_sellerUpdateProduct(ecommerceExecutor);
         // EcommerceTests::test09_sellerDeleteProduct(ecommerceExecutor);
         // EcommerceTests::test10_sellerViewInventory(ecommerceExecutor);
         // EcommerceTests::test11_multiUserBrowse(ecommerceExecutor);
         // EcommerceTests::test12_multiUserAddToCart(ecommerceExecutor);
         // EcommerceTests::test13_multiUserViewCart(ecommerceExecutor);
         // EcommerceTests::test14_multiUserCreateOrder(ecommerceExecutor);
         // EcommerceTests::test15_multiUserViewOrders(ecommerceExecutor);
         // EcommerceTests::test16_sellerViewsOrders(ecommerceExecutor);
         // EcommerceTests::test17_multiUserCreateReview(ecommerceExecutor);
         // EcommerceTests::test18_completeEcommerceFlow(ecommerceExecutor);
         // EcommerceTests::test19_multipleOrders(ecommerceExecutor);
         // EcommerceTests::test20_sellerFullManagement(ecommerceExecutor);
         // EcommerceTests::test21_deepWorkflow(ecommerceExecutor);

         // === UNSAT TESTS ===
         // EcommerceTests::test22_loginWithoutRegister(ecommerceExecutor);
         // EcommerceTests::test23_sellerLoginWithoutRegister(ecommerceExecutor);
         // EcommerceTests::test24_duplicateRegistration(ecommerceExecutor);
         // EcommerceTests::test25_buyerCannotCreateProduct(ecommerceExecutor);
         // EcommerceTests::test26_sellerCannotAddToCart(ecommerceExecutor);
         // EcommerceTests::test27_sellerCannotCreateOrder(ecommerceExecutor);
         // EcommerceTests::test28_addToCartNoProduct(ecommerceExecutor);
         // EcommerceTests::test29_createOrderEmptyCart(ecommerceExecutor);
         // EcommerceTests::test30_reviewWithoutOrder(ecommerceExecutor);

         // ========================================
         // CONFIGURATION
         // ========================================

         // Choose test mode
         // TestMode mode = TestMode::REWRITE_ONLY;
         // TestMode mode = TestMode::ORIGINAL;
         // TestMode mode = TestMode::FULL_PIPELINE; // Needs backend running!

         // Backend URL - Spring Boot default port
         // string backendUrl = "http://localhost:8080";

         // LibraryTestExecutor executor(mode, backendUrl);

         // ========================================
         // RUN TESTS
         // ========================================

         // cout << "\n╔════════════════════════════════════════╗" << endl;
         // cout << "║  TESTGEN - LIBRARY TEST SUITE          ║" << endl;
         // cout << "║  Total Tests: 25                       ║" << endl;
         // cout << "╚════════════════════════════════════════╝\n"
         //     << endl;

         // ========== BASIC TESTS ==========
         // cout << "\n=== BASIC SINGLE OPERATION TESTS ===" << endl;
         // LibraryTests::test01_getAllBooks(executor);
         // LibraryTests::test02_getAllStudents(executor);
         // LibraryTests::test03_saveBook(executor);
         // LibraryTests::test04_saveStudent(executor);

         // ========== DEPTH 2 TESTS ==========
         // cout << "\n=== DEPTH 2 TESTS ===" << endl;
         // LibraryTests::test05_saveAndGetBook(executor);
         // LibraryTests::test06_saveAndGetStudent(executor);
         // LibraryTests::test07_saveBookTwice(executor);
         // LibraryTests::test08_getBookNotFound(executor);

         // ========== CRUD TESTS ==========
         // cout << "\n=== CRUD TESTS ===" << endl;
         // LibraryTests::test09_bookCRUD(executor);
         // LibraryTests::test10_studentCRUD(executor);
         // LibraryTests::test11_createBookAndStudent(executor);

         // ========== BORROW FLOW TESTS ==========
         // cout << "\n=== BORROW FLOW TESTS ===" << endl;
         // LibraryTests::test12_createRequest(executor);
         // LibraryTests::test13_acceptRequest(executor);
         // LibraryTests::test14_fullBorrowReturn(executor);
         // LibraryTests::test15_directLoan(executor);
         // LibraryTests::test16_rejectRequest(executor);

         // ========== NEGATIVE TESTS (UNSAT) ==========
         // cout << "\n=== NEGATIVE TESTS (Expected UNSAT) ===" << endl;
         // LibraryTests::test17_requestWithoutBook(executor);
         // LibraryTests::test18_requestWithoutStudent(executor);
         // LibraryTests::test19_acceptWithoutRequest(executor);
         // LibraryTests::test20_returnWithoutLoan(executor);

         // ========== COMPLEX WORKFLOW TESTS ==========
         // cout << "\n=== COMPLEX WORKFLOW TESTS ===" << endl;
         // LibraryTests::test21_multipleBooks(executor);
         // LibraryTests::test22_multipleStudents(executor);
         // LibraryTests::test23_multipleBorrowings(executor);
         // LibraryTests::test24_fullLibraryWorkflow(executor);
         // LibraryTests::test25_complexScenario(executor);

         // cout << "\n╔════════════════════════════════════════╗" << endl;
         // cout << "║  ALL TESTS COMPLETED                   ║" << endl;
         // cout << "╚════════════════════════════════════════╝\n"
         //     << endl;
    }
    catch (const exception &e)
    {
        cerr << "\n❌ FATAL ERROR: " << e.what() << endl;
        return 1;
    }

    return 0;
}