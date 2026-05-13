#include "TripVaultSpec.hpp"

#include <vector>
#include <memory>

using namespace std;

std::unique_ptr<Spec> makeTripVaultSpec() {

    /* =====================================================
     * 1. GLOBAL DECLARATIONS
     * ===================================================== */

    vector<unique_ptr<Decl>> globals;

    auto mkString = []() {
        return make_unique<TypeConst>("string");
    };

    globals.push_back(make_unique<Decl>("U",         make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("T",         make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("Trips",     make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("Members",   make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("E",         make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("Proposals", make_unique<MapType>(mkString(), mkString())));

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

    /* ---------- registerUserOk ---------- */
    {
        // PRE: userEmail not_in dom(U)
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("not_in", std::move(notInArgs));

        // CALL: registerUser(userEmail, userPassword)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("userPassword"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("registerUser", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: userEmail in dom(U')
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> uPrimeArgs;
        uPrimeArgs.push_back(make_unique<Var>("U"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(uPrimeArgs)));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("in", std::move(inArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerUserOk"
        ));
    }

    /* ---------- registerUserDuplicateErr ---------- */
    {
        // PRE: userEmail in dom(U)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: registerUser(userEmail, userPassword)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("userPassword"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("registerUser", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerUserDuplicateErr"
        ));
    }

    /* ---------- loginUserOk ---------- */
    {
        // PRE: userEmail in dom(U)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: loginUser(userEmail)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("loginUser", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: userEmail in dom(T')
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> tPrimeArgs;
        tPrimeArgs.push_back(make_unique<Var>("T"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(tPrimeArgs)));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("in", std::move(inArgs2));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "loginUserOk"
        ));
    }

    /* ---------- loginUserNotFoundErr ---------- */
    {
        // PRE: userEmail not_in dom(U)
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("not_in", std::move(notInArgs));

        // CALL: loginUser(userEmail)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("loginUser", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "loginUserNotFoundErr"
        ));
    }

    /* ---------- createTripOk ---------- */
    {
        // PRE: userEmail in dom(T)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("T"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: createTrip(userEmail, tripName, destination)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("tripName"));
        callArgs.push_back(make_unique<Var>("destination"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createTrip", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: tripId in dom(Trips')
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> tripsPrimeArgs;
        tripsPrimeArgs.push_back(make_unique<Var>("Trips"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(tripsPrimeArgs)));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("in", std::move(inArgs2));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createTripOk"
        ));
    }

    /* ---------- createTripUnauthErr ---------- */
    {
        // PRE: userEmail not_in dom(T)
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("T"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("not_in", std::move(notInArgs));

        // CALL: createTrip(userEmail, tripName, destination)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("tripName"));
        callArgs.push_back(make_unique<Var>("destination"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createTrip", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createTripUnauthErr"
        ));
    }

    /* ---------- getUserTripsOk ---------- */
    {
        // PRE: userEmail in dom(T)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("T"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: getUserTrips(userEmail)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getUserTrips", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getUserTripsOk"
        ));
    }

    /* ---------- updateTripAdminOk ---------- */
    {
        // PRE: AND(userEmail in dom(T), tripId in dom(Trips))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Trips"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: updateTrip(userEmail, tripId, tripName)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("tripId"));
        callArgs.push_back(make_unique<Var>("tripName"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("updateTrip", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "updateTripAdminOk"
        ));
    }

    /* ---------- deleteTripOk ---------- */
    {
        // PRE: AND(userEmail in dom(T), tripId in dom(Trips))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Trips"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: deleteTrip(userEmail, tripId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("tripId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("deleteTrip", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: tripId not_in dom(Trips')
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> tripsPrimeArgs;
        tripsPrimeArgs.push_back(make_unique<Var>("Trips"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(tripsPrimeArgs)));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("not_in", std::move(notInArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "deleteTripOk"
        ));
    }

    /* ---------- deleteTripForbiddenErr ---------- */
    {
        // PRE: tripId in dom(Trips) AND userEmail not_in dom(T)
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("Trips"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("T"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("not_in", std::move(notInArgs)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: deleteTrip(userEmail, tripId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("tripId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("deleteTrip", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "deleteTripForbiddenErr"
        ));
    }

    /* ---------- addMemberOk ---------- */
    {
        // PRE: AND(adminEmail in dom(T), tripId in dom(Trips), memberEmail in dom(U))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("adminEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Trips"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        vector<unique_ptr<Expr>> inArgs3;
        inArgs3.push_back(make_unique<Var>("memberEmail"));
        vector<unique_ptr<Expr>> domArgs3;
        domArgs3.push_back(make_unique<Var>("U"));
        inArgs3.push_back(make_unique<FuncCall>("dom", std::move(domArgs3)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs3)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: addMember(adminEmail, tripId, memberEmail)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("adminEmail"));
        callArgs.push_back(make_unique<Var>("tripId"));
        callArgs.push_back(make_unique<Var>("memberEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("addMember", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "addMemberOk"
        ));
    }

    /* ---------- addMemberForbiddenErr ---------- */
    {
        // PRE: tripId in dom(Trips) AND adminEmail not_in dom(T)
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("Trips"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("adminEmail"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("T"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("not_in", std::move(notInArgs)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: addMember(adminEmail, tripId, memberEmail)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("adminEmail"));
        callArgs.push_back(make_unique<Var>("tripId"));
        callArgs.push_back(make_unique<Var>("memberEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("addMember", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "addMemberForbiddenErr"
        ));
    }

    /* ---------- joinByInviteOk ---------- */
    {
        // PRE: AND(userEmail in dom(T), tripId in dom(Trips))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Trips"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: joinByInvite(userEmail, tripId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("tripId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("joinByInvite", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "joinByInviteOk"
        ));
    }

    /* ---------- createExpenseOk ---------- */
    {
        // PRE: AND(userEmail in dom(T), tripId in dom(Trips))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Trips"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: createExpense(userEmail, tripId, expenseTitle, amount, category)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("tripId"));
        callArgs.push_back(make_unique<Var>("expenseTitle"));
        callArgs.push_back(make_unique<Var>("amount"));
        callArgs.push_back(make_unique<Var>("category"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createExpense", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: expenseId in dom(E')
        vector<unique_ptr<Expr>> inArgs3;
        inArgs3.push_back(make_unique<Var>("expenseId"));
        vector<unique_ptr<Expr>> ePrimeArgs;
        ePrimeArgs.push_back(make_unique<Var>("E"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(ePrimeArgs)));
        inArgs3.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("in", std::move(inArgs3));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createExpenseOk"
        ));
    }

    /* ---------- createExpenseForbiddenErr ---------- */
    {
        // PRE: AND(userEmail not_in dom(T), tripId in dom(Trips))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("not_in", std::move(notInArgs)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Trips"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: createExpense(userEmail, tripId, expenseTitle, amount, category)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("tripId"));
        callArgs.push_back(make_unique<Var>("expenseTitle"));
        callArgs.push_back(make_unique<Var>("amount"));
        callArgs.push_back(make_unique<Var>("category"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createExpense", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createExpenseForbiddenErr"
        ));
    }

    /* ---------- getExpensesOk ---------- */
    {
        // PRE: AND(userEmail in dom(T), tripId in dom(Trips))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Trips"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: getExpenses(userEmail, tripId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("tripId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getExpenses", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getExpensesOk"
        ));
    }

    /* ---------- deleteExpenseOk ---------- */
    {
        // PRE: AND(userEmail in dom(T), expenseId in dom(E))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("expenseId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("E"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: deleteExpense(userEmail, expenseId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("expenseId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("deleteExpense", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: expenseId not_in dom(E')
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("expenseId"));
        vector<unique_ptr<Expr>> ePrimeArgs;
        ePrimeArgs.push_back(make_unique<Var>("E"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(ePrimeArgs)));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("not_in", std::move(notInArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "deleteExpenseOk"
        ));
    }

    /* ---------- deleteExpenseForbiddenErr ---------- */
    {
        // PRE: AND(userEmail not_in dom(T), expenseId in dom(E))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("not_in", std::move(notInArgs)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("expenseId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("E"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: deleteExpense(userEmail, expenseId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("expenseId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("deleteExpense", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "deleteExpenseForbiddenErr"
        ));
    }

    /* ---------- createProposalOk ---------- */
    {
        // PRE: AND(userEmail in dom(T), tripId in dom(Trips))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Trips"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: createProposal(userEmail, tripId, proposalTitle, proposalType)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("tripId"));
        callArgs.push_back(make_unique<Var>("proposalTitle"));
        callArgs.push_back(make_unique<Var>("proposalType"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createProposal", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: proposalId in dom(Proposals')
        vector<unique_ptr<Expr>> inArgs3;
        inArgs3.push_back(make_unique<Var>("proposalId"));
        vector<unique_ptr<Expr>> propPrimeArgs;
        propPrimeArgs.push_back(make_unique<Var>("Proposals"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(propPrimeArgs)));
        inArgs3.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("in", std::move(inArgs3));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createProposalOk"
        ));
    }

    /* ---------- getProposalsOk ---------- */
    {
        // PRE: AND(userEmail in dom(T), tripId in dom(Trips))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("tripId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Trips"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: getProposals(userEmail, tripId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("tripId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getProposals", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getProposalsOk"
        ));
    }

    /* ---------- deleteProposalOk ---------- */
    {
        // PRE: AND(userEmail in dom(T), proposalId in dom(Proposals))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("proposalId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Proposals"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: deleteProposal(userEmail, proposalId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("proposalId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("deleteProposal", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: proposalId not_in dom(Proposals')
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("proposalId"));
        vector<unique_ptr<Expr>> propPrimeArgs;
        propPrimeArgs.push_back(make_unique<Var>("Proposals"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(propPrimeArgs)));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("not_in", std::move(notInArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "deleteProposalOk"
        ));
    }

    /* ---------- deleteProposalForbiddenErr ---------- */
    {
        // PRE: AND(userEmail not_in dom(T), proposalId in dom(Proposals))
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("not_in", std::move(notInArgs)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("proposalId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Proposals"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: deleteProposal(userEmail, proposalId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("proposalId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("deleteProposal", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "deleteProposalForbiddenErr"
        ));
    }

    /* ---------- registerUser2Ok ---------- */
    {
        // PRE: user2Email not_in dom(U)
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("user2Email"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("not_in", std::move(notInArgs));

        // CALL: registerUser(user2Email, user2Password)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("user2Email"));
        callArgs.push_back(make_unique<Var>("user2Password"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("registerUser", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: user2Email in dom(U')
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("user2Email"));
        vector<unique_ptr<Expr>> uPrimeArgs;
        uPrimeArgs.push_back(make_unique<Var>("U"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(uPrimeArgs)));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("in", std::move(inArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerUser2Ok"
        ));
    }

    /* ---------- loginUser2Ok ---------- */
    {
        // PRE: user2Email in dom(U)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("user2Email"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: loginUser(user2Email)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("user2Email"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("loginUser", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: user2Email in dom(T')
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("user2Email"));
        vector<unique_ptr<Expr>> tPrimeArgs;
        tPrimeArgs.push_back(make_unique<Var>("T"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(tPrimeArgs)));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("in", std::move(inArgs2));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "loginUser2Ok"
        ));
    }

    /* =====================================================
     * 5. ASSEMBLE SPEC
     * ===================================================== */

    return make_unique<Spec>(
        std::move(globals),
        std::move(init),
        std::move(functions),
        std::move(blocks)
    );
}
