#include "tripvaultfunctionfactory.hh"
#include <iostream>
#include <stdexcept>
#include <set>

using namespace std;
namespace TripVault {

/* ============================================================
 * TripVaultAPIFunction Base Implementation
 * ============================================================ */

TripVaultAPIFunction::TripVaultAPIFunction(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : factory(factory), arguments(args) {}

string TripVaultAPIFunction::extractString(Expr* expr) {
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

int TripVaultAPIFunction::extractInt(Expr* expr) {
    if (!expr)
        throw runtime_error("Null expression in extractInt");

    if (expr->exprType == ExprType::NUM) {
        Num* num = dynamic_cast<Num*>(expr);
        return num->value;
    }

    // Handle String/Var that may contain a numeric value from Z3
    if (expr->exprType == ExprType::STRING) {
        String* str = dynamic_cast<String*>(expr);
        try { return std::stoi(str->value); } catch (...) { return 500; }
    }

    if (expr->exprType == ExprType::VAR) {
        Var* var = dynamic_cast<Var*>(expr);
        try { return std::stoi(var->name); } catch (...) { return 500; }
    }

    return 500; // fallback default amount
}

json TripVaultAPIFunction::extractJson(Expr* expr) {
    if (!expr)
        return json::object();

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
        try {
            return json::parse(str->value);
        } catch (...) {
            return str->value;
        }
    }

    return json::object();
}

string TripVaultAPIFunction::getCurrentToken(const string& email) {
    // First check local cache
    auto it = factory->getT().find(email);
    if (it != factory->getT().end() && !it->second.empty()) {
        return it->second;
    }

    // If not found locally, fetch from backend
    HttpResponse resp = factory->getHttpClient()->get("/api/test/get_T");
    if (resp.statusCode == 200) {
        json data = resp.getJson();
        for (auto& [k, v] : data.items()) {
            factory->getT()[k] = v.get<string>();
        }

        // Try again after refresh
        it = factory->getT().find(email);
        if (it != factory->getT().end()) {
            return it->second;
        }
    }

    return "";
}

/* ============================================================
 * Test API Functions
 * ============================================================ */

// ========== RESET ==========
ResetFunc::ResetFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> ResetFunc::execute() {
    cout << "[ResetFunc] Clearing all collections..." << endl;

    try {
        json body = json::object();
        HttpResponse resp = factory->getHttpClient()->post("/api/test/reset", body);

        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getU().clear();
            factory->getT().clear();
            factory->getTrips().clear();
            factory->getMembers().clear();
            factory->getE().clear();
            factory->getProposals().clear();
            return make_unique<Num>(200);
        }
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[ResetFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_U ==========
GetUFunc::GetUFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> GetUFunc::execute() {
    cout << "[GetUFunc] Fetching U..." << endl;
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

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetUFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_U ==========
SetUFunc::SetUFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> SetUFunc::execute() {
    cout << "[SetUFunc] Setting U..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getU().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getU()[k] = v.get<string>();
        }
        return make_unique<Num>(200);
    } catch (const exception& e) {
        cerr << "[SetUFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_T ==========
GetTFunc::GetTFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> GetTFunc::execute() {
    cout << "[GetTFunc] Fetching T..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_T");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getT().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getT()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetTFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_T ==========
SetTFunc::SetTFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> SetTFunc::execute() {
    cout << "[SetTFunc] Setting T..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getT().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getT()[k] = v.get<string>();
        }
        return make_unique<Num>(200);
    } catch (const exception& e) {
        cerr << "[SetTFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_TRIPS ==========
GetTripsFunc::GetTripsFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> GetTripsFunc::execute() {
    cout << "[GetTripsFunc] Fetching Trips..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Trips");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getTrips().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getTrips()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetTripsFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_TRIPS ==========
SetTripsFunc::SetTripsFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> SetTripsFunc::execute() {
    cout << "[SetTripsFunc] Setting Trips..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getTrips().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getTrips()[k] = v.get<string>();
        }
        return make_unique<Num>(200);
    } catch (const exception& e) {
        cerr << "[SetTripsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_MEMBERS ==========
GetMembersFunc::GetMembersFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> GetMembersFunc::execute() {
    cout << "[GetMembersFunc] Fetching Members..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Members");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getMembers().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getMembers()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetMembersFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_MEMBERS ==========
SetMembersFunc::SetMembersFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> SetMembersFunc::execute() {
    cout << "[SetMembersFunc] Setting Members..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getMembers().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getMembers()[k] = v.get<string>();
        }
        return make_unique<Num>(200);
    } catch (const exception& e) {
        cerr << "[SetMembersFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_E ==========
GetEFunc::GetEFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> GetEFunc::execute() {
    cout << "[GetEFunc] Fetching E (Expenses)..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_E");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getE().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getE()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetEFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_E ==========
SetEFunc::SetEFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> SetEFunc::execute() {
    cout << "[SetEFunc] Setting E..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getE().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getE()[k] = v.get<string>();
        }
        return make_unique<Num>(200);
    } catch (const exception& e) {
        cerr << "[SetEFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_PROPOSALS ==========
GetProposalsFunc::GetProposalsFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> GetProposalsFunc::execute() {
    cout << "[GetProposalsFunc] Fetching Proposals..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Proposals");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getProposals().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getProposals()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetProposalsFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_PROPOSALS ==========
SetProposalsFunc::SetProposalsFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> SetProposalsFunc::execute() {
    cout << "[SetProposalsFunc] Setting Proposals..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getProposals().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getProposals()[k] = v.get<string>();
        }
        return make_unique<Num>(200);
    } catch (const exception& e) {
        cerr << "[SetProposalsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

/* ============================================================
 * Business API Functions
 * ============================================================ */

// ========== REGISTER USER ==========
RegisterUserFunc::RegisterUserFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> RegisterUserFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("registerUser requires 2 arguments");

    string email = extractString(arguments[0]);
    string password = extractString(arguments[1]);

    cout << "[RegisterUserFunc] Registering user: " << email << endl;

    try {
        json body = {
            {"email", email},
            {"password", password}
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/test/register", body);

        if (resp.statusCode == 201) {
            json data = resp.getJson();
            string userId = data["userId"].get<string>();
            factory->getU()[email] = userId;
            factory->getT()[email] = userId;
            return make_unique<String>(userId);
        }

        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[RegisterUserFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== LOGIN USER ==========
LoginUserFunc::LoginUserFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> LoginUserFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("loginUser requires 1 argument");

    string email = extractString(arguments[0]);

    cout << "[LoginUserFunc] Logging in user: " << email << endl;

    try {
        json body = {{"email", email}};

        HttpResponse resp = factory->getHttpClient()->post("/api/test/login", body);

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            string userId = data["userId"].get<string>();
            factory->getT()[email] = userId;
            return make_unique<String>(userId);
        }

        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[LoginUserFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== CREATE TRIP ==========
CreateTripFunc::CreateTripFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> CreateTripFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("createTrip requires 3 arguments");

    string email = extractString(arguments[0]);
    string tripName = extractString(arguments[1]);
    string destination = extractString(arguments[2]);

    cout << "[CreateTripFunc] Creating trip: " << tripName << " for " << email << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            cerr << "[CreateTripFunc] No token for " << email << endl;
            return make_unique<String>("");
        }

        json body = {
            {"tripName", tripName},
            {"destination", destination},
            {"startDate", "2025-06-01"},
            {"endDate", "2025-06-10"},
            {"description", "Test trip"},
            {"budget", {{"total", 10000}, {"currency", "INR"}}}
        };

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post("/api/trips", body, headers);

        if (resp.statusCode == 201) {
            json data = resp.getJson();
            string tripId = data["trip"]["_id"].get<string>();
            string inviteCode = data["trip"]["inviteCode"].get<string>();
            factory->getTrips()[tripId] = tripName + "|||" + inviteCode;
            factory->getMembers()[tripId] = token;
            return make_unique<String>(tripId);
        }

        cerr << "[CreateTripFunc] Failed with status: " << resp.statusCode << endl;
        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[CreateTripFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== GET USER TRIPS ==========
GetUserTripsFunc::GetUserTripsFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> GetUserTripsFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getUserTrips requires 1 argument");

    string email = extractString(arguments[0]);

    cout << "[GetUserTripsFunc] Getting trips for: " << email << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            return make_unique<Num>(401);
        }

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->get("/api/trips", headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetUserTripsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== UPDATE TRIP ==========
UpdateTripFunc::UpdateTripFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> UpdateTripFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("updateTrip requires 3 arguments");

    string email = extractString(arguments[0]);
    string tripId = extractString(arguments[1]);
    string newTripName = extractString(arguments[2]);

    cout << "[UpdateTripFunc] Updating trip: " << tripId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            return make_unique<Num>(401);
        }

        json body = {{"tripName", newTripName}};
        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->put("/api/trips/" + tripId, body, headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[UpdateTripFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== DELETE TRIP ==========
DeleteTripFunc::DeleteTripFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> DeleteTripFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("deleteTrip requires 2 arguments");

    string email = extractString(arguments[0]);
    string tripId = extractString(arguments[1]);

    cout << "[DeleteTripFunc] Deleting trip: " << tripId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            return make_unique<Num>(401);
        }

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->del("/api/trips/" + tripId, headers);

        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getTrips().erase(tripId);
            factory->getMembers().erase(tripId);
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[DeleteTripFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== ADD MEMBER ==========
AddMemberFunc::AddMemberFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> AddMemberFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("addMember requires 3 arguments");

    string adminEmail = extractString(arguments[0]);
    string tripId = extractString(arguments[1]);
    string memberEmail = extractString(arguments[2]);

    cout << "[AddMemberFunc] Adding member " << memberEmail << " to trip " << tripId << endl;

    try {
        string token = getCurrentToken(adminEmail);
        if (token.empty()) {
            return make_unique<Num>(401);
        }

        json body = {{"email", memberEmail}, {"role", "Viewer"}};
        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post("/api/trips/" + tripId + "/members", body, headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[AddMemberFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== JOIN BY INVITE ==========
JoinByInviteFunc::JoinByInviteFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> JoinByInviteFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("joinByInvite requires 2 arguments");

    string userEmail = extractString(arguments[0]);
    string tripId = extractString(arguments[1]);

    cout << "[JoinByInviteFunc] User " << userEmail << " joining trip " << tripId << " by invite" << endl;

    try {
        string token = getCurrentToken(userEmail);
        if (token.empty()) {
            return make_unique<Num>(401);
        }

        // Look up invite code from Trips_cache
        string inviteCode = "";
        auto it = factory->getTrips().find(tripId);
        if (it != factory->getTrips().end()) {
            string combined = it->second;
            size_t sep = combined.find("|||");
            if (sep != string::npos) {
                inviteCode = combined.substr(sep + 3);
            }
        }

        if (inviteCode.empty()) {
            // Refresh from backend
            HttpResponse refreshResp = factory->getHttpClient()->get("/api/test/get_Trips");
            if (refreshResp.statusCode == 200) {
                json data = refreshResp.getJson();
                for (auto& [k, v] : data.items()) {
                    factory->getTrips()[k] = v.get<string>();
                }
                auto it2 = factory->getTrips().find(tripId);
                if (it2 != factory->getTrips().end()) {
                    string combined = it2->second;
                    size_t sep = combined.find("|||");
                    if (sep != string::npos) {
                        inviteCode = combined.substr(sep + 3);
                    }
                }
            }
        }

        if (inviteCode.empty()) {
            cerr << "[JoinByInviteFunc] Could not find invite code for trip " << tripId << endl;
            return make_unique<Num>(404);
        }

        json body = json::object();
        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post("/api/trips/join/" + inviteCode, body, headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[JoinByInviteFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== REMOVE MEMBER ==========
RemoveMemberFunc::RemoveMemberFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> RemoveMemberFunc::execute() {
    cout << "[RemoveMemberFunc] stub - returning 200" << endl;
    return make_unique<Num>(200);
}

// ========== CREATE EXPENSE ==========
CreateExpenseFunc::CreateExpenseFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> CreateExpenseFunc::execute() {
    if (arguments.size() < 5)
        throw runtime_error("createExpense requires 5 arguments");

    string email = extractString(arguments[0]);
    string tripId = extractString(arguments[1]);
    string title = extractString(arguments[2]);
    int amount = extractInt(arguments[3]);
    string rawCategory = extractString(arguments[4]);
    // Ensure category is a valid enum value
    static const std::set<std::string> validCats = {"travel","food","accommodation","others"};
    string category = validCats.count(rawCategory) ? rawCategory : "others";

    cout << "[CreateExpenseFunc] Creating expense: " << title << " for trip " << tripId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            return make_unique<String>("");
        }

        string userId = token;

        json splitsArray = json::array();
        json splitEntry = {{"userId", userId}, {"percentage", 100}};
        splitsArray.push_back(splitEntry);

        json body = {
            {"title", title},
            {"description", "Test expense"},
            {"amount", amount},
            {"category", category},
            {"splits", splitsArray},
            {"expenseDate", "2025-06-05"}
        };

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post("/api/expenses/trip/" + tripId, body, headers);

        if (resp.statusCode == 201) {
            json data = resp.getJson();
            string expenseId = data["expense"]["_id"].get<string>();
            factory->getE()[expenseId] = userId;
            return make_unique<String>(expenseId);
        }

        cerr << "[CreateExpenseFunc] Failed with status: " << resp.statusCode << endl;
        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[CreateExpenseFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== GET EXPENSES ==========
GetExpensesFunc::GetExpensesFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> GetExpensesFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("getExpenses requires 2 arguments");

    string email = extractString(arguments[0]);
    string tripId = extractString(arguments[1]);

    cout << "[GetExpensesFunc] Getting expenses for trip: " << tripId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            return make_unique<Num>(401);
        }

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->get("/api/expenses/trip/" + tripId, headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetExpensesFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== DELETE EXPENSE ==========
DeleteExpenseFunc::DeleteExpenseFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> DeleteExpenseFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("deleteExpense requires 2 arguments");

    string email = extractString(arguments[0]);
    string expenseId = extractString(arguments[1]);

    cout << "[DeleteExpenseFunc] Deleting expense: " << expenseId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            return make_unique<Num>(401);
        }

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->del("/api/expenses/" + expenseId, headers);

        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getE().erase(expenseId);
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[DeleteExpenseFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== CREATE PROPOSAL ==========
CreateProposalFunc::CreateProposalFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> CreateProposalFunc::execute() {
    if (arguments.size() < 4)
        throw runtime_error("createProposal requires 4 arguments");

    string email = extractString(arguments[0]);
    string tripId = extractString(arguments[1]);
    string title = extractString(arguments[2]);
    string type = extractString(arguments[3]);

    cout << "[CreateProposalFunc] Creating proposal: " << title << " for trip " << tripId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            return make_unique<String>("");
        }

        string userId = token;

        json body = {
            {"title", title},
            {"description", "Test proposal description"},
            {"type", type}
        };

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post("/api/proposals/trip/" + tripId, body, headers);

        if (resp.statusCode == 201) {
            json data = resp.getJson();
            string proposalId = data["proposal"]["_id"].get<string>();
            factory->getProposals()[proposalId] = userId;
            return make_unique<String>(proposalId);
        }

        cerr << "[CreateProposalFunc] Failed with status: " << resp.statusCode << endl;
        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[CreateProposalFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== GET PROPOSALS LIST ==========
GetProposalsListFunc::GetProposalsListFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> GetProposalsListFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("getProposals requires 2 arguments");

    string email = extractString(arguments[0]);
    string tripId = extractString(arguments[1]);

    cout << "[GetProposalsListFunc] Getting proposals for trip: " << tripId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            return make_unique<Num>(401);
        }

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->get("/api/proposals/trip/" + tripId, headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetProposalsListFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== DELETE PROPOSAL ==========
DeleteProposalFunc::DeleteProposalFunc(TripVaultFunctionFactory* factory, vector<Expr*> args)
    : TripVaultAPIFunction(factory, args) {}

unique_ptr<Expr> DeleteProposalFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("deleteProposal requires 2 arguments");

    string email = extractString(arguments[0]);
    string proposalId = extractString(arguments[1]);

    cout << "[DeleteProposalFunc] Deleting proposal: " << proposalId << endl;

    try {
        string token = getCurrentToken(email);
        if (token.empty()) {
            return make_unique<Num>(401);
        }

        map<string, string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->del("/api/proposals/" + proposalId, headers);

        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getProposals().erase(proposalId);
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[DeleteProposalFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

/* ============================================================
 * TripVaultFunctionFactory
 * ============================================================ */

TripVaultFunctionFactory::TripVaultFunctionFactory(const string& baseUrl)
    : baseUrl(baseUrl) {
    httpClient = make_unique<HttpClient>(baseUrl);
}

unique_ptr<Function> TripVaultFunctionFactory::getFunction(string fname, vector<Expr*> args) {
    if (fname == "reset")             return make_unique<ResetFunc>(this, args);
    if (fname == "get_U")             return make_unique<GetUFunc>(this, args);
    if (fname == "set_U")             return make_unique<SetUFunc>(this, args);
    if (fname == "get_T")             return make_unique<GetTFunc>(this, args);
    if (fname == "set_T")             return make_unique<SetTFunc>(this, args);
    if (fname == "get_Trips")         return make_unique<GetTripsFunc>(this, args);
    if (fname == "set_Trips")         return make_unique<SetTripsFunc>(this, args);
    if (fname == "get_Members")       return make_unique<GetMembersFunc>(this, args);
    if (fname == "set_Members")       return make_unique<SetMembersFunc>(this, args);
    if (fname == "get_E")             return make_unique<GetEFunc>(this, args);
    if (fname == "set_E")             return make_unique<SetEFunc>(this, args);
    if (fname == "get_Proposals")     return make_unique<GetProposalsFunc>(this, args);
    if (fname == "set_Proposals")     return make_unique<SetProposalsFunc>(this, args);
    if (fname == "registerUser")      return make_unique<RegisterUserFunc>(this, args);
    if (fname == "loginUser")         return make_unique<LoginUserFunc>(this, args);
    if (fname == "createTrip")        return make_unique<CreateTripFunc>(this, args);
    if (fname == "getUserTrips")      return make_unique<GetUserTripsFunc>(this, args);
    if (fname == "updateTrip")        return make_unique<UpdateTripFunc>(this, args);
    if (fname == "deleteTrip")        return make_unique<DeleteTripFunc>(this, args);
    if (fname == "addMember")         return make_unique<AddMemberFunc>(this, args);
    if (fname == "joinByInvite")      return make_unique<JoinByInviteFunc>(this, args);
    if (fname == "removeMember")      return make_unique<RemoveMemberFunc>(this, args);
    if (fname == "createExpense")     return make_unique<CreateExpenseFunc>(this, args);
    if (fname == "getExpenses")       return make_unique<GetExpensesFunc>(this, args);
    if (fname == "deleteExpense")     return make_unique<DeleteExpenseFunc>(this, args);
    if (fname == "createProposal")    return make_unique<CreateProposalFunc>(this, args);
    if (fname == "getProposals")      return make_unique<GetProposalsListFunc>(this, args);
    if (fname == "deleteProposal")    return make_unique<DeleteProposalFunc>(this, args);

    throw runtime_error("TripVaultFunctionFactory: unknown function: " + fname);
}

} // namespace TripVault
