#include "RestaurantSpec.hpp"

#include <vector>
#include <memory>

using namespace std;

std::unique_ptr<Spec> makeRestaurantSpec() {

    /* =====================================================
     * 1. GLOBAL DECLARATIONS
     * ===================================================== */

    vector<unique_ptr<Decl>> globals;

    auto mkString = []() {
        return make_unique<TypeConst>("string");
    };

    auto mkRole = []() {
        return make_unique<TypeConst>("Role");
    };

    // User authentication & authorization
    globals.push_back(make_unique<Decl>("U", make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("T", make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("Roles", make_unique<MapType>(mkString(), mkRole())));

    // Customer data
    globals.push_back(make_unique<Decl>("C", make_unique<MapType>(mkString(), mkString())));

    // Restaurant data
    globals.push_back(make_unique<Decl>("R", make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("M", make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("Owners", make_unique<MapType>(mkString(), mkString())));

    // Order data
    globals.push_back(make_unique<Decl>("O", make_unique<MapType>(mkString(), mkString())));
    globals.push_back(make_unique<Decl>("Assignments", make_unique<MapType>(mkString(), mkString())));

    // Review data
    globals.push_back(make_unique<Decl>("Rev", make_unique<MapType>(mkString(), mkString())));

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

    /* ========================================================================
     * CUSTOMER APIs
     * ======================================================================== */

    /* ---------- registerCustomerOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        // email not_in dom(U)
        preArgs.push_back(make_unique<Var>("email"));
        
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        preArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        
        auto pre = make_unique<FuncCall>("not_in", std::move(preArgs));

        // CALL: registerCustomer(email, password, fullName, mobile)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("email"));
        callArgs.push_back(make_unique<Var>("password"));
        callArgs.push_back(make_unique<Var>("fullName"));
        callArgs.push_back(make_unique<Var>("mobile"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("registerCustomer", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: U'[email] = password AND Roles'[email] = CUSTOMER
        vector<unique_ptr<Expr>> postArgs;
        
        // U'[email] = password
        vector<unique_ptr<Expr>> eq1Args;
        vector<unique_ptr<Expr>> uprimeArgs;
        uprimeArgs.push_back(make_unique<Var>("U"));
        vector<unique_ptr<Expr>> indexArgs1;
        indexArgs1.push_back(make_unique<FuncCall>("'", std::move(uprimeArgs)));
        indexArgs1.push_back(make_unique<Var>("email"));
        eq1Args.push_back(make_unique<FuncCall>("[]", std::move(indexArgs1)));
        eq1Args.push_back(make_unique<Var>("password"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq1Args)));
        
        // Roles'[email] = CUSTOMER
        vector<unique_ptr<Expr>> eq2Args;
        vector<unique_ptr<Expr>> rolesPrimeArgs;
        rolesPrimeArgs.push_back(make_unique<Var>("Roles"));
        vector<unique_ptr<Expr>> indexArgs2;
        indexArgs2.push_back(make_unique<FuncCall>("'", std::move(rolesPrimeArgs)));
        indexArgs2.push_back(make_unique<Var>("email"));
        eq2Args.push_back(make_unique<FuncCall>("[]", std::move(indexArgs2)));
        eq2Args.push_back(make_unique<Var>("CUSTOMER"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq2Args)));

        auto post = make_unique<FuncCall>("AND", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerCustomerOk"
        ));
    }

    /* ---------- registerOwnerOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        preArgs.push_back(make_unique<Var>("email"));
        
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        preArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        
        auto pre = make_unique<FuncCall>("not_in", std::move(preArgs));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("email"));
        callArgs.push_back(make_unique<Var>("password"));
        callArgs.push_back(make_unique<Var>("fullName"));
        callArgs.push_back(make_unique<Var>("mobile"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("registerOwner", std::move(callArgs)),
            Response(nullptr)
        );

        vector<unique_ptr<Expr>> postArgs;
        
        vector<unique_ptr<Expr>> eq1Args;
        vector<unique_ptr<Expr>> uprimeArgs;
        uprimeArgs.push_back(make_unique<Var>("U"));
        vector<unique_ptr<Expr>> indexArgs1;
        indexArgs1.push_back(make_unique<FuncCall>("'", std::move(uprimeArgs)));
        indexArgs1.push_back(make_unique<Var>("email"));
        eq1Args.push_back(make_unique<FuncCall>("[]", std::move(indexArgs1)));
        eq1Args.push_back(make_unique<Var>("password"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq1Args)));
        
        vector<unique_ptr<Expr>> eq2Args;
        vector<unique_ptr<Expr>> rolesPrimeArgs;
        rolesPrimeArgs.push_back(make_unique<Var>("Roles"));
        vector<unique_ptr<Expr>> indexArgs2;
        indexArgs2.push_back(make_unique<FuncCall>("'", std::move(rolesPrimeArgs)));
        indexArgs2.push_back(make_unique<Var>("email"));
        eq2Args.push_back(make_unique<FuncCall>("[]", std::move(indexArgs2)));
        eq2Args.push_back(make_unique<Var>("OWNER"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq2Args)));

        auto post = make_unique<FuncCall>("AND", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerOwnerOk"
        ));
    }

    /* ---------- registerAgentOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        preArgs.push_back(make_unique<Var>("email"));
        
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        preArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        
        auto pre = make_unique<FuncCall>("not_in", std::move(preArgs));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("email"));
        callArgs.push_back(make_unique<Var>("password"));
        callArgs.push_back(make_unique<Var>("fullName"));
        callArgs.push_back(make_unique<Var>("mobile"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("registerAgent", std::move(callArgs)),
            Response(nullptr)
        );

        vector<unique_ptr<Expr>> postArgs;
        
        vector<unique_ptr<Expr>> eq1Args;
        vector<unique_ptr<Expr>> uprimeArgs;
        uprimeArgs.push_back(make_unique<Var>("U"));
        vector<unique_ptr<Expr>> indexArgs1;
        indexArgs1.push_back(make_unique<FuncCall>("'", std::move(uprimeArgs)));
        indexArgs1.push_back(make_unique<Var>("email"));
        eq1Args.push_back(make_unique<FuncCall>("[]", std::move(indexArgs1)));
        eq1Args.push_back(make_unique<Var>("password"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq1Args)));
        
        vector<unique_ptr<Expr>> eq2Args;
        vector<unique_ptr<Expr>> rolesPrimeArgs;
        rolesPrimeArgs.push_back(make_unique<Var>("Roles"));
        vector<unique_ptr<Expr>> indexArgs2;
        indexArgs2.push_back(make_unique<FuncCall>("'", std::move(rolesPrimeArgs)));
        indexArgs2.push_back(make_unique<Var>("email"));
        eq2Args.push_back(make_unique<FuncCall>("[]", std::move(indexArgs2)));
        eq2Args.push_back(make_unique<Var>("AGENT"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq2Args)));

        auto post = make_unique<FuncCall>("AND", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerAgentOk"
        ));
    }

    /* ---------- loginOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs)));
        
        vector<unique_ptr<Expr>> eqArgs;
        vector<unique_ptr<Expr>> indexArgs;
        indexArgs.push_back(make_unique<Var>("U"));
        indexArgs.push_back(make_unique<Var>("email"));
        eqArgs.push_back(make_unique<FuncCall>("[]", std::move(indexArgs)));
        eqArgs.push_back(make_unique<Var>("password"));
        preArgs.push_back(make_unique<FuncCall>("=", std::move(eqArgs)));
        
        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("email"));
        callArgs.push_back(make_unique<Var>("password"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("login", std::move(callArgs)),
            Response(nullptr)
        );

        vector<unique_ptr<Expr>> postArgs;
        vector<unique_ptr<Expr>> tprimeArgs;
        tprimeArgs.push_back(make_unique<Var>("T"));
        vector<unique_ptr<Expr>> indexArgs2;
        indexArgs2.push_back(make_unique<FuncCall>("'", std::move(tprimeArgs)));
        indexArgs2.push_back(make_unique<Var>("email"));
        postArgs.push_back(make_unique<FuncCall>("[]", std::move(indexArgs2)));
        postArgs.push_back(make_unique<Var>("token"));
        auto post = make_unique<FuncCall>("=", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "loginOk"
        ));
    }

    /* ---------- browseRestaurantsOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        preArgs.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("T"));
        preArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(preArgs));

        vector<unique_ptr<Expr>> callArgs;
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("browseRestaurants", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "browseRestaurantsOk"
        ));
    }

    /* ---------- viewMenuOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        
        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));
        
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("restaurantId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("R"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));
        
        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("restaurantId"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("viewMenu", std::move(callArgs)),
            Response(nullptr)
        );

        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "viewMenuOk"
        ));
    }

    /* ---------- loginErr ---------- */
{
    // PRE: email NOT in dom(U) (user doesn't exist)
    vector<unique_ptr<Expr>> preArgs;
    preArgs.push_back(make_unique<Var>("email"));
    vector<unique_ptr<Expr>> domArgs;
    domArgs.push_back(make_unique<Var>("U"));
    preArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
    auto pre = make_unique<FuncCall>("not_in", std::move(preArgs));

    vector<unique_ptr<Expr>> callArgs;
    callArgs.push_back(make_unique<Var>("email"));
    callArgs.push_back(make_unique<Var>("password"));
    auto call = make_unique<APIcall>(
        make_unique<FuncCall>("login", std::move(callArgs)),
        Response(nullptr)
    );

    // POST: T unchanged (no token added)
    auto post = make_unique<Num>(1);  // true (no state change)

    blocks.push_back(make_unique<API>(
        std::move(pre), std::move(call), Response(std::move(post)), "loginErr"
    ));
}

/* ---------- registerUserErr ---------- */
{
    // PRE: email IN dom(U) (user already exists)
    vector<unique_ptr<Expr>> preArgs;
    preArgs.push_back(make_unique<Var>("email"));
    vector<unique_ptr<Expr>> domArgs;
    domArgs.push_back(make_unique<Var>("U"));
    preArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
    auto pre = make_unique<FuncCall>("in", std::move(preArgs));

    vector<unique_ptr<Expr>> callArgs;
    callArgs.push_back(make_unique<Var>("email"));
    callArgs.push_back(make_unique<Var>("password"));
    callArgs.push_back(make_unique<Var>("fullName"));
    callArgs.push_back(make_unique<Var>("mobile"));
    auto call = make_unique<APIcall>(
        make_unique<FuncCall>("registerCustomer", std::move(callArgs)),
        Response(nullptr)
    );

    auto post = make_unique<Num>(1);  // true (no state change)

    blocks.push_back(make_unique<API>(
        std::move(pre), std::move(call), Response(std::move(post)), "registerCustomerErr"
    ));
}

/* ---------- placeOrderErr ---------- */
{
    // PRE: email in dom(T) AND email NOT in dom(C) (cart is empty)
    vector<unique_ptr<Expr>> preArgs;
    
    vector<unique_ptr<Expr>> inArgs1;
    inArgs1.push_back(make_unique<Var>("email"));
    vector<unique_ptr<Expr>> domArgs1;
    domArgs1.push_back(make_unique<Var>("T"));
    inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
    preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));
    
    vector<unique_ptr<Expr>> notInArgs;
    notInArgs.push_back(make_unique<Var>("email"));
    vector<unique_ptr<Expr>> domArgs2;
    domArgs2.push_back(make_unique<Var>("C"));
    notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
    preArgs.push_back(make_unique<FuncCall>("not_in", std::move(notInArgs)));
    
    auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

    vector<unique_ptr<Expr>> callArgs;
    callArgs.push_back(make_unique<Var>("deliveryAddress"));
    callArgs.push_back(make_unique<Var>("paymentMethod"));
    auto call = make_unique<APIcall>(
        make_unique<FuncCall>("placeOrder", std::move(callArgs)),
        Response(nullptr)
    );

    auto post = make_unique<Num>(1);  // true (no order created)

    blocks.push_back(make_unique<API>(
        std::move(pre), std::move(call), Response(std::move(post)), "placeOrderErr"
    ));
}

/* ---------- leaveReviewErr ---------- */
{
    // PRE: email NOT in dom(T) (not authenticated)
    vector<unique_ptr<Expr>> preArgs;
    preArgs.push_back(make_unique<Var>("email"));
    vector<unique_ptr<Expr>> domArgs;
    domArgs.push_back(make_unique<Var>("T"));
    preArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
    auto pre = make_unique<FuncCall>("not_in", std::move(preArgs));

    vector<unique_ptr<Expr>> callArgs;
    callArgs.push_back(make_unique<Var>("orderId"));
    callArgs.push_back(make_unique<Var>("restaurantRating"));
    callArgs.push_back(make_unique<Var>("deliveryRating"));
    callArgs.push_back(make_unique<Var>("comment"));
    auto call = make_unique<APIcall>(
        make_unique<FuncCall>("leaveReview", std::move(callArgs)),
        Response(nullptr)
    );

    auto post = make_unique<Num>(1);

    blocks.push_back(make_unique<API>(
        std::move(pre), std::move(call), Response(std::move(post)), "leaveReviewErr"
    ));
}

/* ---------- createRestaurantErr ---------- */
{
    // PRE: email in dom(T) AND Roles[email] != OWNER (wrong role)
    vector<unique_ptr<Expr>> preArgs;
    
    vector<unique_ptr<Expr>> inArgs;
    inArgs.push_back(make_unique<Var>("email"));
    vector<unique_ptr<Expr>> domArgs;
    domArgs.push_back(make_unique<Var>("T"));
    inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
    preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs)));
    
    vector<unique_ptr<Expr>> neqArgs;
    vector<unique_ptr<Expr>> indexArgs;
    indexArgs.push_back(make_unique<Var>("Roles"));
    indexArgs.push_back(make_unique<Var>("email"));
    neqArgs.push_back(make_unique<FuncCall>("[]", std::move(indexArgs)));
    neqArgs.push_back(make_unique<Var>("OWNER"));
    preArgs.push_back(make_unique<FuncCall>("!=", std::move(neqArgs)));
    
    auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

    vector<unique_ptr<Expr>> callArgs;
    callArgs.push_back(make_unique<Var>("name"));
    callArgs.push_back(make_unique<Var>("address"));
    callArgs.push_back(make_unique<Var>("contact"));
    auto call = make_unique<APIcall>(
        make_unique<FuncCall>("createRestaurant", std::move(callArgs)),
        Response(nullptr)
    );

    auto post = make_unique<Num>(1);

    blocks.push_back(make_unique<API>(
        std::move(pre), std::move(call), Response(std::move(post)), "createRestaurantErr"
    ));
}

    /* ---------- addToCartOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        
        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));
        
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("menuItemId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("M"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));
        
        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("menuItemId"));
        callArgs.push_back(make_unique<Var>("quantity"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("addToCart", std::move(callArgs)),
            Response(nullptr)
        );

        vector<unique_ptr<Expr>> postArgs;
        postArgs.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> cprimeArgs;
        cprimeArgs.push_back(make_unique<Var>("C"));
        vector<unique_ptr<Expr>> domArgs3;
        domArgs3.push_back(make_unique<FuncCall>("'", std::move(cprimeArgs)));
        postArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs3)));
        auto post = make_unique<FuncCall>("in", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "addToCartOk"
        ));
    }

    /* ---------- placeOrderOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        
        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));
        
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("C"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));
        
        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("deliveryAddress"));
        callArgs.push_back(make_unique<Var>("paymentMethod"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("placeOrder", std::move(callArgs)),
            Response(nullptr)
        );

        vector<unique_ptr<Expr>> postArgs;
        
        vector<unique_ptr<Expr>> inArgs3;
        inArgs3.push_back(make_unique<Var>("orderId"));
        vector<unique_ptr<Expr>> oprimeArgs;
        oprimeArgs.push_back(make_unique<Var>("O"));
        vector<unique_ptr<Expr>> domArgs3;
        domArgs3.push_back(make_unique<FuncCall>("'", std::move(oprimeArgs)));
        inArgs3.push_back(make_unique<FuncCall>("dom", std::move(domArgs3)));
        postArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs3)));
        
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> cprimeArgs;
        cprimeArgs.push_back(make_unique<Var>("C"));
        vector<unique_ptr<Expr>> domArgs4;
        domArgs4.push_back(make_unique<FuncCall>("'", std::move(cprimeArgs)));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs4)));
        postArgs.push_back(make_unique<FuncCall>("not_in", std::move(notInArgs)));
        
        auto post = make_unique<FuncCall>("AND", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "placeOrderOk"
        ));
    }

    /* ---------- leaveReviewOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        
        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));
        
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("orderId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("O"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));
        
        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("orderId"));
        callArgs.push_back(make_unique<Var>("restaurantRating"));
        callArgs.push_back(make_unique<Var>("deliveryRating"));
        callArgs.push_back(make_unique<Var>("comment"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("leaveReview", std::move(callArgs)),
            Response(nullptr)
        );

        vector<unique_ptr<Expr>> postArgs;
        postArgs.push_back(make_unique<Var>("reviewId"));
        vector<unique_ptr<Expr>> revprimeArgs;
        revprimeArgs.push_back(make_unique<Var>("Rev"));
        vector<unique_ptr<Expr>> domArgs3;
        domArgs3.push_back(make_unique<FuncCall>("'", std::move(revprimeArgs)));
        postArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs3)));
        auto post = make_unique<FuncCall>("in", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "leaveReviewOk"
        ));
    }

    /* ========================================================================
     * OWNER APIs
     * ======================================================================== */

    /* ---------- createRestaurantOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs)));
        
        vector<unique_ptr<Expr>> eqArgs;
        vector<unique_ptr<Expr>> indexArgs;
        indexArgs.push_back(make_unique<Var>("Roles"));
        indexArgs.push_back(make_unique<Var>("email"));
        eqArgs.push_back(make_unique<FuncCall>("[]", std::move(indexArgs)));
        eqArgs.push_back(make_unique<Var>("OWNER"));
        preArgs.push_back(make_unique<FuncCall>("=", std::move(eqArgs)));
        
        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("name"));
        callArgs.push_back(make_unique<Var>("address"));
        callArgs.push_back(make_unique<Var>("contact"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createRestaurant", std::move(callArgs)),
            Response(nullptr)
        );

        vector<unique_ptr<Expr>> postArgs;
        
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("restaurantId"));
        vector<unique_ptr<Expr>> rprimeArgs;
        rprimeArgs.push_back(make_unique<Var>("R"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<FuncCall>("'", std::move(rprimeArgs)));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        postArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));
        
        vector<unique_ptr<Expr>> eq2Args;
        vector<unique_ptr<Expr>> ownersprimeArgs;
        ownersprimeArgs.push_back(make_unique<Var>("Owners"));
        vector<unique_ptr<Expr>> indexArgs2;
        indexArgs2.push_back(make_unique<FuncCall>("'", std::move(ownersprimeArgs)));
        indexArgs2.push_back(make_unique<Var>("restaurantId"));
        eq2Args.push_back(make_unique<FuncCall>("[]", std::move(indexArgs2)));
        eq2Args.push_back(make_unique<Var>("email"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq2Args)));
        
        auto post = make_unique<FuncCall>("AND", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createRestaurantOk"
        ));
    }

    /* ---------- addMenuItemOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        
        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));
        
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("restaurantId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("R"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));
        
        vector<unique_ptr<Expr>> eqArgs;
        vector<unique_ptr<Expr>> indexArgs;
        indexArgs.push_back(make_unique<Var>("Owners"));
        indexArgs.push_back(make_unique<Var>("restaurantId"));
        eqArgs.push_back(make_unique<FuncCall>("[]", std::move(indexArgs)));
        eqArgs.push_back(make_unique<Var>("email"));
        preArgs.push_back(make_unique<FuncCall>("=", std::move(eqArgs)));
        
        vector<unique_ptr<Expr>> and1Args;
        and1Args.push_back(std::move(preArgs[0]));
        and1Args.push_back(std::move(preArgs[1]));
        auto and1 = make_unique<FuncCall>("AND", std::move(and1Args));
        
        vector<unique_ptr<Expr>> preArgsVec;
        preArgsVec.push_back(std::move(and1));
        preArgsVec.push_back(std::move(preArgs[2]));
        auto pre = make_unique<FuncCall>("AND", std::move(preArgsVec));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("restaurantId"));
        callArgs.push_back(make_unique<Var>("name"));
        callArgs.push_back(make_unique<Var>("price"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("addMenuItem", std::move(callArgs)),
            Response(nullptr)
        );

        vector<unique_ptr<Expr>> postArgs;
        postArgs.push_back(make_unique<Var>("menuItemId"));
        vector<unique_ptr<Expr>> mprimeArgs;
        mprimeArgs.push_back(make_unique<Var>("M"));
        vector<unique_ptr<Expr>> domArgs3;
        domArgs3.push_back(make_unique<FuncCall>("'", std::move(mprimeArgs)));
        postArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs3)));
        auto post = make_unique<FuncCall>("in", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "addMenuItemOk"
        ));
    }

    /* ---------- assignOrderOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        
        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));
        
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("orderId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("O"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));
        
        vector<unique_ptr<Expr>> inArgs3;
        inArgs3.push_back(make_unique<Var>("agentEmail"));
        vector<unique_ptr<Expr>> domArgs3;
        domArgs3.push_back(make_unique<Var>("Roles"));
        inArgs3.push_back(make_unique<FuncCall>("dom", std::move(domArgs3)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs3)));
        
        vector<unique_ptr<Expr>> eqArgs;
        vector<unique_ptr<Expr>> indexArgs;
        indexArgs.push_back(make_unique<Var>("Roles"));
        indexArgs.push_back(make_unique<Var>("agentEmail"));
        eqArgs.push_back(make_unique<FuncCall>("[]", std::move(indexArgs)));
        eqArgs.push_back(make_unique<Var>("AGENT"));
        preArgs.push_back(make_unique<FuncCall>("=", std::move(eqArgs)));
        
        vector<unique_ptr<Expr>> and1Args;
        and1Args.push_back(std::move(preArgs[0]));
        and1Args.push_back(std::move(preArgs[1]));
        auto and1 = make_unique<FuncCall>("AND", std::move(and1Args));
        
        vector<unique_ptr<Expr>> and2Args;
        and2Args.push_back(std::move(and1));
        and2Args.push_back(std::move(preArgs[2]));
        auto and2 = make_unique<FuncCall>("AND", std::move(and2Args));
        
        vector<unique_ptr<Expr>> preArgsVec;
        preArgsVec.push_back(std::move(and2));
        preArgsVec.push_back(std::move(preArgs[3]));
        auto pre = make_unique<FuncCall>("AND", std::move(preArgsVec));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("orderId"));
        callArgs.push_back(make_unique<Var>("agentEmail"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("assignOrder", std::move(callArgs)),
            Response(nullptr)
        );

        vector<unique_ptr<Expr>> postArgs;
        vector<unique_ptr<Expr>> assignmentsprimeArgs;
        assignmentsprimeArgs.push_back(make_unique<Var>("Assignments"));
        vector<unique_ptr<Expr>> indexArgs2;
        indexArgs2.push_back(make_unique<FuncCall>("'", std::move(assignmentsprimeArgs)));
        indexArgs2.push_back(make_unique<Var>("orderId"));
        postArgs.push_back(make_unique<FuncCall>("[]", std::move(indexArgs2)));
        postArgs.push_back(make_unique<Var>("agentEmail"));
        auto post = make_unique<FuncCall>("=", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "assignOrderOk"
        ));
    }

    /* ---------- updateOrderStatusOwnerOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        
        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));
        
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("orderId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("O"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));
        
        vector<unique_ptr<Expr>> eqArgs;
        vector<unique_ptr<Expr>> indexArgs;
        indexArgs.push_back(make_unique<Var>("Roles"));
        indexArgs.push_back(make_unique<Var>("email"));
        eqArgs.push_back(make_unique<FuncCall>("[]", std::move(indexArgs)));
        eqArgs.push_back(make_unique<Var>("OWNER"));
        preArgs.push_back(make_unique<FuncCall>("=", std::move(eqArgs)));
        
       vector<unique_ptr<Expr>> and1Args;
        and1Args.push_back(std::move(preArgs[0]));
        and1Args.push_back(std::move(preArgs[1]));
        auto and1 = make_unique<FuncCall>("AND", std::move(and1Args));
        
        vector<unique_ptr<Expr>> preArgsVec;
        preArgsVec.push_back(std::move(and1));
        preArgsVec.push_back(std::move(preArgs[2]));
        auto pre = make_unique<FuncCall>("AND", std::move(preArgsVec));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("orderId"));
        callArgs.push_back(make_unique<Var>("status"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("updateOrderStatusOwner", std::move(callArgs)),
            Response(nullptr)
        );

        vector<unique_ptr<Expr>> postArgs;
        postArgs.push_back(make_unique<Var>("orderId"));
        vector<unique_ptr<Expr>> oprimeArgs;
        oprimeArgs.push_back(make_unique<Var>("O"));
        vector<unique_ptr<Expr>> domArgs3;
        domArgs3.push_back(make_unique<FuncCall>("'", std::move(oprimeArgs)));
        postArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs3)));
        auto post = make_unique<FuncCall>("in", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "updateOrderStatusOwnerOk"
        ));
    }

    /* ========================================================================
     * AGENT APIs
     * ======================================================================== */

    /* ---------- updateOrderStatusAgentOk ---------- */
    {
        vector<unique_ptr<Expr>> preArgs;
        
        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));
        
        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("orderId"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("O"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));
        
        vector<unique_ptr<Expr>> eq1Args;
        vector<unique_ptr<Expr>> indexArgs1;
        indexArgs1.push_back(make_unique<Var>("Roles"));
        indexArgs1.push_back(make_unique<Var>("email"));
        eq1Args.push_back(make_unique<FuncCall>("[]", std::move(indexArgs1)));
        eq1Args.push_back(make_unique<Var>("AGENT"));
        preArgs.push_back(make_unique<FuncCall>("=", std::move(eq1Args)));
        
        vector<unique_ptr<Expr>> eq2Args;
        vector<unique_ptr<Expr>> indexArgs2;
        indexArgs2.push_back(make_unique<Var>("Assignments"));
        indexArgs2.push_back(make_unique<Var>("orderId"));
        eq2Args.push_back(make_unique<FuncCall>("[]", std::move(indexArgs2)));
        eq2Args.push_back(make_unique<Var>("email"));
        preArgs.push_back(make_unique<FuncCall>("=", std::move(eq2Args)));
        
        vector<unique_ptr<Expr>> and1Args;
        and1Args.push_back(std::move(preArgs[0]));
        and1Args.push_back(std::move(preArgs[1]));
        auto and1 = make_unique<FuncCall>("AND", std::move(and1Args));
        
        vector<unique_ptr<Expr>> and2Args;
        and2Args.push_back(std::move(and1));
        and2Args.push_back(std::move(preArgs[2]));
        auto and2 = make_unique<FuncCall>("AND", std::move(and2Args));
        
        vector<unique_ptr<Expr>> preArgsVec;
        preArgsVec.push_back(std::move(and2));
        preArgsVec.push_back(std::move(preArgs[3]));
        auto pre = make_unique<FuncCall>("AND", std::move(preArgsVec));

        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("orderId"));
        callArgs.push_back(make_unique<Var>("status"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("updateOrderStatusAgent", std::move(callArgs)),
            Response(nullptr)
        );

        vector<unique_ptr<Expr>> postArgs;
        postArgs.push_back(make_unique<Var>("orderId"));
        vector<unique_ptr<Expr>> oprimeArgs;
        oprimeArgs.push_back(make_unique<Var>("O"));
        vector<unique_ptr<Expr>> domArgs3;
        domArgs3.push_back(make_unique<FuncCall>("'", std::move(oprimeArgs)));
        postArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs3)));
        auto post = make_unique<FuncCall>("in", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "updateOrderStatusAgentOk"
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
