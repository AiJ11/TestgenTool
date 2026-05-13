#ifndef SERVEEZFUNCTIONFACTORY_HH
#define SERVEEZFUNCTIONFACTORY_HH

#include "functionfactory.hh"
#include "../ast.hh"
#include "httpclient.hh"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <map>

using json = nlohmann::json;
using namespace std;

namespace Serveez {

// Forward declaration
class ServeezFunctionFactory;

/* ============================================================
 * Base class for all Serveez API functions
 * ============================================================ */
class ServeezAPIFunction : public Function {
protected:
    ServeezFunctionFactory* factory;
    vector<Expr*> arguments;

public:
    ServeezAPIFunction(ServeezFunctionFactory* factory, vector<Expr*> args);
    virtual ~ServeezAPIFunction() = default;

    string extractString(Expr* expr);
    json extractJson(Expr* expr);
    string getToken(const string& email);  // Look up JWT from T_cache
};

/* ============================================================
 * Test Helper Functions
 * ============================================================ */

class ResetFunc : public ServeezAPIFunction {
public:
    ResetFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

class GetUFunc : public ServeezAPIFunction {
public:
    GetUFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};
class SetUFunc : public ServeezAPIFunction {
public:
    SetUFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

class GetPFunc : public ServeezAPIFunction {
public:
    GetPFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};
class SetPFunc : public ServeezAPIFunction {
public:
    SetPFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

class GetAFunc : public ServeezAPIFunction {
public:
    GetAFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};
class SetAFunc : public ServeezAPIFunction {
public:
    SetAFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

class GetCFunc : public ServeezAPIFunction {
public:
    GetCFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};
class SetCFunc : public ServeezAPIFunction {
public:
    SetCFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

class GetLFunc : public ServeezAPIFunction {
public:
    GetLFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};
class SetLFunc : public ServeezAPIFunction {
public:
    SetLFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

class GetBFunc : public ServeezAPIFunction {
public:
    GetBFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};
class SetBFunc : public ServeezAPIFunction {
public:
    SetBFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

/* ============================================================
 * Business API Functions
 * ============================================================ */

// RegisterUser(email) → email  [creates USER role]
class RegisterUserFunc : public ServeezAPIFunction {
public:
    RegisterUserFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// RegisterProvider(email) → email  [creates PROVIDER role]
class RegisterProviderFunc : public ServeezAPIFunction {
public:
    RegisterProviderFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// RegisterAdmin(email) → email  [creates ADMIN role]
class RegisterAdminFunc : public ServeezAPIFunction {
public:
    RegisterAdminFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// CreateCategory(adminEmail, catName) → catId
class CreateCategoryFunc : public ServeezAPIFunction {
public:
    CreateCategoryFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// CreateListing(provEmail, catId, listingTitle) → listingId
class CreateListingFunc : public ServeezAPIFunction {
public:
    CreateListingFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// GetListings() → ok (public)
class GetListingsFunc : public ServeezAPIFunction {
public:
    GetListingsFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// GetListingById(listingId) → ok (public)
class GetListingByIdFunc : public ServeezAPIFunction {
public:
    GetListingByIdFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// CreateBooking(userEmail, listingId) → bookingId
class CreateBookingFunc : public ServeezAPIFunction {
public:
    CreateBookingFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// GetMyBookings(userEmail) → ok
class GetMyBookingsFunc : public ServeezAPIFunction {
public:
    GetMyBookingsFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// ConfirmBooking(provEmail, bookingId) → ok
class ConfirmBookingFunc : public ServeezAPIFunction {
public:
    ConfirmBookingFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// CompleteBooking(provEmail, bookingId) → ok
class CompleteBookingFunc : public ServeezAPIFunction {
public:
    CompleteBookingFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// CancelBooking(userEmail, bookingId) → ok
class CancelBookingFunc : public ServeezAPIFunction {
public:
    CancelBookingFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// CreateReview(userEmail, bookingId) → ok
class CreateReviewFunc : public ServeezAPIFunction {
public:
    CreateReviewFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// GetListingReviews(listingId) → ok (public)
class GetListingReviewsFunc : public ServeezAPIFunction {
public:
    GetListingReviewsFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// CreateListingUnauth(catId, listingTitle) → expect 401/403
class CreateListingUnauthFunc : public ServeezAPIFunction {
public:
    CreateListingUnauthFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

// CreateBookingAsProvider(provEmail, listingId) → expect 403
class CreateBookingAsProviderFunc : public ServeezAPIFunction {
public:
    CreateBookingAsProviderFunc(ServeezFunctionFactory* f, vector<Expr*> a) : ServeezAPIFunction(f, a) {}
    unique_ptr<Expr> execute() override;
};

/* ============================================================
 * ServeezFunctionFactory
 * ============================================================ */
class ServeezFunctionFactory : public FunctionFactory {
private:
    unique_ptr<HttpClient> httpClient;
    string baseUrl;

    map<string,string> U_cache;  // email → email  (USER role)
    map<string,string> P_cache;  // email → email  (PROVIDER role)
    map<string,string> A_cache;  // email → email  (ADMIN role)
    map<string,string> T_cache;  // email → JWT token  (all users)
    map<string,string> C_cache;  // catId → catId  (categories)
    map<string,string> L_cache;  // listingId → listingId
    map<string,string> B_cache;  // bookingId → bookingId

public:
    ServeezFunctionFactory(const string& baseUrl = "http://localhost:8083");
    ~ServeezFunctionFactory() = default;

    unique_ptr<Function> getFunction(string fname, vector<Expr*> args) override;

    HttpClient* getHttpClient() { return httpClient.get(); }

    map<string,string>& getU() { return U_cache; }
    map<string,string>& getP() { return P_cache; }
    map<string,string>& getA() { return A_cache; }
    map<string,string>& getT() { return T_cache; }
    map<string,string>& getC() { return C_cache; }
    map<string,string>& getL() { return L_cache; }
    map<string,string>& getB() { return B_cache; }
};

} // namespace Serveez

#endif // SERVEEZFUNCTIONFACTORY_HH
