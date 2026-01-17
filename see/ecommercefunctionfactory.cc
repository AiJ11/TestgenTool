#include "ecommercefunctionfactory.hh"
#include <iostream>
#include <stdexcept>

using namespace std;
namespace Ecommerce {

/* ============================================================
 * EcommerceAPIFunction Base Implementation
 * ============================================================ */

EcommerceAPIFunction::EcommerceAPIFunction(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : factory(factory), arguments(args) {}

string EcommerceAPIFunction::extractString(Expr* expr) {
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

int EcommerceAPIFunction::extractInt(Expr* expr) {
    if (!expr)
        throw runtime_error("Null expression in extractInt");

    if (expr->exprType == ExprType::NUM) {
        Num* num = dynamic_cast<Num*>(expr);
        return num->value;
    }

    throw runtime_error("Expected NUM expression");
}

json EcommerceAPIFunction::extractJson(Expr* expr) {
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

string EcommerceAPIFunction::getCurrentToken(const string& email) {
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
ResetFunc::ResetFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> ResetFunc::execute() {
    cout << "[ResetFunc] Clearing all collections..." << endl;

    try {
        json body = json::object();
        HttpResponse resp = factory->getHttpClient()->post("/api/test/reset", body);

        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getU().clear();
            factory->getT().clear();
            factory->getRoles().clear();
            factory->getP().clear();
            factory->getStock().clear();
            factory->getSellers().clear();
            factory->getC().clear();
            factory->getO().clear();
            factory->getOrderStatus().clear();
            factory->getRev().clear();
            return make_unique<Num>(200);
        }
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[ResetFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_U ==========
GetUFunc::GetUFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

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
SetUFunc::SetUFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

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

// ========== GET_T ==========
GetTFunc::GetTFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

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
SetTFunc::SetTFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

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
        cerr << "[SetTFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_ROLES ==========
GetRolesFunc::GetRolesFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetRolesFunc::execute() {
    cout << "[GetRolesFunc] Fetching Roles..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Roles");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getRoles().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getRoles()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetRolesFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_ROLES ==========
SetRolesFunc::SetRolesFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

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
        cerr << "[SetRolesFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_P ==========
GetPFunc::GetPFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetPFunc::execute() {
    cout << "[GetPFunc] Fetching P (Products)..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_P");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getP().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getP()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetPFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_P ==========
SetPFunc::SetPFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> SetPFunc::execute() {
    cout << "[SetPFunc] Setting P..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getP().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getP()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_P", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetPFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_STOCK ==========
GetStockFunc::GetStockFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetStockFunc::execute() {
    cout << "[GetStockFunc] Fetching Stock..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Stock");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getStock().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                int value = v.get<int>();
                factory->getStock()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<Num>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetStockFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_STOCK ==========
SetStockFunc::SetStockFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> SetStockFunc::execute() {
    cout << "[SetStockFunc] Setting Stock..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getStock().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getStock()[k] = v.get<int>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_Stock", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetStockFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_SELLERS ==========
GetSellersFunc::GetSellersFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetSellersFunc::execute() {
    cout << "[GetSellersFunc] Fetching Sellers..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Sellers");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getSellers().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getSellers()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetSellersFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_SELLERS ==========
SetSellersFunc::SetSellersFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> SetSellersFunc::execute() {
    cout << "[SetSellersFunc] Setting Sellers..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getSellers().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getSellers()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_Sellers", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetSellersFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_C (Carts) ==========
GetCFunc::GetCFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetCFunc::execute() {
    cout << "[GetCFunc] Fetching C (Carts)..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_C");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getC().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getC()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetCFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_C ==========
SetCFunc::SetCFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> SetCFunc::execute() {
    cout << "[SetCFunc] Setting C..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getC().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getC()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_C", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetCFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_O (Orders) ==========
GetOFunc::GetOFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetOFunc::execute() {
    cout << "[GetOFunc] Fetching O (Orders)..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_O");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getO().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getO()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetOFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_O ==========
SetOFunc::SetOFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> SetOFunc::execute() {
    cout << "[SetOFunc] Setting O..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getO().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getO()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_O", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetOFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_ORDERSTATUS ==========
GetOrderStatusFunc::GetOrderStatusFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetOrderStatusFunc::execute() {
    cout << "[GetOrderStatusFunc] Fetching OrderStatus..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_OrderStatus");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getOrderStatus().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getOrderStatus()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetOrderStatusFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_ORDERSTATUS ==========
SetOrderStatusFunc::SetOrderStatusFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> SetOrderStatusFunc::execute() {
    cout << "[SetOrderStatusFunc] Setting OrderStatus..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getOrderStatus().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getOrderStatus()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_OrderStatus", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetOrderStatusFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET_REV ==========
GetRevFunc::GetRevFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetRevFunc::execute() {
    cout << "[GetRevFunc] Fetching Rev..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Rev");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            factory->getRev().clear();

            for (auto& [k, v] : data.items()) {
                string key = k;
                string value = v.get<string>();
                factory->getRev()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    } catch (const exception& e) {
        cerr << "[GetRevFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// ========== SET_REV ==========
SetRevFunc::SetRevFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> SetRevFunc::execute() {
    cout << "[SetRevFunc] Setting Rev..." << endl;
    try {
        json mapData = extractJson(arguments[0]);
        factory->getRev().clear();
        for (auto& [k, v] : mapData.items()) {
            factory->getRev()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_Rev", body);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SetRevFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

/* ============================================================
 * Business API Functions
 * ============================================================ */

// ========== REGISTER BUYER ==========
RegisterBuyerFunc::RegisterBuyerFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> RegisterBuyerFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("registerBuyer requires 3 arguments");

    string email = extractString(arguments[0]);
    string password = extractString(arguments[1]);
    string fullName = extractString(arguments[2]);

    cout << "[RegisterBuyerFunc] Registering buyer: " << email << endl;

    try {
        json body = {
            {"email", email},
            {"password", password},
            {"fullName", fullName},
            {"userType", "Buyer"}  // Backend expects "Buyer"
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/auth/register", body);

        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            // Update local caches only - DO NOT call set_U (it deletes all users!)
            factory->getU()[email] = password;
            factory->getRoles()[email] = "Buyer";
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[RegisterBuyerFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== REGISTER SELLER ==========
RegisterSellerFunc::RegisterSellerFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> RegisterSellerFunc::execute() {
    if (arguments.size() < 5)
        throw runtime_error("registerSeller requires 5 arguments");

    string email = extractString(arguments[0]);
    string password = extractString(arguments[1]);
    string fullName = extractString(arguments[2]);
    string storeName = extractString(arguments[3]);
    string storeDescription = extractString(arguments[4]);

    cout << "[RegisterSellerFunc] Registering seller: " << email << endl;

    try {
        json body = {
            {"email", email},
            {"password", password},
            {"fullName", fullName},
            {"userType", "Seller"},  // Backend expects "Seller"
            {"storeName", storeName},
            {"storeDescription", storeDescription}
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/auth/register", body);

        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            // Update local caches only
            factory->getU()[email] = password;
            factory->getRoles()[email] = "Seller";
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[RegisterSellerFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== LOGIN ==========
LoginFunc::LoginFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> LoginFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("login requires 2 arguments");

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

                // Store in local cache - backend's auth.js now saves token directly
                factory->getT()[email] = token;

                return make_unique<String>(token);
            }
        }
        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[LoginFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== GET ALL PRODUCTS ==========
GetAllProductsFunc::GetAllProductsFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetAllProductsFunc::execute() {
    cout << "[GetAllProductsFunc] Fetching all products..." << endl;

    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/products");

        if (resp.statusCode == 200) {
            json respData = resp.getJson();
            if (respData.contains("products")) {
                cout << "[GetAllProductsFunc] Found " << respData["products"].size() << " products" << endl;
            }
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetAllProductsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET PRODUCT BY ID ==========
GetProductByIdFunc::GetProductByIdFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetProductByIdFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getProductById requires 1 argument");

    string productId = extractString(arguments[0]);

    cout << "[GetProductByIdFunc] Fetching product: " << productId << endl;

    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/products/" + productId);

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetProductByIdFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== CREATE PRODUCT ==========
CreateProductFunc::CreateProductFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> CreateProductFunc::execute() {
    if (arguments.size() < 6)
        throw runtime_error("createProduct requires 6 arguments");

    string sellerEmail = extractString(arguments[0]);
    string title = extractString(arguments[1]);
    string description = extractString(arguments[2]);
    string category = extractString(arguments[3]);
    int price = extractInt(arguments[4]);
    int quantity = extractInt(arguments[5]);

    cout << "[CreateProductFunc] " << sellerEmail << " creating product: " << title << endl;

    try {
        string token = getCurrentToken(sellerEmail);

        if (token.empty()) {
            cerr << "[CreateProductFunc] Error: No token for " << sellerEmail << endl;
            return make_unique<String>("");
        }

        json body = {
            {"title", title},
            {"description", description},
            {"category", category},
            {"price", price},
            {"quantity", quantity}
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/products", body, 
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 201) {
            json respData = resp.getJson();
            if (respData.contains("_id")) {
                string productId = respData["_id"].get<string>();
                factory->getP()[productId] = title;
                factory->getStock()[productId] = quantity;
                factory->getSellers()[productId] = sellerEmail;
                cout << "[CreateProductFunc] Product created: " << productId << endl;
                return make_unique<String>(productId);
            }
        } else {
            cout << "[CreateProductFunc] Error response: " << resp.body << endl;
        }

        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[CreateProductFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== UPDATE PRODUCT ==========
UpdateProductFunc::UpdateProductFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> UpdateProductFunc::execute() {
    if (arguments.size() < 7)
        throw runtime_error("updateProduct requires 7 arguments");

    string sellerEmail = extractString(arguments[0]);
    string productId = extractString(arguments[1]);
    string title = extractString(arguments[2]);
    string description = extractString(arguments[3]);
    string category = extractString(arguments[4]);
    int price = extractInt(arguments[5]);
    int quantity = extractInt(arguments[6]);

    cout << "[UpdateProductFunc] " << sellerEmail << " updating product: " << productId << endl;

    try {
        string token = getCurrentToken(sellerEmail);

        if (token.empty()) {
            cerr << "[UpdateProductFunc] Error: No token for " << sellerEmail << endl;
            return make_unique<Num>(401);
        }

        json body = {
            {"title", title},
            {"description", description},
            {"category", category},
            {"price", price},
            {"quantity", quantity}
        };

        HttpResponse resp = factory->getHttpClient()->put("/api/products/" + productId, body,
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200) {
            factory->getP()[productId] = title;
            factory->getStock()[productId] = quantity;
            cout << "[UpdateProductFunc] Product updated" << endl;
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[UpdateProductFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== DELETE PRODUCT ==========
DeleteProductFunc::DeleteProductFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> DeleteProductFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("deleteProduct requires 2 arguments");

    string sellerEmail = extractString(arguments[0]);
    string productId = extractString(arguments[1]);

    cout << "[DeleteProductFunc] " << sellerEmail << " deleting product: " << productId << endl;

    try {
        string token = getCurrentToken(sellerEmail);

        if (token.empty()) {
            cerr << "[DeleteProductFunc] Error: No token for " << sellerEmail << endl;
            return make_unique<Num>(401);
        }

        HttpResponse resp = factory->getHttpClient()->del("/api/products/" + productId,
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200) {
            factory->getP().erase(productId);
            factory->getStock().erase(productId);
            factory->getSellers().erase(productId);
            cout << "[DeleteProductFunc] Product deleted" << endl;
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[DeleteProductFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET SELLER PRODUCTS ==========
GetSellerProductsFunc::GetSellerProductsFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetSellerProductsFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getSellerProducts requires 1 argument");

    string sellerEmail = extractString(arguments[0]);

    cout << "[GetSellerProductsFunc] Fetching products for: " << sellerEmail << endl;

    try {
        string token = getCurrentToken(sellerEmail);

        if (token.empty()) {
            cerr << "[GetSellerProductsFunc] Error: No token for " << sellerEmail << endl;
            return make_unique<Num>(401);
        }

        HttpResponse resp = factory->getHttpClient()->get("/api/products/seller/inventory",
            {{"Authorization", "Bearer " + token}});

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetSellerProductsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== ADD TO CART ==========
AddToCartFunc::AddToCartFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> AddToCartFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("addToCart requires 3 arguments");

    string buyerEmail = extractString(arguments[0]);
    string productId = extractString(arguments[1]);
    int quantity = extractInt(arguments[2]);

    cout << "[AddToCartFunc] " << buyerEmail << " adding product " << productId << " (qty: " << quantity << ")" << endl;

    try {
        string token = getCurrentToken(buyerEmail);

        if (token.empty()) {
            cerr << "[AddToCartFunc] Error: No token for " << buyerEmail << endl;
            return make_unique<String>("");
        }

        json body = {
            {"productId", productId},
            {"quantity", quantity}
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/cart/add", body,
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200) {
            json respData = resp.getJson();
            if (respData.contains("_id")) {
                string cartId = respData["_id"].get<string>();
                factory->getC()[buyerEmail] = cartId;
                cout << "[AddToCartFunc] Cart updated: " << cartId << endl;
                return make_unique<String>(cartId);
            }
        }

        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[AddToCartFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== GET CART ==========
GetCartFunc::GetCartFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetCartFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getCart requires 1 argument");

    string buyerEmail = extractString(arguments[0]);

    cout << "[GetCartFunc] Fetching cart for: " << buyerEmail << endl;

    try {
        string token = getCurrentToken(buyerEmail);

        if (token.empty()) {
            cerr << "[GetCartFunc] Error: No token for " << buyerEmail << endl;
            return make_unique<Num>(401);
        }

        HttpResponse resp = factory->getHttpClient()->get("/api/cart",
            {{"Authorization", "Bearer " + token}});

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetCartFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== UPDATE CART ==========
UpdateCartFunc::UpdateCartFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> UpdateCartFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("updateCart requires 3 arguments");

    string buyerEmail = extractString(arguments[0]);
    string productId = extractString(arguments[1]);
    int quantity = extractInt(arguments[2]);

    cout << "[UpdateCartFunc] " << buyerEmail << " updating cart, product " << productId << " to qty: " << quantity << endl;

    try {
        string token = getCurrentToken(buyerEmail);

        if (token.empty()) {
            cerr << "[UpdateCartFunc] Error: No token for " << buyerEmail << endl;
            return make_unique<Num>(401);
        }

        json body = {
            {"productId", productId},
            {"quantity", quantity}
        };

        HttpResponse resp = factory->getHttpClient()->put("/api/cart/update", body,
            {{"Authorization", "Bearer " + token}});

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[UpdateCartFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== CREATE ORDER ==========
CreateOrderFunc::CreateOrderFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> CreateOrderFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("createOrder requires 3 arguments");

    string buyerEmail = extractString(arguments[0]);
    string shippingAddressStr = extractString(arguments[1]);
    string paymentMethod = extractString(arguments[2]);

    cout << "[CreateOrderFunc] " << buyerEmail << " placing order..." << endl;

    try {
        string token = getCurrentToken(buyerEmail);

        if (token.empty()) {
            cerr << "[CreateOrderFunc] Error: No token for " << buyerEmail << endl;
            return make_unique<String>("");
        }

        // Parse shipping address (expected format: "street,city,state,zip,country")
        json shippingAddress = {
            {"street", "123 Test St"},
            {"city", "Test City"},
            {"state", "TS"},
            {"zipCode", "12345"},
            {"country", "USA"}
        };

        // Try to parse if comma-separated
        size_t pos = 0;
        vector<string> parts;
        string addr = shippingAddressStr;
        while ((pos = addr.find(",")) != string::npos) {
            parts.push_back(addr.substr(0, pos));
            addr.erase(0, pos + 1);
        }
        parts.push_back(addr);

        if (parts.size() >= 5) {
            shippingAddress = {
                {"street", parts[0]},
                {"city", parts[1]},
                {"state", parts[2]},
                {"zipCode", parts[3]},
                {"country", parts[4]}
            };
        }

        json body = {
            {"shippingAddress", shippingAddress},
            {"paymentMethod", paymentMethod}
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/orders", body,
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 201) {
            json respData = resp.getJson();
            if (respData.contains("_id")) {
                string orderId = respData["_id"].get<string>();
                factory->getO()[orderId] = buyerEmail;
                factory->getOrderStatus()[orderId] = "Pending";
                factory->getC().erase(buyerEmail);  // Cart cleared after order
                cout << "[CreateOrderFunc] Order placed: " << orderId << endl;
                return make_unique<String>(orderId);
            }
        } else {
            cout << "[CreateOrderFunc] Error response: " << resp.body << endl;
        }

        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[CreateOrderFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== GET BUYER ORDERS ==========
GetBuyerOrdersFunc::GetBuyerOrdersFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetBuyerOrdersFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getBuyerOrders requires 1 argument");

    string buyerEmail = extractString(arguments[0]);

    cout << "[GetBuyerOrdersFunc] Fetching orders for buyer: " << buyerEmail << endl;

    try {
        string token = getCurrentToken(buyerEmail);

        if (token.empty()) {
            cerr << "[GetBuyerOrdersFunc] Error: No token for " << buyerEmail << endl;
            return make_unique<Num>(401);
        }

        HttpResponse resp = factory->getHttpClient()->get("/api/orders/buyer",
            {{"Authorization", "Bearer " + token}});

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetBuyerOrdersFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET SELLER ORDERS ==========
GetSellerOrdersFunc::GetSellerOrdersFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetSellerOrdersFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getSellerOrders requires 1 argument");

    string sellerEmail = extractString(arguments[0]);

    cout << "[GetSellerOrdersFunc] Fetching orders for seller: " << sellerEmail << endl;

    try {
        string token = getCurrentToken(sellerEmail);

        if (token.empty()) {
            cerr << "[GetSellerOrdersFunc] Error: No token for " << sellerEmail << endl;
            return make_unique<Num>(401);
        }

        HttpResponse resp = factory->getHttpClient()->get("/api/orders/seller",
            {{"Authorization", "Bearer " + token}});

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetSellerOrdersFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== UPDATE ORDER STATUS ==========
UpdateOrderStatusFunc::UpdateOrderStatusFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> UpdateOrderStatusFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("updateOrderStatus requires 3 arguments");

    string sellerEmail = extractString(arguments[0]);
    string orderId = extractString(arguments[1]);
    string status = extractString(arguments[2]);

    cout << "[UpdateOrderStatusFunc] " << sellerEmail << " updating order " << orderId << " to " << status << endl;

    try {
        string token = getCurrentToken(sellerEmail);

        if (token.empty()) {
            cerr << "[UpdateOrderStatusFunc] Error: No token for " << sellerEmail << endl;
            return make_unique<Num>(401);
        }

        json body = {{"status", status}};

        HttpResponse resp = factory->getHttpClient()->put("/api/orders/" + orderId + "/status", body,
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200) {
            factory->getOrderStatus()[orderId] = status;
            cout << "[UpdateOrderStatusFunc] Order status updated" << endl;
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[UpdateOrderStatusFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== CREATE REVIEW ==========
CreateReviewFunc::CreateReviewFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> CreateReviewFunc::execute() {
    if (arguments.size() < 5)
        throw runtime_error("createReview requires 5 arguments");

    string buyerEmail = extractString(arguments[0]);
    string productId = extractString(arguments[1]);
    string orderId = extractString(arguments[2]);
    int rating = extractInt(arguments[3]);
    string comment = extractString(arguments[4]);

    cout << "[CreateReviewFunc] " << buyerEmail << " reviewing product: " << productId << endl;

    try {
        string token = getCurrentToken(buyerEmail);

        if (token.empty()) {
            cerr << "[CreateReviewFunc] Error: No token for " << buyerEmail << endl;
            return make_unique<String>("");
        }

        json body = {
            {"productId", productId},
            {"orderId", orderId},
            {"rating", rating},
            {"comment", comment}
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/reviews", body,
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 201) {
            json respData = resp.getJson();
            if (respData.contains("_id")) {
                string reviewId = respData["_id"].get<string>();
                factory->getRev()[reviewId] = productId;
                cout << "[CreateReviewFunc] Review created: " << reviewId << endl;
                return make_unique<String>(reviewId);
            }
        } else {
            cout << "[CreateReviewFunc] Error response: " << resp.body << endl;
        }

        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[CreateReviewFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== GET PRODUCT REVIEWS ==========
GetProductReviewsFunc::GetProductReviewsFunc(EcommerceFunctionFactory* factory, vector<Expr*> args)
    : EcommerceAPIFunction(factory, args) {}

unique_ptr<Expr> GetProductReviewsFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getProductReviews requires 1 argument");

    string productId = extractString(arguments[0]);

    cout << "[GetProductReviewsFunc] Fetching reviews for product: " << productId << endl;

    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/reviews/" + productId);

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetProductReviewsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

/* ============================================================
 * EcommerceFunctionFactory Implementation
 * ============================================================ */

EcommerceFunctionFactory::EcommerceFunctionFactory(const string& baseUrl)
    : baseUrl(baseUrl) {
    httpClient = make_unique<HttpClient>(baseUrl);
    cout << "[EcommerceFunctionFactory] Initialized with baseUrl: " << baseUrl << endl;
}

unique_ptr<Function> EcommerceFunctionFactory::getFunction(string fname, vector<Expr*> args) {
    cout << "[Factory] Creating function: " << fname << endl;

    // Test API functions
    if (fname == "reset") return make_unique<ResetFunc>(this, args);
    if (fname == "get_U") return make_unique<GetUFunc>(this, args);
    if (fname == "set_U") return make_unique<SetUFunc>(this, args);
    if (fname == "get_T") return make_unique<GetTFunc>(this, args);
    if (fname == "set_T") return make_unique<SetTFunc>(this, args);
    if (fname == "get_Roles") return make_unique<GetRolesFunc>(this, args);
    if (fname == "set_Roles") return make_unique<SetRolesFunc>(this, args);
    if (fname == "get_P") return make_unique<GetPFunc>(this, args);
    if (fname == "set_P") return make_unique<SetPFunc>(this, args);
    if (fname == "get_Stock") return make_unique<GetStockFunc>(this, args);
    if (fname == "set_Stock") return make_unique<SetStockFunc>(this, args);
    if (fname == "get_Sellers") return make_unique<GetSellersFunc>(this, args);
    if (fname == "set_Sellers") return make_unique<SetSellersFunc>(this, args);
    if (fname == "get_C") return make_unique<GetCFunc>(this, args);
    if (fname == "set_C") return make_unique<SetCFunc>(this, args);
    if (fname == "get_O") return make_unique<GetOFunc>(this, args);
    if (fname == "set_O") return make_unique<SetOFunc>(this, args);
    if (fname == "get_OrderStatus") return make_unique<GetOrderStatusFunc>(this, args);
    if (fname == "set_OrderStatus") return make_unique<SetOrderStatusFunc>(this, args);
    if (fname == "get_Rev") return make_unique<GetRevFunc>(this, args);
    if (fname == "set_Rev") return make_unique<SetRevFunc>(this, args);

    // Business API functions
    if (fname == "registerBuyer") return make_unique<RegisterBuyerFunc>(this, args);
    if (fname == "registerSeller") return make_unique<RegisterSellerFunc>(this, args);
    if (fname == "login") return make_unique<LoginFunc>(this, args);
    if (fname == "getAllProducts") return make_unique<GetAllProductsFunc>(this, args);
    if (fname == "getProductById") return make_unique<GetProductByIdFunc>(this, args);
    if (fname == "createProduct") return make_unique<CreateProductFunc>(this, args);
    if (fname == "updateProduct") return make_unique<UpdateProductFunc>(this, args);
    if (fname == "deleteProduct") return make_unique<DeleteProductFunc>(this, args);
    if (fname == "getSellerProducts") return make_unique<GetSellerProductsFunc>(this, args);
    if (fname == "addToCart") return make_unique<AddToCartFunc>(this, args);
    if (fname == "getCart") return make_unique<GetCartFunc>(this, args);
    if (fname == "updateCart") return make_unique<UpdateCartFunc>(this, args);
    if (fname == "createOrder") return make_unique<CreateOrderFunc>(this, args);
    if (fname == "getBuyerOrders") return make_unique<GetBuyerOrdersFunc>(this, args);
    if (fname == "getSellerOrders") return make_unique<GetSellerOrdersFunc>(this, args);
    if (fname == "updateOrderStatus") return make_unique<UpdateOrderStatusFunc>(this, args);
    if (fname == "createReview") return make_unique<CreateReviewFunc>(this, args);
    if (fname == "getProductReviews") return make_unique<GetProductReviewsFunc>(this, args);

    throw runtime_error("Unknown function: " + fname);
}
}
