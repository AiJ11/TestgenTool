#ifndef TRIPVAULTFUNCTIONFACTORY_HH
#define TRIPVAULTFUNCTIONFACTORY_HH

#include "functionfactory.hh"
#include "../ast.hh"
#include "httpclient.hh"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <map>

using json = nlohmann::json;
using namespace std;

namespace TripVault {

// Forward declarations
class TripVaultFunctionFactory;

/* ============================================================
 * Base class for all API functions
 * ============================================================ */
class TripVaultAPIFunction : public Function {
protected:
    TripVaultFunctionFactory* factory;
    vector<Expr*> arguments;

public:
    TripVaultAPIFunction(TripVaultFunctionFactory* factory, vector<Expr*> args);
    virtual ~TripVaultAPIFunction() = default;

    string extractString(Expr* expr);
    int extractInt(Expr* expr);
    json extractJson(Expr* expr);
    string getCurrentToken(const string& email);
};

/* ============================================================
 * Test API Helper Functions
 * ============================================================ */

class ResetFunc : public TripVaultAPIFunction {
public:
    ResetFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetUFunc : public TripVaultAPIFunction {
public:
    GetUFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetUFunc : public TripVaultAPIFunction {
public:
    SetUFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetTFunc : public TripVaultAPIFunction {
public:
    GetTFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetTFunc : public TripVaultAPIFunction {
public:
    SetTFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetTripsFunc : public TripVaultAPIFunction {
public:
    GetTripsFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetTripsFunc : public TripVaultAPIFunction {
public:
    SetTripsFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetMembersFunc : public TripVaultAPIFunction {
public:
    GetMembersFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetMembersFunc : public TripVaultAPIFunction {
public:
    SetMembersFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetEFunc : public TripVaultAPIFunction {
public:
    GetEFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetEFunc : public TripVaultAPIFunction {
public:
    SetEFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetProposalsFunc : public TripVaultAPIFunction {
public:
    GetProposalsFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetProposalsFunc : public TripVaultAPIFunction {
public:
    SetProposalsFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

/* ============================================================
 * Business API Functions
 * ============================================================ */

class RegisterUserFunc : public TripVaultAPIFunction {
public:
    RegisterUserFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class LoginUserFunc : public TripVaultAPIFunction {
public:
    LoginUserFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class CreateTripFunc : public TripVaultAPIFunction {
public:
    CreateTripFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetUserTripsFunc : public TripVaultAPIFunction {
public:
    GetUserTripsFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class UpdateTripFunc : public TripVaultAPIFunction {
public:
    UpdateTripFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class DeleteTripFunc : public TripVaultAPIFunction {
public:
    DeleteTripFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class AddMemberFunc : public TripVaultAPIFunction {
public:
    AddMemberFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class JoinByInviteFunc : public TripVaultAPIFunction {
public:
    JoinByInviteFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class RemoveMemberFunc : public TripVaultAPIFunction {
public:
    RemoveMemberFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class CreateExpenseFunc : public TripVaultAPIFunction {
public:
    CreateExpenseFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetExpensesFunc : public TripVaultAPIFunction {
public:
    GetExpensesFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class DeleteExpenseFunc : public TripVaultAPIFunction {
public:
    DeleteExpenseFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class CreateProposalFunc : public TripVaultAPIFunction {
public:
    CreateProposalFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetProposalsListFunc : public TripVaultAPIFunction {
public:
    GetProposalsListFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class DeleteProposalFunc : public TripVaultAPIFunction {
public:
    DeleteProposalFunc(TripVaultFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

/* ============================================================
 * TripVaultFunctionFactory
 * ============================================================ */
class TripVaultFunctionFactory : public FunctionFactory {
private:
    unique_ptr<HttpClient> httpClient;
    string baseUrl;

    map<string, string> U_cache;
    map<string, string> T_cache;
    map<string, string> Trips_cache;
    map<string, string> Members_cache;
    map<string, string> E_cache;
    map<string, string> Proposals_cache;

public:
    TripVaultFunctionFactory(const string& baseUrl = "http://localhost:4001");
    ~TripVaultFunctionFactory() = default;

    unique_ptr<Function> getFunction(string fname, vector<Expr*> args) override;

    HttpClient* getHttpClient() { return httpClient.get(); }

    map<string, string>& getU() { return U_cache; }
    map<string, string>& getT() { return T_cache; }
    map<string, string>& getTrips() { return Trips_cache; }
    map<string, string>& getMembers() { return Members_cache; }
    map<string, string>& getE() { return E_cache; }
    map<string, string>& getProposals() { return Proposals_cache; }
};

} // namespace TripVault

#endif // TRIPVAULTFUNCTIONFACTORY_HH
