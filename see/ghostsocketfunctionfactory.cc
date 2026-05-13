#include "ghostsocketfunctionfactory.hh"
#include <iostream>
#include <stdexcept>

using namespace std;
namespace GhostSocket {

/* ============================================================
 * GhostSocketAPIFunction Base Implementation
 * ============================================================ */

GhostSocketAPIFunction::GhostSocketAPIFunction(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : factory(factory), arguments(args) {}

string GhostSocketAPIFunction::extractString(Expr* expr) {
    if (!expr)
        throw runtime_error("Null expression in extractString");

    if (expr->exprType == ExprType::STRING) {
        String* str = dynamic_cast<String*>(expr);
        return str->value;
    }

    if (expr->exprType == ExprType::VAR) {
        Var* var = dynamic_cast<Var*>(expr);
        return var->name;
    }

    throw runtime_error("Expected STRING or VAR expression");
}

json GhostSocketAPIFunction::extractJson(Expr* expr) {
    if (!expr) return json::object();

    if (expr->exprType == ExprType::MAP) {
        Map* map = dynamic_cast<Map*>(expr);
        json obj = json::object();
        for (const auto& kv : map->value) {
            string key = kv.first->name;
            if (kv.second->exprType == ExprType::STRING) {
                String* val = dynamic_cast<String*>(kv.second.get());
                obj[key] = val->value;
            } else if (kv.second->exprType == ExprType::NUM) {
                Num* val = dynamic_cast<Num*>(kv.second.get());
                obj[key] = val->value;
            } else {
                obj[key] = "unsupported_type";
            }
        }
        return obj;
    }

    if (expr->exprType == ExprType::STRING) {
        String* str = dynamic_cast<String*>(expr);
        try { return json::parse(str->value); } catch (...) { return str->value; }
    }

    return json::object();
}

string GhostSocketAPIFunction::getCurrentToken(const string& email) {
    auto it = factory->getU().find(email);
    if (it != factory->getU().end() && !it->second.empty()) {
        return it->second;
    }

    // Refresh from backend
    HttpResponse resp = factory->getHttpClient()->get("/api/test/get_U");
    if (resp.statusCode == 200) {
        json data = resp.getJson();
        for (auto& [k, v] : data.items()) {
            factory->getU()[k] = v.get<string>();
        }
        it = factory->getU().find(email);
        if (it != factory->getU().end()) return it->second;
    }

    return "";
}

/* ============================================================
 * Test API Helper Functions
 * ============================================================ */

// ========== RESET ==========
ResetFunc::ResetFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> ResetFunc::execute() {
    cout << "[GS:ResetFunc] Clearing all collections..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->post("/api/test/reset", json::object());
        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getU().clear();
            factory->getD().clear();
            factory->getS().clear();
            return make_unique<Num>(200);
        }
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GS:ResetFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_U ==========
GetUFunc::GetUFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> GetUFunc::execute() {
    cout << "[GS:GetUFunc] Fetching U..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_U");
        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getU().clear();
            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getU()[key] = value;
                pairs.push_back(make_pair(make_unique<Var>(key), make_unique<String>(value)));
            }
        }
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GS:GetUFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_U ==========
SetUFunc::SetUFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> SetUFunc::execute() {
    try {
        json mapData = extractJson(arguments[0]);
        factory->getU().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getU()[k] = v.get<string>();
        }
        return make_unique<Num>(200);
    } catch (const exception& e) {
        cerr << "[GS:SetUFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_D ==========
GetDFunc::GetDFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> GetDFunc::execute() {
    cout << "[GS:GetDFunc] Fetching D..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_D");
        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getD().clear();
            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getD()[key] = value;
                pairs.push_back(make_pair(make_unique<Var>(key), make_unique<String>(value)));
            }
        }
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GS:GetDFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_D ==========
SetDFunc::SetDFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> SetDFunc::execute() {
    try {
        json mapData = extractJson(arguments[0]);
        factory->getD().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getD()[k] = v.get<string>();
        }
        return make_unique<Num>(200);
    } catch (const exception& e) {
        cerr << "[GS:SetDFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_S ==========
GetSFunc::GetSFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> GetSFunc::execute() {
    cout << "[GS:GetSFunc] Fetching S..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_S");
        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getS().clear();
            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getS()[key] = value;
                pairs.push_back(make_pair(make_unique<Var>(key), make_unique<String>(value)));
            }
        }
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GS:GetSFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_S ==========
SetSFunc::SetSFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> SetSFunc::execute() {
    try {
        json mapData = extractJson(arguments[0]);
        factory->getS().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getS()[k] = v.get<string>();
        }
        return make_unique<Num>(200);
    } catch (const exception& e) {
        cerr << "[GS:SetSFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

/* ============================================================
 * Business API Functions
 * ============================================================ */

// ========== REGISTER USER ==========
RegisterUserFunc::RegisterUserFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> RegisterUserFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("registerUser requires 1 argument");

    string email = extractString(arguments[0]);
    cout << "[GS:RegisterUserFunc] Registering: " << email << endl;

    try {
        json body = {{"email", email}, {"firstName", "Test"}, {"lastName", "User"}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/register", body);

        if (resp.statusCode == 201) {
            json data = resp.getJson();
            string userId = data["userId"].get<string>();
            factory->getU()[email] = userId;
            return make_unique<String>(userId);
        }

        cerr << "[GS:RegisterUserFunc] Failed with status: " << resp.statusCode << endl;
        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[GS:RegisterUserFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== REGISTER DEVICE ==========
RegisterDeviceFunc::RegisterDeviceFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> RegisterDeviceFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("registerDevice requires 2 arguments");

    string email = extractString(arguments[0]);
    string deviceId = extractString(arguments[1]);
    cout << "[GS:RegisterDeviceFunc] Registering device: " << deviceId << " for " << email << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            cerr << "[GS:RegisterDeviceFunc] No token for " << email << endl;
            return make_unique<String>("");
        }

        json body = {{"deviceId", deviceId}, {"userId", token}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/register-device", body);

        if (resp.statusCode == 200) {
            factory->getD()[deviceId] = deviceId;
            return make_unique<String>(deviceId);
        }

        cerr << "[GS:RegisterDeviceFunc] Failed with status: " << resp.statusCode << endl;
        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[GS:RegisterDeviceFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== GET MY DEVICES ==========
GetMyDevicesFunc::GetMyDevicesFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> GetMyDevicesFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getMyDevices requires 1 argument");

    string email = extractString(arguments[0]);
    cout << "[GS:GetMyDevicesFunc] Getting devices for: " << email << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) return make_unique<Num>(401);

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->get("/devices/my", headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GS:GetMyDevicesFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET OTHER DEVICES ==========
GetOtherDevicesFunc::GetOtherDevicesFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> GetOtherDevicesFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getOtherDevices requires 1 argument");

    string email = extractString(arguments[0]);
    cout << "[GS:GetOtherDevicesFunc] Getting other devices for: " << email << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) return make_unique<Num>(401);

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->get("/devices/other", headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GS:GetOtherDevicesFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET DEVICE INFO ==========
GetDeviceInfoFunc::GetDeviceInfoFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> GetDeviceInfoFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("getDeviceInfo requires 2 arguments");

    string email = extractString(arguments[0]);
    string deviceId = extractString(arguments[1]);
    cout << "[GS:GetDeviceInfoFunc] Getting info for device: " << deviceId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) return make_unique<Num>(401);

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->get("/devices/" + deviceId, headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GS:GetDeviceInfoFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== DELETE DEVICE ==========
DeleteDeviceFunc::DeleteDeviceFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> DeleteDeviceFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("deleteDevice requires 2 arguments");

    string email = extractString(arguments[0]);
    string deviceId = extractString(arguments[1]);
    cout << "[GS:DeleteDeviceFunc] Deleting device: " << deviceId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) return make_unique<Num>(401);

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->del("/devices/" + deviceId, headers);

        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getD().erase(deviceId);
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GS:DeleteDeviceFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== CREATE SESSION ==========
CreateSessionFunc::CreateSessionFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> CreateSessionFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("createSession requires 2 arguments");

    string email = extractString(arguments[0]);
    string deviceId = extractString(arguments[1]);
    cout << "[GS:CreateSessionFunc] Creating session for device: " << deviceId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            cerr << "[GS:CreateSessionFunc] No token for " << email << endl;
            return make_unique<String>("");
        }

        // Hardcode a minimal set of permissions
        json body = {
            {"deviceId", deviceId},
            {"permissions", json::array({
                {{"remoteControl", false}},
                {{"screenShare", true}},
                {{"terminalAccess", false}},
                {{"fileAccess", false}},
                {{"webcamFeed", false}},
                {{"resourceMonitor", true}}
            })}
        };

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post("/sessions/create", body, headers);

        if (resp.statusCode == 201) {
            json data = resp.getJson();
            string sessionKey = data["sessionKey"].get<string>();
            factory->getS()[sessionKey] = sessionKey;
            return make_unique<String>(sessionKey);
        }

        cerr << "[GS:CreateSessionFunc] Failed with status: " << resp.statusCode << endl;
        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[GS:CreateSessionFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== JOIN SESSION ==========
JoinSessionFunc::JoinSessionFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> JoinSessionFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("joinSession requires 2 arguments");

    string email = extractString(arguments[0]);
    string sessionId = extractString(arguments[1]);
    cout << "[GS:JoinSessionFunc] Joining session: " << sessionId << " as " << email << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) return make_unique<Num>(401);

        // sessionId is the actual session key (identity-mapped in S_cache)
        json body = {{"sessionKey", sessionId}};
        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post("/sessions/join", body, headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GS:JoinSessionFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET SESSIONS ==========
GetSessionsFunc::GetSessionsFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> GetSessionsFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getSessions requires 1 argument");

    string email = extractString(arguments[0]);
    cout << "[GS:GetSessionsFunc] Getting sessions for: " << email << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) return make_unique<Num>(401);

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->get("/sessions/get-sessions", headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GS:GetSessionsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== TERMINATE SESSION ==========
TerminateSessionFunc::TerminateSessionFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> TerminateSessionFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("terminateSession requires 2 arguments");

    string email = extractString(arguments[0]);
    string sessionId = extractString(arguments[1]);
    cout << "[GS:TerminateSessionFunc] Terminating session: " << sessionId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) return make_unique<Num>(401);

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->del("/sessions/" + sessionId, headers);

        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getS().erase(sessionId);
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GS:TerminateSessionFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== UPDATE PERMISSIONS ==========
UpdatePermissionsFunc::UpdatePermissionsFunc(GhostSocketFunctionFactory* factory, vector<Expr*> args)
    : GhostSocketAPIFunction(factory, args) {}

unique_ptr<Expr> UpdatePermissionsFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("updatePermissions requires 2 arguments");

    string email = extractString(arguments[0]);
    string sessionId = extractString(arguments[1]);
    cout << "[GS:UpdatePermissionsFunc] Updating permissions for session: " << sessionId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) return make_unique<Num>(401);

        json body = {
            {"sessionKey", sessionId},
            {"permissions", json::array({
                {{"remoteControl", true}},
                {{"screenShare", true}},
                {{"terminalAccess", false}},
                {{"fileAccess", false}},
                {{"webcamFeed", false}},
                {{"resourceMonitor", true}}
            })}
        };

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->put("/sessions/update-permissions", body, headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GS:UpdatePermissionsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

/* ============================================================
 * GhostSocketFunctionFactory
 * ============================================================ */

GhostSocketFunctionFactory::GhostSocketFunctionFactory(const string& baseUrl)
    : baseUrl(baseUrl) {
    httpClient = make_unique<HttpClient>(baseUrl);
}

unique_ptr<Function> GhostSocketFunctionFactory::getFunction(string fname, vector<Expr*> args) {
    if (fname == "reset")               return make_unique<ResetFunc>(this, args);
    if (fname == "get_U")               return make_unique<GetUFunc>(this, args);
    if (fname == "set_U")               return make_unique<SetUFunc>(this, args);
    if (fname == "get_D")               return make_unique<GetDFunc>(this, args);
    if (fname == "set_D")               return make_unique<SetDFunc>(this, args);
    if (fname == "get_S")               return make_unique<GetSFunc>(this, args);
    if (fname == "set_S")               return make_unique<SetSFunc>(this, args);
    if (fname == "registerUser")        return make_unique<RegisterUserFunc>(this, args);
    if (fname == "registerDevice")      return make_unique<RegisterDeviceFunc>(this, args);
    if (fname == "getMyDevices")        return make_unique<GetMyDevicesFunc>(this, args);
    if (fname == "getOtherDevices")     return make_unique<GetOtherDevicesFunc>(this, args);
    if (fname == "getDeviceInfo")       return make_unique<GetDeviceInfoFunc>(this, args);
    if (fname == "deleteDevice")        return make_unique<DeleteDeviceFunc>(this, args);
    if (fname == "createSession")       return make_unique<CreateSessionFunc>(this, args);
    if (fname == "joinSession")         return make_unique<JoinSessionFunc>(this, args);
    if (fname == "getSessions")         return make_unique<GetSessionsFunc>(this, args);
    if (fname == "terminateSession")    return make_unique<TerminateSessionFunc>(this, args);
    if (fname == "updatePermissions")   return make_unique<UpdatePermissionsFunc>(this, args);

    throw runtime_error("GhostSocketFunctionFactory: unknown function: " + fname);
}

} // namespace GhostSocket
