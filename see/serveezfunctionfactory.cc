#include "serveezfunctionfactory.hh"
#include <iostream>
#include <stdexcept>

using namespace std;
namespace Serveez {

/* ============================================================
 * ServeezAPIFunction Base
 * ============================================================ */

ServeezAPIFunction::ServeezAPIFunction(ServeezFunctionFactory* f, vector<Expr*> a)
    : factory(f), arguments(a) {}

string ServeezAPIFunction::extractString(Expr* expr) {
    if (!expr) throw runtime_error("Null expression in extractString");
    if (expr->exprType == ExprType::STRING) {
        String* s = dynamic_cast<String*>(expr);
        return s->value;
    }
    if (expr->exprType == ExprType::VAR) {
        Var* v = dynamic_cast<Var*>(expr);
        return v->name;
    }
    throw runtime_error("Expected STRING or VAR expression");
}

json ServeezAPIFunction::extractJson(Expr* expr) {
    if (!expr) return json::object();
    if (expr->exprType == ExprType::MAP) {
        Map* m = dynamic_cast<Map*>(expr);
        json obj = json::object();
        for (const auto& kv : m->value) {
            string key = kv.first->name;
            if (kv.second->exprType == ExprType::STRING) {
                String* v = dynamic_cast<String*>(kv.second.get());
                obj[key] = v->value;
            } else if (kv.second->exprType == ExprType::NUM) {
                Num* v = dynamic_cast<Num*>(kv.second.get());
                obj[key] = v->value;
            } else {
                obj[key] = "unsupported_type";
            }
        }
        return obj;
    }
    if (expr->exprType == ExprType::STRING) {
        String* s = dynamic_cast<String*>(expr);
        try { return json::parse(s->value); } catch (...) { return s->value; }
    }
    return json::object();
}

string ServeezAPIFunction::getToken(const string& email) {
    // 1. Check local T_cache
    auto it = factory->getT().find(email);
    if (it != factory->getT().end() && !it->second.empty()) return it->second;
    // 2. Fetch from server
    HttpResponse resp = factory->getHttpClient()->get("/api/test/get_T");
    if (resp.statusCode == 200) {
        json data = resp.getJson();
        for (auto& [k, v] : data.items())
            factory->getT()[k] = v.get<string>();
        auto it2 = factory->getT().find(email);
        if (it2 != factory->getT().end()) return it2->second;
    }
    return "";
}

/* ============================================================
 * Test Helper Functions
 * ============================================================ */

// ─── Reset ───────────────────────────────────────────────────────────────────
unique_ptr<Expr> ResetFunc::execute() {
    cout << "[SV:ResetFunc] Clearing all collections..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->post("/api/test/reset", json::object());
        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getU().clear();
            factory->getP().clear();
            factory->getA().clear();
            factory->getT().clear();
            factory->getC().clear();
            factory->getL().clear();
            factory->getB().clear();
            return make_unique<Num>(200);
        }
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SV:ResetFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ─── GetU / SetU ─────────────────────────────────────────────────────────────
unique_ptr<Expr> GetUFunc::execute() {
    cout << "[SV:GetUFunc] Fetching U..." << endl;
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k,v] : factory->getU())
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    return make_unique<Map>(std::move(pairs));
}
unique_ptr<Expr> SetUFunc::execute() {
    json data = extractJson(arguments[0]);
    factory->getU().clear();
    for (auto& [k,v] : data.items()) factory->getU()[k] = v.get<string>();
    return make_unique<Num>(200);
}

// ─── GetP / SetP ─────────────────────────────────────────────────────────────
unique_ptr<Expr> GetPFunc::execute() {
    cout << "[SV:GetPFunc] Fetching P..." << endl;
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k,v] : factory->getP())
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    return make_unique<Map>(std::move(pairs));
}
unique_ptr<Expr> SetPFunc::execute() {
    json data = extractJson(arguments[0]);
    factory->getP().clear();
    for (auto& [k,v] : data.items()) factory->getP()[k] = v.get<string>();
    return make_unique<Num>(200);
}

// ─── GetA / SetA ─────────────────────────────────────────────────────────────
unique_ptr<Expr> GetAFunc::execute() {
    cout << "[SV:GetAFunc] Fetching A..." << endl;
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k,v] : factory->getA())
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    return make_unique<Map>(std::move(pairs));
}
unique_ptr<Expr> SetAFunc::execute() {
    json data = extractJson(arguments[0]);
    factory->getA().clear();
    for (auto& [k,v] : data.items()) factory->getA()[k] = v.get<string>();
    return make_unique<Num>(200);
}

// ─── GetC / SetC ─────────────────────────────────────────────────────────────
unique_ptr<Expr> GetCFunc::execute() {
    cout << "[SV:GetCFunc] Fetching C..." << endl;
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k,v] : factory->getC())
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    return make_unique<Map>(std::move(pairs));
}
unique_ptr<Expr> SetCFunc::execute() {
    json data = extractJson(arguments[0]);
    factory->getC().clear();
    for (auto& [k,v] : data.items()) factory->getC()[k] = v.get<string>();
    return make_unique<Num>(200);
}

// ─── GetL / SetL ─────────────────────────────────────────────────────────────
unique_ptr<Expr> GetLFunc::execute() {
    cout << "[SV:GetLFunc] Fetching L..." << endl;
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k,v] : factory->getL())
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    return make_unique<Map>(std::move(pairs));
}
unique_ptr<Expr> SetLFunc::execute() {
    json data = extractJson(arguments[0]);
    factory->getL().clear();
    for (auto& [k,v] : data.items()) factory->getL()[k] = v.get<string>();
    return make_unique<Num>(200);
}

// ─── GetB / SetB ─────────────────────────────────────────────────────────────
unique_ptr<Expr> GetBFunc::execute() {
    cout << "[SV:GetBFunc] Fetching B..." << endl;
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k,v] : factory->getB())
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    return make_unique<Map>(std::move(pairs));
}
unique_ptr<Expr> SetBFunc::execute() {
    json data = extractJson(arguments[0]);
    factory->getB().clear();
    for (auto& [k,v] : data.items()) factory->getB()[k] = v.get<string>();
    return make_unique<Num>(200);
}

/* ============================================================
 * Business API Functions
 * ============================================================ */

// ─── RegisterUser (USER role) ─────────────────────────────────────────────────
// args[0] = email
unique_ptr<Expr> RegisterUserFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("registerUser requires 1 argument");

    string email = extractString(arguments[0]);
    cout << "[SV:RegisterUserFunc] Registering USER: " << email << endl;

    try {
        json body = {{"email", email}, {"role", "USER"}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/register", body);

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            string token = data["token"].get<string>();
            factory->getT()[email] = token;
            factory->getU()[email] = email;
            return make_unique<String>(email);
        }

        throw runtime_error("[SV:RegisterUserFunc] API call failed: " +
            to_string(resp.statusCode) + " " + resp.body);
    } catch (const runtime_error&) {
        throw;
    } catch (const exception& e) {
        throw runtime_error(string("[SV:RegisterUserFunc] Error: ") + e.what());
    }
}

// ─── RegisterProvider (PROVIDER role) ─────────────────────────────────────────
// args[0] = email
unique_ptr<Expr> RegisterProviderFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("registerProvider requires 1 argument");

    string email = extractString(arguments[0]);
    cout << "[SV:RegisterProviderFunc] Registering PROVIDER: " << email << endl;

    try {
        json body = {{"email", email}, {"role", "PROVIDER"}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/register", body);

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            string token = data["token"].get<string>();
            factory->getT()[email] = token;
            factory->getP()[email] = email;
            return make_unique<String>(email);
        }

        throw runtime_error("[SV:RegisterProviderFunc] API call failed: " +
            to_string(resp.statusCode) + " " + resp.body);
    } catch (const runtime_error&) {
        throw;
    } catch (const exception& e) {
        throw runtime_error(string("[SV:RegisterProviderFunc] Error: ") + e.what());
    }
}

// ─── RegisterAdmin (ADMIN role) ───────────────────────────────────────────────
// args[0] = email
unique_ptr<Expr> RegisterAdminFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("registerAdmin requires 1 argument");

    string email = extractString(arguments[0]);
    cout << "[SV:RegisterAdminFunc] Registering ADMIN: " << email << endl;

    try {
        json body = {{"email", email}, {"role", "ADMIN"}};
        HttpResponse resp = factory->getHttpClient()->post("/api/test/register", body);

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            string token = data["token"].get<string>();
            factory->getT()[email] = token;
            factory->getA()[email] = email;
            return make_unique<String>(email);
        }

        throw runtime_error("[SV:RegisterAdminFunc] API call failed: " +
            to_string(resp.statusCode) + " " + resp.body);
    } catch (const runtime_error&) {
        throw;
    } catch (const exception& e) {
        throw runtime_error(string("[SV:RegisterAdminFunc] Error: ") + e.what());
    }
}

// ─── CreateCategory ──────────────────────────────────────────────────────────
// args[0] = adminEmail, args[1] = catName
unique_ptr<Expr> CreateCategoryFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("createCategory requires 2 arguments");

    string adminEmail = extractString(arguments[0]);
    string catName    = extractString(arguments[1]);
    cout << "[SV:CreateCategoryFunc] Creating category: " << catName << endl;

    try {
        string token = getToken(adminEmail);
        if (token.empty()) {
            throw runtime_error("[SV:CreateCategoryFunc] No token for " + adminEmail + " — user not registered or login failed");
        }

        json body = {{"name", catName}, {"description", "Test category " + catName}};
        map<string,string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post("/api/admin/categories", body, headers);

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            string catId = data["id"].get<string>();
            factory->getC()[catId] = catId;
            return make_unique<String>(catId);
        }

        throw runtime_error("[SV:CreateCategoryFunc] API call failed: " +
            to_string(resp.statusCode) + " " + resp.body);
    } catch (const runtime_error&) {
        throw;
    } catch (const exception& e) {
        throw runtime_error(string("[SV:CreateCategoryFunc] Error: ") + e.what());
    }
}

// ─── CreateListing ───────────────────────────────────────────────────────────
// args[0] = provEmail, args[1] = catId, args[2] = listingTitle
unique_ptr<Expr> CreateListingFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("createListing requires 3 arguments");

    string provEmail    = extractString(arguments[0]);
    string catId        = extractString(arguments[1]);
    string listingTitle = extractString(arguments[2]);
    cout << "[SV:CreateListingFunc] Creating listing: " << listingTitle << endl;

    try {
        string token = getToken(provEmail);
        if (token.empty()) {
            throw runtime_error("[SV:CreateListingFunc] No token for " + provEmail + " — user not registered or login failed");
        }

        json body = {
            {"categoryId", catId},
            {"title", listingTitle},
            {"description", "Test listing description"},
            {"price", 99.99},
            {"location", "Test City"},
            {"estimatedDuration", 60}
        };
        map<string,string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post("/api/providers/listings", body, headers);

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            string listingId = data["id"].get<string>();
            factory->getL()[listingId] = listingId;
            return make_unique<String>(listingId);
        }

        throw runtime_error("[SV:CreateListingFunc] API call failed: " +
            to_string(resp.statusCode) + " " + resp.body);
    } catch (const runtime_error&) {
        throw;
    } catch (const exception& e) {
        throw runtime_error(string("[SV:CreateListingFunc] Error: ") + e.what());
    }
}

// ─── GetListings ─────────────────────────────────────────────────────────────
unique_ptr<Expr> GetListingsFunc::execute() {
    cout << "[SV:GetListingsFunc] Getting all listings..." << endl;
    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/listings");
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SV:GetListingsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ─── GetListingById ──────────────────────────────────────────────────────────
// args[0] = listingId
unique_ptr<Expr> GetListingByIdFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getListingById requires 1 argument");

    string listingId = extractString(arguments[0]);
    cout << "[SV:GetListingByIdFunc] Getting listing: " << listingId << endl;

    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/listings/" + listingId);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SV:GetListingByIdFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ─── CreateBooking ───────────────────────────────────────────────────────────
// args[0] = userEmail, args[1] = listingId
unique_ptr<Expr> CreateBookingFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("createBooking requires 2 arguments");

    string userEmail = extractString(arguments[0]);
    string listingId = extractString(arguments[1]);
    cout << "[SV:CreateBookingFunc] Creating booking for listing: " << listingId << endl;

    try {
        string token = getToken(userEmail);
        if (token.empty()) {
            throw runtime_error("[SV:CreateBookingFunc] No token for " + userEmail + " — user not registered or login failed");
        }

        json body = {
            {"serviceListingId", listingId},
            {"scheduledAt", "2027-06-15T10:00:00"},
            {"notes", "Test booking"}
        };
        map<string,string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post("/api/bookings", body, headers);

        if (resp.statusCode == 200) {
            json data = resp.getJson();
            string bookingId = data["id"].get<string>();
            factory->getB()[bookingId] = bookingId;
            return make_unique<String>(bookingId);
        }

        throw runtime_error("[SV:CreateBookingFunc] API call failed: " +
            to_string(resp.statusCode) + " " + resp.body);
    } catch (const runtime_error&) {
        throw;
    } catch (const exception& e) {
        throw runtime_error(string("[SV:CreateBookingFunc] Error: ") + e.what());
    }
}

// ─── GetMyBookings ───────────────────────────────────────────────────────────
// args[0] = userEmail
unique_ptr<Expr> GetMyBookingsFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getMyBookings requires 1 argument");

    string userEmail = extractString(arguments[0]);
    cout << "[SV:GetMyBookingsFunc] Getting bookings for: " << userEmail << endl;

    try {
        string token = getToken(userEmail);
        if (token.empty()) return make_unique<Num>(401);

        map<string,string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->get("/api/bookings/my", headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SV:GetMyBookingsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ─── ConfirmBooking ──────────────────────────────────────────────────────────
// args[0] = provEmail, args[1] = bookingId
unique_ptr<Expr> ConfirmBookingFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("confirmBooking requires 2 arguments");

    string provEmail = extractString(arguments[0]);
    string bookingId = extractString(arguments[1]);
    cout << "[SV:ConfirmBookingFunc] Confirming booking: " << bookingId << endl;

    try {
        string token = getToken(provEmail);
        if (token.empty()) return make_unique<Num>(401);

        map<string,string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post(
            "/api/bookings/" + bookingId + "/confirm", json::object(), headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SV:ConfirmBookingFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ─── CompleteBooking ─────────────────────────────────────────────────────────
// args[0] = provEmail, args[1] = bookingId
unique_ptr<Expr> CompleteBookingFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("completeBooking requires 2 arguments");

    string provEmail = extractString(arguments[0]);
    string bookingId = extractString(arguments[1]);
    cout << "[SV:CompleteBookingFunc] Completing booking: " << bookingId << endl;

    try {
        string token = getToken(provEmail);
        if (token.empty()) return make_unique<Num>(401);

        map<string,string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post(
            "/api/bookings/" + bookingId + "/complete", json::object(), headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SV:CompleteBookingFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ─── CancelBooking ───────────────────────────────────────────────────────────
// args[0] = userEmail, args[1] = bookingId
unique_ptr<Expr> CancelBookingFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("cancelBooking requires 2 arguments");

    string userEmail = extractString(arguments[0]);
    string bookingId = extractString(arguments[1]);
    cout << "[SV:CancelBookingFunc] Cancelling booking: " << bookingId << endl;

    try {
        string token = getToken(userEmail);
        if (token.empty()) return make_unique<Num>(401);

        map<string,string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post(
            "/api/bookings/" + bookingId + "/cancel", json::object(), headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SV:CancelBookingFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ─── CreateReview ────────────────────────────────────────────────────────────
// args[0] = userEmail, args[1] = bookingId
unique_ptr<Expr> CreateReviewFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("createReview requires 2 arguments");

    string userEmail = extractString(arguments[0]);
    string bookingId = extractString(arguments[1]);
    cout << "[SV:CreateReviewFunc] Creating review for booking: " << bookingId << endl;

    try {
        string token = getToken(userEmail);
        if (token.empty()) return make_unique<Num>(401);

        json body = {{"rating", 5}, {"comment", "Excellent test service!"}};
        map<string,string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post(
            "/api/bookings/" + bookingId + "/reviews", body, headers);
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SV:CreateReviewFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ─── GetListingReviews ───────────────────────────────────────────────────────
// args[0] = listingId
unique_ptr<Expr> GetListingReviewsFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getListingReviews requires 1 argument");

    string listingId = extractString(arguments[0]);
    cout << "[SV:GetListingReviewsFunc] Getting reviews for listing: " << listingId << endl;

    try {
        HttpResponse resp = factory->getHttpClient()->get("/api/listings/" + listingId + "/reviews");
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SV:GetListingReviewsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ─── CreateListingUnauth ─────────────────────────────────────────────────────
// args[0] = catId, args[1] = listingTitle  — no auth → expect 401/403
unique_ptr<Expr> CreateListingUnauthFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("createListingUnauth requires 2 arguments");

    string catId        = extractString(arguments[0]);
    string listingTitle = extractString(arguments[1]);
    cout << "[SV:CreateListingUnauthFunc] Attempting unauth listing creation..." << endl;

    try {
        json body = {
            {"categoryId", catId},
            {"title", listingTitle},
            {"description", "Unauthorized listing attempt"},
            {"price", 50.0},
            {"location", "Nowhere"},
            {"estimatedDuration", 30}
        };
        // No Authorization header — should get 401/403
        HttpResponse resp = factory->getHttpClient()->post("/api/providers/listings", body);
        cout << "[SV:CreateListingUnauthFunc] Got " << resp.statusCode << " (expected 401/403)" << endl;
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SV:CreateListingUnauthFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ─── CreateBookingAsProvider ─────────────────────────────────────────────────
// args[0] = provEmail, args[1] = listingId  — provider role → expect 403
unique_ptr<Expr> CreateBookingAsProviderFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("createBookingAsProvider requires 2 arguments");

    string provEmail = extractString(arguments[0]);
    string listingId = extractString(arguments[1]);
    cout << "[SV:CreateBookingAsProviderFunc] Provider attempting to book..." << endl;

    try {
        string token = getToken(provEmail);
        if (token.empty()) return make_unique<Num>(401);

        json body = {
            {"serviceListingId", listingId},
            {"scheduledAt", "2027-07-20T14:00:00"}
        };
        map<string,string> headers = {{"Authorization", "Bearer " + token}};
        HttpResponse resp = factory->getHttpClient()->post("/api/bookings", body, headers);
        cout << "[SV:CreateBookingAsProviderFunc] Got " << resp.statusCode << " (expected 403)" << endl;
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[SV:CreateBookingAsProviderFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

/* ============================================================
 * ServeezFunctionFactory
 * ============================================================ */

ServeezFunctionFactory::ServeezFunctionFactory(const string& url)
    : baseUrl(url) {
    httpClient = make_unique<HttpClient>(url);
}

unique_ptr<Function> ServeezFunctionFactory::getFunction(string fname, vector<Expr*> args) {
    if (fname == "reset")                    return make_unique<ResetFunc>(this, args);
    if (fname == "get_U")                    return make_unique<GetUFunc>(this, args);
    if (fname == "set_U")                    return make_unique<SetUFunc>(this, args);
    if (fname == "get_P")                    return make_unique<GetPFunc>(this, args);
    if (fname == "set_P")                    return make_unique<SetPFunc>(this, args);
    if (fname == "get_A")                    return make_unique<GetAFunc>(this, args);
    if (fname == "set_A")                    return make_unique<SetAFunc>(this, args);
    if (fname == "get_C")                    return make_unique<GetCFunc>(this, args);
    if (fname == "set_C")                    return make_unique<SetCFunc>(this, args);
    if (fname == "get_L")                    return make_unique<GetLFunc>(this, args);
    if (fname == "set_L")                    return make_unique<SetLFunc>(this, args);
    if (fname == "get_B")                    return make_unique<GetBFunc>(this, args);
    if (fname == "set_B")                    return make_unique<SetBFunc>(this, args);
    if (fname == "registerUser")             return make_unique<RegisterUserFunc>(this, args);
    if (fname == "registerProvider")         return make_unique<RegisterProviderFunc>(this, args);
    if (fname == "registerAdmin")            return make_unique<RegisterAdminFunc>(this, args);
    if (fname == "createCategory")           return make_unique<CreateCategoryFunc>(this, args);
    if (fname == "createListing")            return make_unique<CreateListingFunc>(this, args);
    if (fname == "getListings")              return make_unique<GetListingsFunc>(this, args);
    if (fname == "getListingById")           return make_unique<GetListingByIdFunc>(this, args);
    if (fname == "createBooking")            return make_unique<CreateBookingFunc>(this, args);
    if (fname == "getMyBookings")            return make_unique<GetMyBookingsFunc>(this, args);
    if (fname == "confirmBooking")           return make_unique<ConfirmBookingFunc>(this, args);
    if (fname == "completeBooking")          return make_unique<CompleteBookingFunc>(this, args);
    if (fname == "cancelBooking")            return make_unique<CancelBookingFunc>(this, args);
    if (fname == "createReview")             return make_unique<CreateReviewFunc>(this, args);
    if (fname == "getListingReviews")        return make_unique<GetListingReviewsFunc>(this, args);
    if (fname == "createListingUnauth")      return make_unique<CreateListingUnauthFunc>(this, args);
    if (fname == "createBookingAsProvider")  return make_unique<CreateBookingAsProviderFunc>(this, args);

    throw runtime_error("ServeezFunctionFactory: unknown function: " + fname);
}

} // namespace Serveez
