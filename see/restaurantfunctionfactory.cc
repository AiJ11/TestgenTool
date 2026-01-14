#include "restaurantfunctionfactory.hh"
#include <iostream>
#include <stdexcept>

using namespace std;

/* ============================================================
 * APIFunction Base Implementation
 * ============================================================ */

APIFunction::APIFunction(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : factory(factory), arguments(args) {}

string APIFunction::extractString(Expr* expr) {
    if (!expr) throw runtime_error("Null expression in extractString");
    
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

int APIFunction::extractInt(Expr* expr) {
    if (!expr) throw runtime_error("Null expression in extractInt");
    
    if (expr->exprType == ExprType::NUM) {
        Num* num = dynamic_cast<Num*>(expr);
        return num->value;
    }
    
    throw runtime_error("Expected NUM expression");
}

json APIFunction::extractJson(Expr* expr) {
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
        try {
            return json::parse(str->value);
        } catch (...) {
            return str->value;
        }
    }
    
    return json::object();
}

string APIFunction::getCurrentToken(const string& email) {
    auto it = factory->getT().find(email);
    if (it != factory->getT().end()) {
        return it->second;
    }
    return "";
}

/* ============================================================
 * Test API Functions
 * ============================================================ */

ResetFunc::ResetFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> ResetFunc::execute() {
    cout << "[ResetFunc] Clearing all collections..." << endl;
    
    try {
        json body = json::object();
        HttpResponse resp = factory->getHttpClient()->post("/api/test/reset", body);
        
        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getU().clear();
            factory->getT().clear();
            factory->getRoles().clear();
            factory->getC().clear();
            factory->getR().clear();
            factory->getM().clear();
            factory->getO().clear();
            factory->getRev().clear();
            factory->getOwners().clear();
            factory->getAssignments().clear();
            return make_unique<Num>(200);
        }
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[ResetFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_U
GetUFunc::GetUFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

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
                    make_unique<String>(value)
                ));
            }
        }
        
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetUFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// set_U
SetUFunc::SetUFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetUFunc::execute() {
    cout << "[SetUFunc] Setting U..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getU().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getU()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_U", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetUFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_T
GetTFunc::GetTFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}
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
                    make_unique<String>(value)
                ));
            }
        }
        
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// set_T
SetTFunc::SetTFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetTFunc::execute() {
    cout << "[SetTFunc] Setting T..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getT().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getT()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_T", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        return make_unique<Num>(500);
    }
}

// get_C
GetCFunc::GetCFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetCFunc::execute() {
    cout << "[GetCFunc] Fetching C..." << endl;
    
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_C");
        
        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getC().clear();
            
            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.dump(); // Store as JSON string
                factory->getC()[key] = value;
                
                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)
                ));
            }
        }
        
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetCFunc] Error: " << e.what() << endl;
        return make_unique<Map>(std::move(pairs));
    }
}

// set_C
SetCFunc::SetCFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetCFunc::execute() {
    cout << "[SetCFunc] Setting C..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getC().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getC()[k] = v.dump();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_C", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetCFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_R
GetRFunc::GetRFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetRFunc::execute() {
    cout << "[GetRFunc] Fetching R..." << endl;
    
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_R");
        
        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getR().clear();
            
            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.dump();
                factory->getR()[key] = value;
                
                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)
                ));
            }
        }
        
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetRFunc] Error: " << e.what() << endl;
        return make_unique<Map>(std::move(pairs));
    }
}

// set_R
SetRFunc::SetRFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetRFunc::execute() {
    cout << "[SetRFunc] Setting R..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getR().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getR()[k] = v.dump();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_R", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetRFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_M
GetMFunc::GetMFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetMFunc::execute() {
    cout << "[GetMFunc] Fetching M..." << endl;
    
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_M");
        
        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getM().clear();
            
            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.dump();
                factory->getM()[key] = value;
                
                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)
                ));
            }
        }
        
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetMFunc] Error: " << e.what() << endl;
        return make_unique<Map>(std::move(pairs));
    }
}

// set_M
SetMFunc::SetMFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetMFunc::execute() {
    cout << "[SetMFunc] Setting M..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getM().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getM()[k] = v.dump();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_M", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetMFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_O
GetOFunc::GetOFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetOFunc::execute() {
    cout << "[GetOFunc] Fetching O..." << endl;
    
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_O");
        
        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getO().clear();
            
            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.dump();
                factory->getO()[key] = value;
                
                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)
                ));
            }
        }
        
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetOFunc] Error: " << e.what() << endl;
        return make_unique<Map>(std::move(pairs));
    }
}

// set_O
SetOFunc::SetOFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetOFunc::execute() {
    cout << "[SetOFunc] Setting O..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getO().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getO()[k] = v.dump();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_O", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetOFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_Rev
GetRevFunc::GetRevFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetRevFunc::execute() {
    cout << "[GetRevFunc] Fetching Rev..." << endl;
    
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Rev");
        
        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getRev().clear();
            
            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.dump();
                factory->getRev()[key] = value;
                
                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)
                ));
            }
        }
        
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetRevFunc] Error: " << e.what() << endl;
        return make_unique<Map>(std::move(pairs));
    }
}

// set_Rev
SetRevFunc::SetRevFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetRevFunc::execute() {
    cout << "[SetRevFunc] Setting Rev..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getRev().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getRev()[k] = v.dump();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_Rev", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetRevFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_Roles
GetRolesFunc::GetRolesFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetRolesFunc::execute() {
    cout << "[GetRolesFunc] Fetching Roles..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Roles");
        
        // Build a Map expression to return
        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
        
        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getRoles().clear();
            
            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                
                factory->getRoles()[key] = value;
                
                // Add to AST Map
                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)
                ));
            }
        }
        
        // Return a Map expression (even if empty)
        return make_unique<Map>(std::move(pairs));
        
    } catch (const exception& e) {
        cerr << "[GetRolesFunc] Error: " << e.what() << endl;
        // Return empty map on error
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// set_Roles
SetRolesFunc::SetRolesFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetRolesFunc::execute() {
    cout << "[SetRolesFunc] Setting Roles..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getRoles().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getRoles()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_Roles", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        return make_unique<Num>(500);
    }
}

// get_C, set_C, get_R, set_R, get_M, set_M, get_O, set_O, get_Rev, set_Rev
// will do next...

// get_Owners
GetOwnersFunc::GetOwnersFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetOwnersFunc::execute() {
    cout << "[GetOwnersFunc] Fetching Owners..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Owners");
        
        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
        
        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getOwners().clear();
            
            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getOwners()[key] = value;
                
                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)
                ));
            }
        }
        
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// set_Owners
SetOwnersFunc::SetOwnersFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetOwnersFunc::execute() {
    cout << "[SetOwnersFunc] Setting Owners..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getOwners().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getOwners()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_Owners", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        return make_unique<Num>(500);
    }
}

// get_Assignments
GetAssignmentsFunc::GetAssignmentsFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)

    : APIFunction(factory, args) {}


unique_ptr<Expr> GetAssignmentsFunc::execute() {
    cout << "[GetAssignmentsFunc] Fetching Assignments..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Assignments");
        
        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
        
        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getAssignments().clear();
            
            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getAssignments()[key] = value;
                
                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)
                ));
            }
        }
        
        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}
// set_Assignments
SetAssignmentsFunc::SetAssignmentsFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetAssignmentsFunc::execute() {
    cout << "[SetAssignmentsFunc] Setting Assignments..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getAssignments().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getAssignments()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_Assignments", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        return make_unique<Num>(500);
    }
}

/* ============================================================
 * Business API Functions
 * ============================================================ */

// ========== CUSTOMER REGISTRATION (role = "customer") ==========
RegisterCustomerFunc::RegisterCustomerFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> RegisterCustomerFunc::execute() {
    if (arguments.size() < 4) throw runtime_error("registerCustomer requires 4 arguments");
    
    string email = extractString(arguments[0]);
    string password = extractString(arguments[1]);
    string fullName = extractString(arguments[2]);
    string mobile = extractString(arguments[3]);
    
    cout << "[RegisterCustomerFunc] Registering customer: " << email << endl;
    
    try {
        json body = {
            {"email", email},
            {"password", password},
            {"fullName", fullName},
            {"mobile", mobile},
            {"role", "customer"}  // ← FIXED: Explicit customer role
        };
        
        HttpResponse resp = factory->getHttpClient()->post("/api/auth/register", body);
        
        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            // Update caches
            factory->getU()[email] = password;
            factory->getRoles()[email] = "customer";
            
            // Sync to backend Test API
            json uMap, rolesMap;
            for (const auto& [e, p] : factory->getU()) uMap[e] = p;
            for (const auto& [e, r] : factory->getRoles()) rolesMap[e] = r;
            
            factory->getHttpClient()->post("/api/test/set_U", {{"data", uMap}});
            factory->getHttpClient()->post("/api/test/set_Roles", {{"data", rolesMap}});
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[RegisterCustomerFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== OWNER REGISTRATION (role = "restaurant_owner") ==========
RegisterOwnerFunc::RegisterOwnerFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> RegisterOwnerFunc::execute() {
    if (arguments.size() < 4) throw runtime_error("registerOwner requires 4 arguments");
    
    string email = extractString(arguments[0]);
    string password = extractString(arguments[1]);
    string fullName = extractString(arguments[2]);
    string mobile = extractString(arguments[3]);
    
    cout << "[RegisterOwnerFunc] Registering owner: " << email << endl;
    
    try {
        json body = {
            {"email", email},
            {"password", password},
            {"fullName", fullName},
            {"mobile", mobile},
            {"role", "restaurant_owner"}  // ← Owner role
        };
        
        HttpResponse resp = factory->getHttpClient()->post("/api/auth/register", body);
        
        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getU()[email] = password;
            factory->getRoles()[email] = "restaurant_owner";
            
            json uMap, rolesMap;
            for (const auto& [e, p] : factory->getU()) uMap[e] = p;
            for (const auto& [e, r] : factory->getRoles()) rolesMap[e] = r;
            
            factory->getHttpClient()->post("/api/test/set_U", {{"data", uMap}});
            factory->getHttpClient()->post("/api/test/set_Roles", {{"data", rolesMap}});
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[RegisterOwnerFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== AGENT REGISTRATION (role = "delivery_agent") ==========
RegisterAgentFunc::RegisterAgentFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> RegisterAgentFunc::execute() {
    if (arguments.size() < 4) throw runtime_error("registerAgent requires 4 arguments");
    
    string email = extractString(arguments[0]);
    string password = extractString(arguments[1]);
    string fullName = extractString(arguments[2]);
    string mobile = extractString(arguments[3]);
    
    cout << "[RegisterAgentFunc] Registering agent: " << email << endl;
    
    try {
        json body = {
            {"email", email},
            {"password", password},
            {"fullName", fullName},
            {"mobile", mobile},
            {"role", "delivery_agent"}  // ← Agent role
        };
        
        HttpResponse resp = factory->getHttpClient()->post("/api/auth/register", body);
        
        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getU()[email] = password;
            factory->getRoles()[email] = "delivery_agent";
            
            json uMap, rolesMap;
            for (const auto& [e, p] : factory->getU()) uMap[e] = p;
            for (const auto& [e, r] : factory->getRoles()) rolesMap[e] = r;
            
            factory->getHttpClient()->post("/api/test/set_U", {{"data", uMap}});
            factory->getHttpClient()->post("/api/test/set_Roles", {{"data", rolesMap}});
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[RegisterAgentFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== LOGIN ==========
LoginFunc::LoginFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> LoginFunc::execute() {
    if (arguments.size() < 2) throw runtime_error("login requires 2 arguments");
    
    string email = extractString(arguments[0]);
    string password = extractString(arguments[1]);
    
    cout << "[LoginFunc] Logging in: " << email << endl;
    
    try {
        json body = {
            {"email", email},
            {"password", password}
        };
        
        HttpResponse resp = factory->getHttpClient()->post("/api/auth/login", body);
        
        if (resp.statusCode == 200) {
            json respData = resp.getJson();
            
            if (respData.contains("token")) {
                string token = respData["token"].get<string>();
                cout << "[LoginFunc] Token received for: " << email << endl;
                
                factory->getT()[email] = token;
                
                json tMap;
                for (const auto& [e, t] : factory->getT()) tMap[e] = t;
                factory->getHttpClient()->post("/api/test/set_T", {{"data", tMap}});
                
                return make_unique<String>(token);
            }
        }
        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[LoginFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== BROWSE RESTAURANTS ==========
BrowseRestaurantsFunc::BrowseRestaurantsFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> BrowseRestaurantsFunc::execute() {
    cout << "[BrowseRestaurantsFunc] Fetching restaurants..." << endl;
    
    try {
        string token = getCurrentToken(extractString(arguments[0])); // email from context
        
        HttpResponse resp = factory->getHttpClient()->get("/api/restaurants", {
            {"Authorization", "Bearer " + token}
        });
        
        if (resp.statusCode == 200) {
            json respData = resp.getJson();
            cout << "[BrowseRestaurantsFunc] Found " << respData["restaurants"].size() << " restaurants" << endl;
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[BrowseRestaurantsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== VIEW MENU ==========
ViewMenuFunc::ViewMenuFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> ViewMenuFunc::execute() {
    if (arguments.size() < 1) throw runtime_error("viewMenu requires 1 argument");
    
    string restaurantId = extractString(arguments[0]);
    
    cout << "[ViewMenuFunc] Viewing menu for restaurant: " << restaurantId << endl;
    
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/restaurants/" + restaurantId + "/menu");
        
        if (resp.statusCode == 200) {
            json menuItems = resp.getJson();
            cout << "[ViewMenuFunc] Found " << menuItems.size() << " menu items" << endl;
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[ViewMenuFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== ADD TO CART ==========
AddToCartFunc::AddToCartFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> AddToCartFunc::execute() {
    if (arguments.size() < 2) throw runtime_error("addToCart requires 2 arguments");
    
    string menuItemId = extractString(arguments[0]);
    string quantity = extractString(arguments[1]);
    
    cout << "[AddToCartFunc] Adding item " << menuItemId << " to cart" << endl;
    
    try {
        // Need to get token from current user context
        // This would be set by symbolic execution when email is resolved
        string email = ""; // Will be resolved from symbolic execution context
        string token = getCurrentToken(email);
        
        json body = {
            {"menuItemId", menuItemId},
            {"quantity", stoi(quantity)}
        };
        
        HttpResponse resp = factory->getHttpClient()->post("/api/cart", body, {
            {"Authorization", "Bearer " + token}
        });
        
        if (resp.statusCode == 200) {
            // Update C_cache
            json respData = resp.getJson();
            if (respData.contains("cart")) {
                factory->getC()[email] = respData["cart"].dump();
            }
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[AddToCartFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== PLACE ORDER ==========
PlaceOrderFunc::PlaceOrderFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> PlaceOrderFunc::execute() {
    if (arguments.size() < 2) throw runtime_error("placeOrder requires 2 arguments");
    
    string deliveryAddress = extractString(arguments[0]);
    string paymentMethod = extractString(arguments[1]);
    
    cout << "[PlaceOrderFunc] Placing order..." << endl;
    
    try {
        string email = ""; // From symbolic execution context
        string token = getCurrentToken(email);
        
        json body = {
            {"deliveryAddress", {
                {"street", deliveryAddress},
                {"city", "TestCity"},
                {"state", "TestState"},
                {"zipCode", "12345"}
            }},
            {"paymentMethod", paymentMethod}
        };
        
        HttpResponse resp = factory->getHttpClient()->post("/api/orders", body, {
            {"Authorization", "Bearer " + token}
        });
        
        if (resp.statusCode == 201) {
            json respData = resp.getJson();
            if (respData.contains("order") && respData["order"].contains("_id")) {
                string orderId = respData["order"]["_id"].get<string>();
                factory->getO()[orderId] = respData["order"].dump();
                
                // Clear cart
                factory->getC().erase(email);
                
                cout << "[PlaceOrderFunc] Order placed: " << orderId << endl;
            }
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[PlaceOrderFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== LEAVE REVIEW ==========
LeaveReviewFunc::LeaveReviewFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> LeaveReviewFunc::execute() {
    if (arguments.size() < 4) throw runtime_error("leaveReview requires 4 arguments");
    
    string orderId = extractString(arguments[0]);
    string restaurantRating = extractString(arguments[1]);
    string deliveryRating = extractString(arguments[2]);
    string comment = extractString(arguments[3]);
    
    cout << "[LeaveReviewFunc] Leaving review for order: " << orderId << endl;
    
    try {
        string email = ""; // From context
        string token = getCurrentToken(email);
        
        json body = {
            {"orderId", orderId},
            {"restaurantRating", stoi(restaurantRating)},
            {"deliveryRating", stoi(deliveryRating)},
            {"restaurantComment", comment},
            {"deliveryComment", comment}
        };
        
        HttpResponse resp = factory->getHttpClient()->post("/api/reviews", body, {
            {"Authorization", "Bearer " + token}
        });
        
        if (resp.statusCode == 201) {
            json respData = resp.getJson();
            if (respData.contains("review") && respData["review"].contains("_id")) {
                string reviewId = respData["review"]["_id"].get<string>();
                factory->getRev()[reviewId] = respData["review"].dump();
                cout << "[LeaveReviewFunc] Review created: " << reviewId << endl;
            }
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[LeaveReviewFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== CREATE RESTAURANT (OWNER) ==========
CreateRestaurantFunc::CreateRestaurantFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> CreateRestaurantFunc::execute() {
    if (arguments.size() < 3) throw runtime_error("createRestaurant requires 3 arguments");
    
    string name = extractString(arguments[0]);
    string address = extractString(arguments[1]);
    string contact = extractString(arguments[2]);
    
    cout << "[CreateRestaurantFunc] Creating restaurant: " << name << endl;
    
    try {
        string email = ""; // From context
        string token = getCurrentToken(email);
        
        json body = {
            {"name", name},
            {"address", {
                {"street", address},
                {"city", "TestCity"},
                {"state", "TestState"},
                {"zipCode", "12345"}
            }},
            {"contact", {
                {"phone", contact},
                {"email", email}
            }},
            {"cuisineTypes", json::array({"Indian", "Continental"})},
            {"hours", {
                {"opening", "09:00"},
                {"closing", "22:00"}
            }}
        };
        
        HttpResponse resp = factory->getHttpClient()->post("/api/restaurants", body, {
            {"Authorization", "Bearer " + token}
        });
        
        if (resp.statusCode == 201) {
            json respData = resp.getJson();
            if (respData.contains("restaurant") && respData["restaurant"].contains("_id")) {
                string restaurantId = respData["restaurant"]["_id"].get<string>();
                factory->getR()[restaurantId] = respData["restaurant"].dump();
                factory->getOwners()[restaurantId] = email;
                
                cout << "[CreateRestaurantFunc] Restaurant created: " << restaurantId << endl;
            }
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[CreateRestaurantFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== ADD MENU ITEM (OWNER) ==========
AddMenuItemFunc::AddMenuItemFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> AddMenuItemFunc::execute() {
    if (arguments.size() < 3) throw runtime_error("addMenuItem requires 3 arguments");
    
    string restaurantId = extractString(arguments[0]);
    string name = extractString(arguments[1]);
    string price = extractString(arguments[2]);
    
    cout << "[AddMenuItemFunc] Adding menu item: " << name << endl;
    
    try {
        string email = ""; // From context
        string token = getCurrentToken(email);
        
        json body = {
            {"restaurantId", restaurantId},
            {"name", name},
            {"description", "Test menu item"},
            {"category", "Main Course"},
            {"price", stof(price)},
            {"isVegetarian", true}
        };
        
        HttpResponse resp = factory->getHttpClient()->post("/api/menu", body, {
            {"Authorization", "Bearer " + token}
        });
        
        if (resp.statusCode == 201) {
            json respData = resp.getJson();
            if (respData.contains("menuItem") && respData["menuItem"].contains("_id")) {
                string menuItemId = respData["menuItem"]["_id"].get<string>();
                factory->getM()[menuItemId] = respData["menuItem"].dump();
                cout << "[AddMenuItemFunc] Menu item created: " << menuItemId << endl;
            }
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[AddMenuItemFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== ASSIGN ORDER (OWNER) ==========
AssignOrderFunc::AssignOrderFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> AssignOrderFunc::execute() {
    if (arguments.size() < 2) throw runtime_error("assignOrder requires 2 arguments");
    
    string orderId = extractString(arguments[0]);
    string agentEmail = extractString(arguments[1]);
    
    cout << "[AssignOrderFunc] Assigning order " << orderId << " to " << agentEmail << endl;
    
    try {
        string email = ""; // Owner email from context
        string token = getCurrentToken(email);
        
        // Need to get agent's user ID
        // In real implementation, would query backend for user ID by email
        // For now, pass email directly (backend needs to handle this)
        
        json body = {
            {"deliveryAgentId", agentEmail}  // Simplified - should be ID
        };
        
        HttpResponse resp = factory->getHttpClient()->put("/api/orders/" + orderId + "/assign", body, {
            {"Authorization", "Bearer " + token}
        });
        
        if (resp.statusCode == 200) {
            factory->getAssignments()[orderId] = agentEmail;
            cout << "[AssignOrderFunc] Order assigned successfully" << endl;
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[AssignOrderFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== UPDATE ORDER STATUS (OWNER) ==========
UpdateOrderStatusOwnerFunc::UpdateOrderStatusOwnerFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> UpdateOrderStatusOwnerFunc::execute() {
    if (arguments.size() < 2) throw runtime_error("updateOrderStatusOwner requires 2 arguments");
    
    string orderId = extractString(arguments[0]);
    string status = extractString(arguments[1]);
    
    cout << "[UpdateOrderStatusOwnerFunc] Updating order " << orderId << " to " << status << endl;
    
    try {
        string email = ""; // From context
        string token = getCurrentToken(email);
        
        json body = {
            {"status", status}
        };
        
        HttpResponse resp = factory->getHttpClient()->put("/api/orders/" + orderId + "/status", body, {
            {"Authorization", "Bearer " + token}
        });
        
        if (resp.statusCode == 200) {
            cout << "[UpdateOrderStatusOwnerFunc] Order status updated" << endl;
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[UpdateOrderStatusOwnerFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== UPDATE ORDER STATUS (AGENT) ==========
UpdateOrderStatusAgentFunc::UpdateOrderStatusAgentFunc(RestaurantFunctionFactory* factory, vector<Expr*> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> UpdateOrderStatusAgentFunc::execute() {
    if (arguments.size() < 2) throw runtime_error("updateOrderStatusAgent requires 2 arguments");
    
    string orderId = extractString(arguments[0]);
    string status = extractString(arguments[1]);
    
    cout << "[UpdateOrderStatusAgentFunc] Updating order " << orderId << " to " << status << endl;
    
    try {
        string email = ""; // From context
        string token = getCurrentToken(email);
        
        json body = {
            {"status", status}
        };
        
        HttpResponse resp = factory->getHttpClient()->put("/api/orders/" + orderId + "/status", body, {
            {"Authorization", "Bearer " + token}
        });
        
        if (resp.statusCode == 200) {
            cout << "[UpdateOrderStatusAgentFunc] Order status updated" << endl;
        }
        
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[UpdateOrderStatusAgentFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

/* ============================================================
 * RestaurantFunctionFactory Implementation
 * ============================================================ */

RestaurantFunctionFactory::RestaurantFunctionFactory(const string& baseUrl)
    : baseUrl(baseUrl) {
    httpClient = make_unique<HttpClient>(baseUrl);
    cout << "[RestaurantFunctionFactory] Initialized with baseUrl: " << baseUrl << endl;
}

unique_ptr<Function> RestaurantFunctionFactory::getFunction(string fname, vector<Expr*> args) {
    cout << "[Factory] Creating function: " << fname << endl;
    
    // Test API functions
    if (fname == "reset") return make_unique<ResetFunc>(this, args);
    if (fname == "get_U") return make_unique<GetUFunc>(this, args);
    if (fname == "set_U") return make_unique<SetUFunc>(this, args);
    if (fname == "get_T") return make_unique<GetTFunc>(this, args);
    if (fname == "set_T") return make_unique<SetTFunc>(this, args);
    if (fname == "get_Roles") return make_unique<GetRolesFunc>(this, args);
    if (fname == "set_Roles") return make_unique<SetRolesFunc>(this, args);
    if (fname == "get_Owners") return make_unique<GetOwnersFunc>(this, args);
    if (fname == "set_Owners") return make_unique<SetOwnersFunc>(this, args);
    if (fname == "get_Assignments") return make_unique<GetAssignmentsFunc>(this, args);
    if (fname == "set_Assignments") return make_unique<SetAssignmentsFunc>(this, args);
    if (fname == "get_C") return make_unique<GetCFunc>(this, args);
    if (fname == "set_C") return make_unique<SetCFunc>(this, args);
    if (fname == "get_R") return make_unique<GetRFunc>(this, args);
    if (fname == "set_R") return make_unique<SetRFunc>(this, args);
    if (fname == "get_M") return make_unique<GetMFunc>(this, args);
    if (fname == "set_M") return make_unique<SetMFunc>(this, args);
    if (fname == "get_O") return make_unique<GetOFunc>(this, args);
    if (fname == "set_O") return make_unique<SetOFunc>(this, args);
    if (fname == "get_Rev") return make_unique<GetRevFunc>(this, args);
    if (fname == "set_Rev") return make_unique<SetRevFunc>(this, args);
    // Business API functions - 3 separate register functions
    if (fname == "registerCustomer") return make_unique<RegisterCustomerFunc>(this, args);
    if (fname == "registerOwner") return make_unique<RegisterOwnerFunc>(this, args);
    if (fname == "registerAgent") return make_unique<RegisterAgentFunc>(this, args);
    
    if (fname == "login") return make_unique<LoginFunc>(this, args);
    if (fname == "browseRestaurants") return make_unique<BrowseRestaurantsFunc>(this, args);
    if (fname == "viewMenu") return make_unique<ViewMenuFunc>(this, args);
    if (fname == "addToCart") return make_unique<AddToCartFunc>(this, args);
    if (fname == "placeOrder") return make_unique<PlaceOrderFunc>(this, args);
    if (fname == "leaveReview") return make_unique<LeaveReviewFunc>(this, args);
    if (fname == "createRestaurant") return make_unique<CreateRestaurantFunc>(this, args);
    if (fname == "addMenuItem") return make_unique<AddMenuItemFunc>(this, args);
    if (fname == "assignOrder") return make_unique<AssignOrderFunc>(this, args);
    if (fname == "updateOrderStatusOwner") return make_unique<UpdateOrderStatusOwnerFunc>(this, args);
    if (fname == "updateOrderStatusAgent") return make_unique<UpdateOrderStatusAgentFunc>(this, args);
    
    throw runtime_error("Unknown function: " + fname);
}
