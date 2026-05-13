#include "GhostSocketSpec.hpp"

#include <vector>
#include <memory>

using namespace std;

std::unique_ptr<Spec> makeGhostSocketSpec() {

    /* =====================================================
     * 1. GLOBAL DECLARATIONS
     * ===================================================== */

    vector<unique_ptr<Decl>> globals;

    auto mkString = []() {
        return make_unique<TypeConst>("string");
    };

    globals.push_back(make_unique<Decl>("U", make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("D", make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("S", make_unique<MapType>(mkString(), mkString())));

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

        // CALL: registerUser(userEmail)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
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

    /* ---------- registerUser2Ok ---------- */
    {
        // PRE: user2Email not_in dom(U)
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("user2Email"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("not_in", std::move(notInArgs));

        // CALL: registerUser(user2Email)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("user2Email"));
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

    /* ---------- registerDeviceOk ---------- */
    {
        // PRE: userEmail in dom(U)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: registerDevice(userEmail, devId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("devId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("registerDevice", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: devId in dom(D')
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("devId"));
        vector<unique_ptr<Expr>> dPrimeArgs;
        dPrimeArgs.push_back(make_unique<Var>("D"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(dPrimeArgs)));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("in", std::move(inArgs2));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerDeviceOk"
        ));
    }

    /* ---------- getMyDevicesOk ---------- */
    {
        // PRE: userEmail in dom(U)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: getMyDevices(userEmail)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getMyDevices", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getMyDevicesOk"
        ));
    }

    /* ---------- getOtherDevicesOk ---------- */
    {
        // PRE: user2Email in dom(U)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("user2Email"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: getOtherDevices(user2Email)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("user2Email"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getOtherDevices", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getOtherDevicesOk"
        ));
    }

    /* ---------- getDeviceInfoOk ---------- */
    {
        // PRE: devId in dom(D)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("devId"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("D"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: getDeviceInfo(userEmail, devId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("devId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getDeviceInfo", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getDeviceInfoOk"
        ));
    }

    /* ---------- getDeviceInfoForbiddenErr ---------- */
    {
        // PRE: devId not_in dom(D)
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("devId"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("D"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("not_in", std::move(notInArgs));

        // CALL: getDeviceInfo(userEmail, devId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("devId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getDeviceInfo", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getDeviceInfoForbiddenErr"
        ));
    }

    /* ---------- deleteDeviceOk ---------- */
    {
        // PRE: devId in dom(D)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("devId"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("D"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: deleteDevice(userEmail, devId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("devId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("deleteDevice", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "deleteDeviceOk"
        ));
    }

    /* ---------- createSessionOk ---------- */
    {
        // PRE: devId in dom(D)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("devId"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("D"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: createSession(userEmail, devId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("devId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createSession", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: sessId in dom(S')
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("sessId"));
        vector<unique_ptr<Expr>> sPrimeArgs;
        sPrimeArgs.push_back(make_unique<Var>("S"));
        vector<unique_ptr<Expr>> domPrimeArgs;
        domPrimeArgs.push_back(make_unique<FuncCall>("'", std::move(sPrimeArgs)));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domPrimeArgs)));
        auto post = make_unique<FuncCall>("in", std::move(inArgs2));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createSessionOk"
        ));
    }

    /* ---------- createSessionForbiddenErr ---------- */
    {
        // PRE: devId not_in dom(D)
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("devId"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("D"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("not_in", std::move(notInArgs));

        // CALL: createSession(userEmail, devId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("devId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createSession", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createSessionForbiddenErr"
        ));
    }

    /* ---------- joinSessionOk ---------- */
    {
        // PRE: sessId in dom(S)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("sessId"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("S"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: joinSession(user2Email, sessId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("user2Email"));
        callArgs.push_back(make_unique<Var>("sessId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("joinSession", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "joinSessionOk"
        ));
    }

    /* ---------- joinSessionNotFoundErr ---------- */
    {
        // PRE: sessId not_in dom(S)
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("sessId"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("S"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("not_in", std::move(notInArgs));

        // CALL: joinSession(userEmail, sessId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("sessId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("joinSession", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "joinSessionNotFoundErr"
        ));
    }

    /* ---------- getSessionsOk ---------- */
    {
        // PRE: userEmail in dom(U)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("userEmail"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: getSessions(userEmail)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getSessions", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getSessionsOk"
        ));
    }

    /* ---------- terminateSessionOk ---------- */
    {
        // PRE: sessId in dom(S)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("sessId"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("S"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: terminateSession(userEmail, sessId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("sessId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("terminateSession", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "terminateSessionOk"
        ));
    }

    /* ---------- terminateSessionForbiddenErr ---------- */
    {
        // PRE: sessId not_in dom(S)
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("sessId"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("S"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("not_in", std::move(notInArgs));

        // CALL: terminateSession(userEmail, sessId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("sessId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("terminateSession", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "terminateSessionForbiddenErr"
        ));
    }

    /* ---------- updatePermissionsOk ---------- */
    {
        // PRE: sessId in dom(S)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("sessId"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("S"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: updatePermissions(userEmail, sessId)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("userEmail"));
        callArgs.push_back(make_unique<Var>("sessId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("updatePermissions", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: Num(1)
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "updatePermissionsOk"
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
