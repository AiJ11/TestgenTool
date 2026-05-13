#ifndef GHOSTSOCKETFUNCTIONFACTORY_HH
#define GHOSTSOCKETFUNCTIONFACTORY_HH

#include "functionfactory.hh"
#include "../ast.hh"
#include "httpclient.hh"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <map>

using json = nlohmann::json;
using namespace std;

namespace GhostSocket {

// Forward declarations
class GhostSocketFunctionFactory;

/* ============================================================
 * Base class for all API functions
 * ============================================================ */
class GhostSocketAPIFunction : public Function {
protected:
    GhostSocketFunctionFactory* factory;
    vector<Expr*> arguments;

public:
    GhostSocketAPIFunction(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    virtual ~GhostSocketAPIFunction() = default;

    string extractString(Expr* expr);
    json extractJson(Expr* expr);
    string getCurrentToken(const string& email);
};

/* ============================================================
 * Test API Helper Functions
 * ============================================================ */

class ResetFunc : public GhostSocketAPIFunction {
public:
    ResetFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetUFunc : public GhostSocketAPIFunction {
public:
    GetUFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetUFunc : public GhostSocketAPIFunction {
public:
    SetUFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetDFunc : public GhostSocketAPIFunction {
public:
    GetDFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetDFunc : public GhostSocketAPIFunction {
public:
    SetDFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetSFunc : public GhostSocketAPIFunction {
public:
    GetSFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetSFunc : public GhostSocketAPIFunction {
public:
    SetSFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

/* ============================================================
 * Business API Functions
 * ============================================================ */

class RegisterUserFunc : public GhostSocketAPIFunction {
public:
    RegisterUserFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class RegisterDeviceFunc : public GhostSocketAPIFunction {
public:
    RegisterDeviceFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetMyDevicesFunc : public GhostSocketAPIFunction {
public:
    GetMyDevicesFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetOtherDevicesFunc : public GhostSocketAPIFunction {
public:
    GetOtherDevicesFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetDeviceInfoFunc : public GhostSocketAPIFunction {
public:
    GetDeviceInfoFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class DeleteDeviceFunc : public GhostSocketAPIFunction {
public:
    DeleteDeviceFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class CreateSessionFunc : public GhostSocketAPIFunction {
public:
    CreateSessionFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class JoinSessionFunc : public GhostSocketAPIFunction {
public:
    JoinSessionFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetSessionsFunc : public GhostSocketAPIFunction {
public:
    GetSessionsFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class TerminateSessionFunc : public GhostSocketAPIFunction {
public:
    TerminateSessionFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class UpdatePermissionsFunc : public GhostSocketAPIFunction {
public:
    UpdatePermissionsFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

/* ============================================================
 * GhostSocketFunctionFactory
 * ============================================================ */
class GhostSocketFunctionFactory : public FunctionFactory {
private:
    unique_ptr<HttpClient> httpClient;
    string baseUrl;

    map<string, string> U_cache;   // email → token
    map<string, string> D_cache;   // deviceId → deviceId
    map<string, string> S_cache;   // sessionId → sessionId

public:
    GhostSocketFunctionFactory(const string& baseUrl = "http://localhost:4002");
    ~GhostSocketFunctionFactory() = default;

    unique_ptr<Function> getFunction(string fname, vector<Expr*> args) override;

    HttpClient* getHttpClient() { return httpClient.get(); }

    map<string, string>& getU() { return U_cache; }
    map<string, string>& getD() { return D_cache; }
    map<string, string>& getS() { return S_cache; }
};

} // namespace GhostSocket

#endif // GHOSTSOCKETFUNCTIONFACTORY_HH
