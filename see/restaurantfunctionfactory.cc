#include "restaurantfunctionfactory.hh"
#include <iostream>
#include <stdexcept>

using namespace std;

/* ============================================================
 * APIFunction Base Implementation
 * ============================================================ */

APIFunction::APIFunction(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : factory(factory), arguments(args) {}

string APIFunction::extractString(Expr *expr)
{
    if (!expr)
        throw runtime_error("Null expression in extractString");

    if (expr->exprType == ExprType::STRING)
    {
        String *str = dynamic_cast<String *>(expr);
        return str->value;
    }

    if (expr->exprType == ExprType::VAR)
    {
        Var *var = dynamic_cast<Var *>(expr);
        return var->name;
    }

    throw runtime_error("Expected STRING or VAR expression");
}

int APIFunction::extractInt(Expr *expr)
{
    if (!expr)
        throw runtime_error("Null expression in extractInt");

    if (expr->exprType == ExprType::NUM)
    {
        Num *num = dynamic_cast<Num *>(expr);
        return num->value;
    }

    throw runtime_error("Expected NUM expression");
}

json APIFunction::extractJson(Expr *expr)
{
    if (!expr)
        return json::object();

    if (expr->exprType == ExprType::MAP)
    {
        Map *map = dynamic_cast<Map *>(expr);
        json obj = json::object();

        for (const auto &kv : map->value)
        {
            string key = kv.first->name;

            if (kv.second->exprType == ExprType::STRING)
            {
                String *val = dynamic_cast<String *>(kv.second.get());
                obj[key] = val->value;
            }
            else if (kv.second->exprType == ExprType::NUM)
            {
                Num *val = dynamic_cast<Num *>(kv.second.get());
                obj[key] = val->value;
            }
            else
            {
                obj[key] = "unsupported_type";
            }
        }

        return obj;
    }

    if (expr->exprType == ExprType::STRING)
    {
        String *str = dynamic_cast<String *>(expr);
        try
        {
            return json::parse(str->value);
        }
        catch (...)
        {
            return str->value;
        }
    }

    return json::object();
}

string APIFunction::getCurrentToken(const string &email)
{
    // First check local cache
    auto it = factory->getT().find(email);
    if (it != factory->getT().end() && !it->second.empty())
    {
        return it->second;
    }

    // If not found locally, fetch from backend
    HttpResponse resp = factory->getHttpClient()->get("/api/test/get_T");
    if (resp.statusCode == 200)
    {
        json data = resp.getJson();
        for (auto &[k, v] : data.items())
        {
            factory->getT()[k] = v.get<string>();
        }

        // Try again after refresh
        it = factory->getT().find(email);
        if (it != factory->getT().end())
        {
            return it->second;
        }
    }

    return "";
}

/* ============================================================
 * Test API Functions
 * ============================================================ */

ResetFunc::ResetFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> ResetFunc::execute()
{
    cout << "[ResetFunc] Clearing all collections..." << endl;

    try
    {
        json body = json::object();
        HttpResponse resp = factory->getHttpClient()->post("/api/test/reset", body);

        if (resp.statusCode >= 200 && resp.statusCode < 300)
        {
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
    }
    catch (const exception &e)
    {
        cerr << "[ResetFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_U
GetUFunc::GetUFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetUFunc::execute()
{
    cout << "[GetUFunc] Fetching U..." << endl;
    try
    {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_U");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200)
        {
            json data = resp.getJson();
            factory->getU().clear();

            for (auto &[k, v] : data.items())
            {
                string key = k;
                string value = v.get<string>();
                factory->getU()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    }
    catch (const exception &e)
    {
        cerr << "[GetUFunc] Error: " << e.what() << endl;
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// set_U
SetUFunc::SetUFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetUFunc::execute()
{
    cout << "[SetUFunc] Setting U..." << endl;
    try
    {
        json mapData = extractJson(arguments[0]);
        factory->getU().clear();
        for (auto &[k, v] : mapData.items())
        {
            factory->getU()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_U", body);
        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[SetUFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_T
GetTFunc::GetTFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}
unique_ptr<Expr> GetTFunc::execute()
{
    cout << "[GetTFunc] Fetching T..." << endl;
    try
    {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_T");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200)
        {
            json data = resp.getJson();
            factory->getT().clear();

            for (auto &[k, v] : data.items())
            {
                string key = k;
                string value = v.get<string>();
                factory->getT()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    }
    catch (const exception &e)
    {
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// set_T
SetTFunc::SetTFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetTFunc::execute()
{
    cout << "[SetTFunc] Setting T..." << endl;
    try
    {
        json mapData = extractJson(arguments[0]);
        factory->getT().clear();
        for (auto &[k, v] : mapData.items())
        {
            factory->getT()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_T", body);
        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        return make_unique<Num>(500);
    }
}

// get_C
GetCFunc::GetCFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetCFunc::execute()
{
    cout << "[GetCFunc] Fetching C..." << endl;

    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

    try
    {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_C");

        if (resp.statusCode == 200)
        {
            json data = resp.getJson();
            factory->getC().clear();

            for (auto &[k, v] : data.items())
            {
                string key = k;
                string value = v.dump(); // Store as JSON string
                factory->getC()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    }
    catch (const exception &e)
    {
        cerr << "[GetCFunc] Error: " << e.what() << endl;
        return make_unique<Map>(std::move(pairs));
    }
}

// set_C
SetCFunc::SetCFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetCFunc::execute()
{
    cout << "[SetCFunc] Setting C..." << endl;
    try
    {
        json mapData = extractJson(arguments[0]);
        factory->getC().clear();
        for (auto &[k, v] : mapData.items())
        {
            factory->getC()[k] = v.dump();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_C", body);
        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[SetCFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_R
GetRFunc::GetRFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetRFunc::execute()
{
    cout << "[GetRFunc] Fetching R..." << endl;

    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

    try
    {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_R");

        if (resp.statusCode == 200)
        {
            json data = resp.getJson();
            factory->getR().clear();

            for (auto &[k, v] : data.items())
            {
                string key = k;
                string value = v.dump();
                factory->getR()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    }
    catch (const exception &e)
    {
        cerr << "[GetRFunc] Error: " << e.what() << endl;
        return make_unique<Map>(std::move(pairs));
    }
}

// set_R
SetRFunc::SetRFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetRFunc::execute()
{
    cout << "[SetRFunc] Setting R..." << endl;
    try
    {
        json mapData = extractJson(arguments[0]);
        factory->getR().clear();
        for (auto &[k, v] : mapData.items())
        {
            factory->getR()[k] = v.dump();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_R", body);
        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[SetRFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_M
GetMFunc::GetMFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

// REPLACE GetMFunc::execute() with this implementation:

unique_ptr<Expr> GetMFunc::execute()
{
    cout << "[GetMFunc] Fetching M..." << endl;

    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

    try
    {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_M");

        if (resp.statusCode == 200)
        {
            json data = resp.getJson();
            factory->getM().clear();

            // data format: { restaurantId: [{id, name, price, category}, ...], ... }
            // We need to return: { menuItemId: restaurantId, ... }
            // This allows findKeyFromMapInSigma("tmp_M_") to return menuItemId

            for (auto &[restaurantId, menuItems] : data.items())
            {
                if (menuItems.is_array())
                {
                    for (auto &item : menuItems)
                    {
                        if (item.contains("id"))
                        {
                            string menuItemId = item["id"].get<string>();
                            // Store menuItemId -> restaurantId mapping
                            factory->getM()[menuItemId] = restaurantId;

                            pairs.push_back(make_pair(
                                make_unique<Var>(menuItemId),
                                make_unique<String>(restaurantId)));

                            cout << "[GetMFunc] Menu item: " << menuItemId
                                 << " -> restaurant: " << restaurantId << endl;
                        }
                    }
                }
            }
        }

        return make_unique<Map>(std::move(pairs));
    }
    catch (const exception &e)
    {
        cerr << "[GetMFunc] Error: " << e.what() << endl;
        return make_unique<Map>(std::move(pairs));
    }
}

// set_M
SetMFunc::SetMFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetMFunc::execute()
{
    cout << "[SetMFunc] Setting M..." << endl;
    try
    {
        json mapData = extractJson(arguments[0]);
        factory->getM().clear();
        for (auto &[k, v] : mapData.items())
        {
            factory->getM()[k] = v.dump();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_M", body);
        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[SetMFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_O
GetOFunc::GetOFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetOFunc::execute()
{
    cout << "[GetOFunc] Fetching O..." << endl;

    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

    try
    {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_O");

        if (resp.statusCode == 200)
        {
            json data = resp.getJson();
            factory->getO().clear();

            for (auto &[k, v] : data.items())
            {
                string key = k;
                string value = v.dump();
                factory->getO()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    }
    catch (const exception &e)
    {
        cerr << "[GetOFunc] Error: " << e.what() << endl;
        return make_unique<Map>(std::move(pairs));
    }
}

// set_O
SetOFunc::SetOFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetOFunc::execute()
{
    cout << "[SetOFunc] Setting O..." << endl;
    try
    {
        json mapData = extractJson(arguments[0]);
        factory->getO().clear();
        for (auto &[k, v] : mapData.items())
        {
            factory->getO()[k] = v.dump();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_O", body);
        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[SetOFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_Rev
GetRevFunc::GetRevFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetRevFunc::execute()
{
    cout << "[GetRevFunc] Fetching Rev..." << endl;

    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

    try
    {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Rev");

        if (resp.statusCode == 200)
        {
            json data = resp.getJson();
            factory->getRev().clear();

            for (auto &[k, v] : data.items())
            {
                string key = k;
                string value = v.dump();
                factory->getRev()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    }
    catch (const exception &e)
    {
        cerr << "[GetRevFunc] Error: " << e.what() << endl;
        return make_unique<Map>(std::move(pairs));
    }
}

// set_Rev
SetRevFunc::SetRevFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetRevFunc::execute()
{
    cout << "[SetRevFunc] Setting Rev..." << endl;
    try
    {
        json mapData = extractJson(arguments[0]);
        factory->getRev().clear();
        for (auto &[k, v] : mapData.items())
        {
            factory->getRev()[k] = v.dump();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_Rev", body);
        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[SetRevFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// get_Roles
GetRolesFunc::GetRolesFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetRolesFunc::execute()
{
    cout << "[GetRolesFunc] Fetching Roles..." << endl;
    try
    {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Roles");

        // Build a Map expression to return
        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200)
        {
            json data = resp.getJson();
            factory->getRoles().clear();

            for (auto &[k, v] : data.items())
            {
                string key = k;
                string value = v.get<string>();

                factory->getRoles()[key] = value;

                // Add to AST Map
                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        // Return a Map expression (even if empty)
        return make_unique<Map>(std::move(pairs));
    }
    catch (const exception &e)
    {
        cerr << "[GetRolesFunc] Error: " << e.what() << endl;
        // Return empty map on error
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// set_Roles
SetRolesFunc::SetRolesFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetRolesFunc::execute()
{
    cout << "[SetRolesFunc] Setting Roles..." << endl;
    try
    {
        json mapData = extractJson(arguments[0]);
        factory->getRoles().clear();
        for (auto &[k, v] : mapData.items())
        {
            factory->getRoles()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_Roles", body);
        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        return make_unique<Num>(500);
    }
}

// get_C, set_C, get_R, set_R, get_M, set_M, get_O, set_O, get_Rev, set_Rev
// will do next...

// get_Owners
GetOwnersFunc::GetOwnersFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> GetOwnersFunc::execute()
{
    cout << "[GetOwnersFunc] Fetching Owners..." << endl;
    try
    {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Owners");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200)
        {
            json data = resp.getJson();
            factory->getOwners().clear();

            for (auto &[k, v] : data.items())
            {
                string key = k;
                string value = v.get<string>();
                factory->getOwners()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    }
    catch (const exception &e)
    {
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}

// set_Owners
SetOwnersFunc::SetOwnersFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetOwnersFunc::execute()
{
    cout << "[SetOwnersFunc] Setting Owners..." << endl;
    try
    {
        json mapData = extractJson(arguments[0]);
        factory->getOwners().clear();
        for (auto &[k, v] : mapData.items())
        {
            factory->getOwners()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_Owners", body);
        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        return make_unique<Num>(500);
    }
}

// get_Assignments
GetAssignmentsFunc::GetAssignmentsFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)

    : APIFunction(factory, args)
{
}

unique_ptr<Expr> GetAssignmentsFunc::execute()
{
    cout << "[GetAssignmentsFunc] Fetching Assignments..." << endl;
    try
    {
        HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Assignments");

        vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

        if (resp.statusCode == 200)
        {
            json data = resp.getJson();
            factory->getAssignments().clear();

            for (auto &[k, v] : data.items())
            {
                string key = k;
                string value = v.get<string>();
                factory->getAssignments()[key] = value;

                pairs.push_back(make_pair(
                    make_unique<Var>(key),
                    make_unique<String>(value)));
            }
        }

        return make_unique<Map>(std::move(pairs));
    }
    catch (const exception &e)
    {
        return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
    }
}
// set_Assignments
SetAssignmentsFunc::SetAssignmentsFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> SetAssignmentsFunc::execute()
{
    cout << "[SetAssignmentsFunc] Setting Assignments..." << endl;
    try
    {
        json mapData = extractJson(arguments[0]);
        factory->getAssignments().clear();
        for (auto &[k, v] : mapData.items())
        {
            factory->getAssignments()[k] = v.get<string>();
        }
        json body = {{"data", mapData}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/set_Assignments", body);
        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        return make_unique<Num>(500);
    }
}

/* ============================================================
 * Business API Functions
 * ============================================================ */
// ========== CUSTOMER REGISTRATION ==========
RegisterCustomerFunc::RegisterCustomerFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> RegisterCustomerFunc::execute()
{
    if (arguments.size() < 4)
        throw runtime_error("registerCustomer requires 4 arguments");

    string email = extractString(arguments[0]);
    string password = extractString(arguments[1]);
    string fullName = extractString(arguments[2]);
    string mobile = extractString(arguments[3]);

    cout << "[RegisterCustomerFunc] Registering customer: " << email << endl;

    try
    {
        json body = {
            {"email", email},
            {"password", password},
            {"fullName", fullName},
            {"mobile", mobile},
            {"role", "customer"}};

        HttpResponse resp = factory->getHttpClient()->post("/api/auth/register", body);

        if (resp.statusCode >= 200 && resp.statusCode < 300)
        {
            // Update local caches only - DO NOT call set_U (it deletes all users!)
            factory->getU()[email] = password;
            factory->getRoles()[email] = "customer";
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[RegisterCustomerFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== OWNER REGISTRATION (role = "restaurant_owner") ==========
RegisterOwnerFunc::RegisterOwnerFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> RegisterOwnerFunc::execute()
{
    if (arguments.size() < 4)
        throw runtime_error("registerOwner requires 4 arguments");

    string email = extractString(arguments[0]);
    string password = extractString(arguments[1]);
    string fullName = extractString(arguments[2]);
    string mobile = extractString(arguments[3]);

    cout << "[RegisterOwnerFunc] Registering owner: " << email << endl;

    try
    {
        json body = {
            {"email", email},
            {"password", password},
            {"fullName", fullName},
            {"mobile", mobile},
            {"role", "restaurant_owner"}};

        HttpResponse resp = factory->getHttpClient()->post("/api/auth/register", body);

        if (resp.statusCode >= 200 && resp.statusCode < 300)
        {
            // Update local caches only - DO NOT call set_U (it deletes all users!)
            factory->getU()[email] = password;
            factory->getRoles()[email] = "restaurant_owner";
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[RegisterOwnerFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== AGENT REGISTRATION (role = "delivery_agent") ==========
RegisterAgentFunc::RegisterAgentFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> RegisterAgentFunc::execute()
{
    if (arguments.size() < 4)
        throw runtime_error("registerAgent requires 4 arguments");

    string email = extractString(arguments[0]);
    string password = extractString(arguments[1]);
    string fullName = extractString(arguments[2]);
    string mobile = extractString(arguments[3]);

    cout << "[RegisterAgentFunc] Registering agent: " << email << endl;

    try
    {
        json body = {
            {"email", email},
            {"password", password},
            {"fullName", fullName},
            {"mobile", mobile},
            {"role", "delivery_agent"}};

        HttpResponse resp = factory->getHttpClient()->post("/api/auth/register", body);

        if (resp.statusCode >= 200 && resp.statusCode < 300)
        {
            // Update local caches only - DO NOT call set_U (it deletes all users!)
            factory->getU()[email] = password;
            factory->getRoles()[email] = "delivery_agent";
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[RegisterAgentFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}
// ========== LOGIN ==========
LoginFunc::LoginFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> LoginFunc::execute()
{
    if (arguments.size() < 2)
        throw runtime_error("login requires 2 arguments");

    string email = extractString(arguments[0]);
    string password = extractString(arguments[1]);

    cout << "[LoginFunc] Logging in: " << email << endl;

    try
    {
        json body = {
            {"email", email},
            {"password", password}};

        HttpResponse resp = factory->getHttpClient()->post("/api/auth/login", body);

        if (resp.statusCode == 200)
        {
            json respData = resp.getJson();

            if (respData.contains("token"))
            {
                string token = respData["token"].get<string>();
                cout << "[LoginFunc] Token received for: " << email << endl;

                // Store in local cache only - backend auth.js now saves token directly
                factory->getT()[email] = token;

                // REMOVED: Don't call set_T here - it overwrites other users' tokens
                // The backend's auth.js now saves the token to the User document directly

                return make_unique<String>(token);
            }
        }
        return make_unique<String>("");
    }
    catch (const exception &e)
    {
        cerr << "[LoginFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== BROWSE RESTAURANTS ==========
BrowseRestaurantsFunc::BrowseRestaurantsFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> BrowseRestaurantsFunc::execute()
{
    if (arguments.size() < 1)
        throw runtime_error("browseRestaurants requires 1 argument");

    string email = extractString(arguments[0]);

    cout << "[BrowseRestaurantsFunc] Fetching restaurants for " << email << endl;

    try
    {
        string token = getCurrentToken(email);

        if (token.empty())
        {
            cerr << "[BrowseRestaurantsFunc] Error: No token for " << email << endl;
            return make_unique<Num>(401);
        }

        HttpResponse resp = factory->getHttpClient()->get("/api/restaurants", {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200)
        {
            json respData = resp.getJson();
            cout << "[BrowseRestaurantsFunc] Found " << respData["restaurants"].size() << " restaurants" << endl;
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[BrowseRestaurantsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== VIEW MENU ==========
ViewMenuFunc::ViewMenuFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> ViewMenuFunc::execute()
{
    if (arguments.size() < 2)
        throw runtime_error("viewMenu requires 2 arguments");

    string email = extractString(arguments[0]);
    string restaurantId = extractString(arguments[1]);

    cout << "[ViewMenuFunc] " << email << " viewing menu for restaurant: " << restaurantId << endl;

    try
    {
        string token = getCurrentToken(email);

        if (token.empty())
        {
            cerr << "[ViewMenuFunc] Error: No token for " << email << endl;
            return make_unique<Num>(401);
        }

        HttpResponse resp = factory->getHttpClient()->get(
            "/api/restaurants/" + restaurantId + "/menu",
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200)
        {
            json menuItems = resp.getJson();
            cout << "[ViewMenuFunc] Found " << menuItems.size() << " menu items" << endl;
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[ViewMenuFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== ADD TO CART ==========
AddToCartFunc::AddToCartFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> AddToCartFunc::execute()
{
    if (arguments.size() < 3)
        throw runtime_error("addToCart requires 3 arguments");

    string email = extractString(arguments[0]);
    string menuItemId = extractString(arguments[1]);
    int quantity = extractInt(arguments[2]);

    cout << "[AddToCartFunc] " << email << " adding item " << menuItemId << " (qty: " << quantity << ")" << endl;

    try
    {
        string token = getCurrentToken(email);

        if (token.empty())
        {
            cerr << "[AddToCartFunc] Error: No token for " << email << endl;
            return make_unique<Num>(401);
        }

        json body = {
            {"menuItemId", menuItemId},
            {"quantity", quantity}};

        HttpResponse resp = factory->getHttpClient()->post("/api/cart", body, {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200)
        {
            json respData = resp.getJson();
            if (respData.contains("cart"))
            {
                factory->getC()[email] = respData["cart"].dump();
            }
            cout << "[AddToCartFunc] Item added to cart" << endl;
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[AddToCartFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== PLACE ORDER ==========

PlaceOrderFunc::PlaceOrderFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> PlaceOrderFunc::execute()
{
    if (arguments.size() < 3)
        throw runtime_error("placeOrder requires 3 arguments");

    string email = extractString(arguments[0]);
    string deliveryAddress = extractString(arguments[1]);
    string paymentMethod = extractString(arguments[2]);

    cout << "[PlaceOrderFunc] " << email << " placing order..." << endl;

    try
    {
        string token = getCurrentToken(email);

        if (token.empty())
        {
            cerr << "[PlaceOrderFunc] Error: No token for " << email << endl;
            return make_unique<Num>(401);
        }

        json body = {
            {"deliveryAddress", {{"street", deliveryAddress}, {"city", "TestCity"}, {"state", "TestState"}, {"zipCode", "12345"}}},
            {"paymentMethod", paymentMethod}};

        HttpResponse resp = factory->getHttpClient()->post("/api/orders", body, {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 201)
        {
            json respData = resp.getJson();
            if (respData.contains("order") && respData["order"].contains("_id"))
            {
                string orderId = respData["order"]["_id"].get<string>();
                factory->getO()[orderId] = respData["order"].dump();
                factory->getC().erase(email); // Clear cart after order
                cout << "[PlaceOrderFunc] Order placed: " << orderId << endl;
                return make_unique<String>(orderId); // Return order ID on success!
            }
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[PlaceOrderFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}
// ========== LEAVE REVIEW ==========
LeaveReviewFunc::LeaveReviewFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> LeaveReviewFunc::execute()
{
    if (arguments.size() < 5)
        throw runtime_error("leaveReview requires 5 arguments");

    string email = extractString(arguments[0]);
    string orderId = extractString(arguments[1]);
    int restaurantRating = extractInt(arguments[2]);
    int deliveryRating = extractInt(arguments[3]);
    string comment = extractString(arguments[4]);

    cout << "[LeaveReviewFunc] " << email << " leaving review for order: " << orderId << endl;

    try
    {
        string token = getCurrentToken(email);

        if (token.empty())
        {
            cerr << "[LeaveReviewFunc] Error: No token for " << email << endl;
            return make_unique<Num>(401);
        }

        json body = {
            {"orderId", orderId},
            {"restaurantRating", restaurantRating},
            {"deliveryRating", deliveryRating},
            {"restaurantComment", comment},
            {"deliveryComment", comment}};

        HttpResponse resp = factory->getHttpClient()->post("/api/reviews", body, {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 201)
        {
            json respData = resp.getJson();
            if (respData.contains("review") && respData["review"].contains("_id"))
            {
                string reviewId = respData["review"]["_id"].get<string>();
                factory->getRev()[reviewId] = respData["review"].dump();
                cout << "[LeaveReviewFunc] Review created: " << reviewId << endl;

                // Return the review ID (not the status code) - spec expects this
                return make_unique<String>(reviewId);
            }
        }

        // Return status code on failure
        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[LeaveReviewFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== CREATE RESTAURANT (OWNER) ==========
CreateRestaurantFunc::CreateRestaurantFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> CreateRestaurantFunc::execute()
{
    if (arguments.size() < 4)
        throw runtime_error("createRestaurant requires 4 arguments");

    string email = extractString(arguments[0]);
    string name = extractString(arguments[1]);
    string address = extractString(arguments[2]);
    string contact = extractString(arguments[3]);

    cout << "[CreateRestaurantFunc] " << email << " creating restaurant: " << name << endl;

    try
    {
        string token = getCurrentToken(email);

        if (token.empty())
        {
            cerr << "[CreateRestaurantFunc] Error: No token for " << email << endl;
            return make_unique<Num>(401);
        }

        json body = {
            {"name", name},
            {"address", {{"street", address}, {"city", "TestCity"}, {"state", "TestState"}, {"zipCode", "12345"}}},
            {"contact", {
                            {"phone", "1234567890"}, // ← Use a valid phone number
                            {"email", contact}       // ← Use contact as email instead
                        }},
            {"cuisineTypes", json::array({"Indian", "Continental"})},
            {"hours", {{"opening", "09:00"}, {"closing", "22:00"}}}};

        cout << "[CreateRestaurantFunc] Request body: " << body.dump(2) << endl; // Debug

        HttpResponse resp = factory->getHttpClient()->post("/api/restaurants", body, {{"Authorization", "Bearer " + token}});

        cout << "[CreateRestaurantFunc] Response status: " << resp.statusCode << endl; // Debug

        if (resp.statusCode == 201)
        {
            json respData = resp.getJson();
            if (respData.contains("restaurant") && respData["restaurant"].contains("_id"))
            {
                string restaurantId = respData["restaurant"]["_id"].get<string>();
                factory->getR()[restaurantId] = respData["restaurant"].dump();
                factory->getOwners()[restaurantId] = email;
                cout << "[CreateRestaurantFunc] Restaurant created: " << restaurantId << endl;

                // Return the restaurant ID as a string so it can be used later
                return make_unique<String>(restaurantId);
            }
        }
        else
        {
            // Print response body for debugging
            cout << "[CreateRestaurantFunc] Error response: " << resp.body << endl;
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[CreateRestaurantFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== ADD MENU ITEM (OWNER) ==========
AddMenuItemFunc::AddMenuItemFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> AddMenuItemFunc::execute()
{
    if (arguments.size() < 4)
        throw runtime_error("addMenuItem requires 4 arguments");

    string email = extractString(arguments[0]);
    string restaurantId = extractString(arguments[1]);
    string name = extractString(arguments[2]);
    int price = extractInt(arguments[3]);

    cout << "[AddMenuItemFunc] " << email << " adding menu item: " << name << " to restaurant " << restaurantId << endl;

    try
    {
        string token = getCurrentToken(email);

        if (token.empty())
        {
            cerr << "[AddMenuItemFunc] Error: No token for " << email << endl;
            return make_unique<Num>(401);
        }

        json body = {
            {"restaurantId", restaurantId},
            {"name", name},
            {"description", "Test menu item"},
            {"category", "Main Course"},
            {"price", price},
            {"isVegetarian", true}};

        HttpResponse resp = factory->getHttpClient()->post("/api/menu", body, {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 201)
        {
            json respData = resp.getJson();
            if (respData.contains("menuItem") && respData["menuItem"].contains("_id"))
            {
                string menuItemId = respData["menuItem"]["_id"].get<string>();
                factory->getM()[menuItemId] = respData["menuItem"].dump();
                cout << "[AddMenuItemFunc] Menu item created: " << menuItemId << endl;
            }
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[AddMenuItemFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== ASSIGN ORDER (OWNER) ==========
AssignOrderFunc::AssignOrderFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> AssignOrderFunc::execute()
{
    if (arguments.size() < 3)
        throw runtime_error("assignOrder requires 3 arguments");

    string ownerEmail = extractString(arguments[0]);
    string orderId = extractString(arguments[1]);
    string agentEmail = extractString(arguments[2]);

    cout << "[AssignOrderFunc] " << ownerEmail << " assigning order " << orderId << " to agent " << agentEmail << endl;

    try
    {
        string token = getCurrentToken(ownerEmail);

        if (token.empty())
        {
            cerr << "[AssignOrderFunc] Error: No token for " << ownerEmail << endl;
            return make_unique<Num>(401);
        }

        // First, get the agent's user ID using test API endpoint
        HttpResponse agentResp = factory->getHttpClient()->get("/api/test/get_user_id/" + agentEmail);

        string agentId = "";
        if (agentResp.statusCode == 200)
        {
            json agentData = agentResp.getJson();
            if (agentData.contains("userId"))
            {
                agentId = agentData["userId"].get<string>();
                cout << "[AssignOrderFunc] Found agent ID: " << agentId << " for email: " << agentEmail << endl;
            }
        }

        if (agentId.empty())
        {
            cerr << "[AssignOrderFunc] Error: Could not find agent ID for " << agentEmail << endl;
            return make_unique<Num>(404);
        }

        json body = {
            {"deliveryAgentId", agentId}};

        HttpResponse resp = factory->getHttpClient()->put("/api/orders/" + orderId + "/assign", body, {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200)
        {
            factory->getAssignments()[orderId] = agentEmail;
            cout << "[AssignOrderFunc] Order assigned successfully" << endl;
        }
        else
        {
            cerr << "[AssignOrderFunc] Error response: " << resp.body << endl;
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[AssignOrderFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== UPDATE ORDER STATUS (OWNER) ==========
UpdateOrderStatusOwnerFunc::UpdateOrderStatusOwnerFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> UpdateOrderStatusOwnerFunc::execute()
{
    if (arguments.size() < 3)
        throw runtime_error("updateOrderStatusOwner requires 3 arguments");

    string email = extractString(arguments[0]);
    string orderId = extractString(arguments[1]);
    string status = extractString(arguments[2]);

    cout << "[UpdateOrderStatusOwnerFunc] " << email << " updating order " << orderId << " to " << status << endl;

    try
    {
        string token = getCurrentToken(email);

        if (token.empty())
        {
            cerr << "[UpdateOrderStatusOwnerFunc] Error: No token for " << email << endl;
            return make_unique<Num>(401);
        }

        json body = {{"status", status}};

        HttpResponse resp = factory->getHttpClient()->put("/api/orders/" + orderId + "/status", body, {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200)
        {
            cout << "[UpdateOrderStatusOwnerFunc] Order status updated" << endl;
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[UpdateOrderStatusOwnerFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== UPDATE ORDER STATUS (AGENT) ==========
UpdateOrderStatusAgentFunc::UpdateOrderStatusAgentFunc(RestaurantFunctionFactory *factory, vector<Expr *> args)
    : APIFunction(factory, args) {}

unique_ptr<Expr> UpdateOrderStatusAgentFunc::execute()
{
    if (arguments.size() < 3)
        throw runtime_error("updateOrderStatusAgent requires 3 arguments");

    string email = extractString(arguments[0]);
    string orderId = extractString(arguments[1]);
    string status = extractString(arguments[2]);

    cout << "[UpdateOrderStatusAgentFunc] " << email << " updating order " << orderId << " to " << status << endl;

    try
    {
        string token = getCurrentToken(email);

        if (token.empty())
        {
            cerr << "[UpdateOrderStatusAgentFunc] Error: No token for " << email << endl;
            return make_unique<Num>(401);
        }

        json body = {{"status", status}};

        HttpResponse resp = factory->getHttpClient()->put("/api/orders/" + orderId + "/status", body, {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200)
        {
            cout << "[UpdateOrderStatusAgentFunc] Order status updated" << endl;
        }

        return make_unique<Num>(resp.statusCode);
    }
    catch (const exception &e)
    {
        cerr << "[UpdateOrderStatusAgentFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}
/* ============================================================
 * RestaurantFunctionFactory Implementation
 * ============================================================ */

RestaurantFunctionFactory::RestaurantFunctionFactory(const string &baseUrl)
    : baseUrl(baseUrl)
{
    httpClient = make_unique<HttpClient>(baseUrl);
    cout << "[RestaurantFunctionFactory] Initialized with baseUrl: " << baseUrl << endl;
}

unique_ptr<Function> RestaurantFunctionFactory::getFunction(string fname, vector<Expr *> args)
{
    cout << "[Factory] Creating function: " << fname << endl;

    // Test API functions
    if (fname == "reset")
        return make_unique<ResetFunc>(this, args);
    if (fname == "get_U")
        return make_unique<GetUFunc>(this, args);
    if (fname == "set_U")
        return make_unique<SetUFunc>(this, args);
    if (fname == "get_T")
        return make_unique<GetTFunc>(this, args);
    if (fname == "set_T")
        return make_unique<SetTFunc>(this, args);
    if (fname == "get_Roles")
        return make_unique<GetRolesFunc>(this, args);
    if (fname == "set_Roles")
        return make_unique<SetRolesFunc>(this, args);
    if (fname == "get_Owners")
        return make_unique<GetOwnersFunc>(this, args);
    if (fname == "set_Owners")
        return make_unique<SetOwnersFunc>(this, args);
    if (fname == "get_Assignments")
        return make_unique<GetAssignmentsFunc>(this, args);
    if (fname == "set_Assignments")
        return make_unique<SetAssignmentsFunc>(this, args);
    if (fname == "get_C")
        return make_unique<GetCFunc>(this, args);
    if (fname == "set_C")
        return make_unique<SetCFunc>(this, args);
    if (fname == "get_R")
        return make_unique<GetRFunc>(this, args);
    if (fname == "set_R")
        return make_unique<SetRFunc>(this, args);
    if (fname == "get_M")
        return make_unique<GetMFunc>(this, args);
    if (fname == "set_M")
        return make_unique<SetMFunc>(this, args);
    if (fname == "get_O")
        return make_unique<GetOFunc>(this, args);
    if (fname == "set_O")
        return make_unique<SetOFunc>(this, args);
    if (fname == "get_Rev")
        return make_unique<GetRevFunc>(this, args);
    if (fname == "set_Rev")
        return make_unique<SetRevFunc>(this, args);
    // Business API functions - 3 separate register functions
    if (fname == "registerCustomer")
        return make_unique<RegisterCustomerFunc>(this, args);
    if (fname == "registerOwner")
        return make_unique<RegisterOwnerFunc>(this, args);
    if (fname == "registerAgent")
        return make_unique<RegisterAgentFunc>(this, args);

    if (fname == "login")
        return make_unique<LoginFunc>(this, args);
    if (fname == "browseRestaurants")
        return make_unique<BrowseRestaurantsFunc>(this, args);
    if (fname == "viewMenu")
        return make_unique<ViewMenuFunc>(this, args);
    if (fname == "addToCart")
        return make_unique<AddToCartFunc>(this, args);
    if (fname == "placeOrder")
        return make_unique<PlaceOrderFunc>(this, args);
    if (fname == "leaveReview")
        return make_unique<LeaveReviewFunc>(this, args);
    if (fname == "createRestaurant")
        return make_unique<CreateRestaurantFunc>(this, args);
    if (fname == "addMenuItem")
        return make_unique<AddMenuItemFunc>(this, args);
    if (fname == "assignOrder")
        return make_unique<AssignOrderFunc>(this, args);
    if (fname == "updateOrderStatusOwner")
        return make_unique<UpdateOrderStatusOwnerFunc>(this, args);
    if (fname == "updateOrderStatusAgent")
        return make_unique<UpdateOrderStatusAgentFunc>(this, args);

    throw runtime_error("Unknown function: " + fname);
}
