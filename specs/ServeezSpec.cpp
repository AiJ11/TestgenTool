#include "ServeezSpec.hpp"

#include <vector>
#include <memory>

using namespace std;

std::unique_ptr<Spec> makeServeezSpec() {

    /* =====================================================
     * 1. GLOBAL DECLARATIONS
     * ===================================================== */

    vector<unique_ptr<Decl>> globals;

    auto mkString = []() {
        return make_unique<TypeConst>("string");
    };

    // U: email → email  (USER-role users)
    globals.push_back(make_unique<Decl>("U", make_unique<MapType>(mkString(), mkString())));
    // P: email → email  (PROVIDER-role users)
    globals.push_back(make_unique<Decl>("P", make_unique<MapType>(mkString(), mkString())));
    // A: email → email  (ADMIN-role users)
    globals.push_back(make_unique<Decl>("A", make_unique<MapType>(mkString(), mkString())));
    // C: catId → catId  (categories)
    globals.push_back(make_unique<Decl>("C", make_unique<MapType>(mkString(), mkString())));
    // L: listingId → listingId  (listings)
    globals.push_back(make_unique<Decl>("L", make_unique<MapType>(mkString(), mkString())));
    // B: bookingId → bookingId  (bookings)
    globals.push_back(make_unique<Decl>("B", make_unique<MapType>(mkString(), mkString())));

    /* =====================================================
     * 2. INITIALIZATIONS
     * ===================================================== */

    vector<unique_ptr<Init>> init;
    for (const auto& g : globals) {
        init.push_back(
            make_unique<Init>(
                g->name,
                make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{})
            )
        );
    }

    /* =====================================================
     * 3. FUNCTION DECLARATIONS (none)
     * ===================================================== */

    vector<unique_ptr<APIFuncDecl>> functions;

    /* =====================================================
     * 4. API BLOCKS
     * ===================================================== */

    vector<unique_ptr<API>> blocks;

    /* ─── Helper lambdas ─────────────────────────────────────────────────── */

    // Build: varName in dom(MapName)
    auto inDom = [](const string& varName, const string& mapName) -> unique_ptr<Expr> {
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>(varName));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>(mapName));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        return make_unique<FuncCall>("in", std::move(inArgs));
    };

    // Build: varName not_in dom(MapName)
    auto notInDom = [](const string& varName, const string& mapName) -> unique_ptr<Expr> {
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>(varName));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>(mapName));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        return make_unique<FuncCall>("not_in", std::move(notInArgs));
    };

    // Build: varName in dom(MapName')   (post-state)
    auto inDomPrime = [](const string& varName, const string& mapName) -> unique_ptr<Expr> {
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>(varName));
        vector<unique_ptr<Expr>> primeArgs;
        primeArgs.push_back(make_unique<Var>(mapName));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(primeArgs)));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        return make_unique<FuncCall>("in", std::move(inArgs));
    };

    /* ─────────────────────────────────────────────────────────────────────
     * Block 1: registerUserOk
     *   PRE:  userEmail not_in dom(U)
     *   CALL: registerUser(userEmail)   [always creates USER role]
     *   POST: userEmail in dom(U')
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = notInDom("userEmail", "U");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("registerUser", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = inDomPrime("userEmail", "U");

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerUserOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 2: registerProviderOk
     *   PRE:  provEmail not_in dom(P)
     *   CALL: registerProvider(provEmail)   [always creates PROVIDER role]
     *   POST: provEmail in dom(P')
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = notInDom("provEmail", "P");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("provEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("registerProvider", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = inDomPrime("provEmail", "P");

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerProviderOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 3: registerAdminOk
     *   PRE:  adminEmail not_in dom(A)
     *   CALL: registerAdmin(adminEmail)   [always creates ADMIN role]
     *   POST: adminEmail in dom(A')
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = notInDom("adminEmail", "A");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("adminEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("registerAdmin", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = inDomPrime("adminEmail", "A");

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerAdminOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 4: createCategoryOk
     *   PRE:  adminEmail in dom(A)
     *   CALL: createCategory(adminEmail, catName)
     *   POST: catId in dom(C')
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("adminEmail", "A");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("adminEmail"));
        callArgs.push_back(make_unique<Var>("catName"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createCategory", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = inDomPrime("catId", "C");

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createCategoryOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 5: createListingOk
     *   PRE:  catId in dom(C)
     *   CALL: createListing(provEmail, catId, listingTitle)
     *   POST: listingId in dom(L')
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("catId", "C");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("provEmail"));
        callArgs.push_back(make_unique<Var>("catId"));
        callArgs.push_back(make_unique<Var>("listingTitle"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createListing", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = inDomPrime("listingId", "L");

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createListingOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 6: getListingsOk
     *   PRE:  listingId in dom(L)
     *   CALL: getListings()
     *   POST: Num(1)
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("listingId", "L");

        vector<unique_ptr<Expr>> callArgs;
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getListings", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getListingsOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 7: getListingByIdOk
     *   PRE:  listingId in dom(L)
     *   CALL: getListingById(listingId)
     *   POST: Num(1)
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("listingId", "L");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("listingId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getListingById", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getListingByIdOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 8: createBookingOk
     *   PRE:  listingId in dom(L)
     *   CALL: createBooking(userEmail, listingId)
     *   POST: bookingId in dom(B')
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("listingId", "L");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("listingId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createBooking", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = inDomPrime("bookingId", "B");

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createBookingOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 9: getMyBookingsOk
     *   PRE:  bookingId in dom(B)
     *   CALL: getMyBookings(userEmail)
     *   POST: Num(1)
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("bookingId", "B");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getMyBookings", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getMyBookingsOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 10: confirmBookingOk
     *   PRE:  bookingId in dom(B)
     *   CALL: confirmBooking(provEmail, bookingId)
     *   POST: Num(1)
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("bookingId", "B");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("provEmail"));
        callArgs.push_back(make_unique<Var>("bookingId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("confirmBooking", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "confirmBookingOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 11: completeBookingOk
     *   PRE:  bookingId in dom(B)
     *   CALL: completeBooking(provEmail, bookingId)
     *   POST: Num(1)
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("bookingId", "B");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("provEmail"));
        callArgs.push_back(make_unique<Var>("bookingId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("completeBooking", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "completeBookingOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 12: cancelBookingOk
     *   PRE:  bookingId in dom(B)
     *   CALL: cancelBooking(userEmail, bookingId)
     *   POST: Num(1)
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("bookingId", "B");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("bookingId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("cancelBooking", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "cancelBookingOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 13: createReviewOk
     *   PRE:  bookingId in dom(B)
     *   CALL: createReview(userEmail, bookingId)
     *   POST: Num(1)
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("bookingId", "B");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("bookingId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createReview", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "svzCreateReviewOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 14: getListingReviewsOk
     *   PRE:  listingId in dom(L)
     *   CALL: getListingReviews(listingId)
     *   POST: Num(1)
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("listingId", "L");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("listingId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getListingReviews", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getListingReviewsOk"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 15: createListingUnauthErr
     *   PRE:  catId in dom(C)
     *   CALL: createListingUnauth(catId, listingTitle)
     *   POST: Num(1)   (factory throws if 200 — verifies 401/403 enforcement)
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("catId", "C");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("catId"));
        callArgs.push_back(make_unique<Var>("listingTitle"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createListingUnauth", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createListingUnauthErr"
        ));
    }

    /* ─────────────────────────────────────────────────────────────────────
     * Block 16: createBookingAsProviderErr
     *   PRE:  listingId in dom(L)
     *   CALL: createBookingAsProvider(provEmail, listingId)
     *   POST: Num(1)   (factory throws if 200 — verifies role enforcement)
     * ──────────────────────────────────────────────────────────────────── */
    {
        auto pre = inDom("listingId", "L");

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("provEmail"));
        callArgs.push_back(make_unique<Var>("listingId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createBookingAsProvider", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createBookingAsProviderErr"
        ));
    }

    /* =====================================================
     * 5. BUILD SPEC
     * ===================================================== */

    return make_unique<Spec>(
        std::move(globals),
        std::move(init),
        std::move(functions),
        std::move(blocks)
    );
}
