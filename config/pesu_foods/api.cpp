#include "../../APIFunction.hpp"                    // Go up 2 levels to root
#include "../executors/PesuFoodsExecutor.hpp"      // Relative to config/
#include <iostream>

// Helper functions
namespace {
    std::string extractValue(const Expr& expr) {
        if (auto* str = dynamic_cast<const String*>(&expr)) {
            return str->value;
        }
        if (auto* num = dynamic_cast<const Num*>(&expr)) {
            return std::to_string(num->value);
        }
        throw std::runtime_error("Cannot extract value from non-concrete Expr");
    }
    
    std::unique_ptr<Expr> cloneExpr(const Expr* expr) {
        if (auto* str = dynamic_cast<const String*>(expr)) {
            return std::make_unique<String>(str->value);
        }
        if (auto* num = dynamic_cast<const Num*>(expr)) {
            return std::make_unique<Num>(num->value);
        }
        if (auto* var = dynamic_cast<const Var*>(expr)) {
            return std::make_unique<Var>(var->name);
        }
        throw std::runtime_error("Unsupported Expr type for cloning");
    }
}

namespace PesuFoods {
// SIGNUP FUNCTION
class SignupFunction : public APIFunction {
private:
    std::unique_ptr<Expr> nameExpr;
    std::unique_ptr<Expr> emailExpr;
    std::unique_ptr<Expr> passwordExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 3) {
            nameExpr = cloneExpr(params[0].get());
            emailExpr = cloneExpr(params[1].get());
            passwordExpr = cloneExpr(params[2].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string name = extractValue(*nameExpr);
        std::string email = extractValue(*emailExpr);
        std::string password = extractValue(*passwordExpr);
        
        std::cout << "[SignupFunction] Executing: " << email << std::endl;
        
        auto resp = PesuFoodsExecutor::signup(name, email, password);
        
        return std::make_unique<Num>(resp.statusCode);
    }
    
    std::string getName() const override {
        return "signup";
    }
};
// LOGIN FUNCTION
class LoginFunction : public APIFunction {
private:
    std::unique_ptr<Expr> emailExpr;
    std::unique_ptr<Expr> passwordExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 2) {
            emailExpr = cloneExpr(params[0].get());
            passwordExpr = cloneExpr(params[1].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string email = extractValue(*emailExpr);
        std::string password = extractValue(*passwordExpr);
        
        std::cout << "[LoginFunction] Executing: " << email << std::endl;
        
        auto resp = PesuFoodsExecutor::login(email, password);
        
        return std::make_unique<Num>(resp.statusCode);
    }
    
    std::string getName() const override {
        return "login";
    }
};
// GET ACCOUNT FUNCTION

class GetAccountFunction : public APIFunction {
private:
    std::unique_ptr<Expr> tokenExpr;
    
public:
    void setParameters(const std::vector<std::unique_ptr<Expr>>& params) override {
        if (params.size() >= 1) {
            tokenExpr = cloneExpr(params[0].get());
        }
    }
    
    std::unique_ptr<Expr> execute() override {
        std::string token = extractValue(*tokenExpr);
        
        std::cout << "[GetAccountFunction] Executing" << std::endl;
        
        auto resp = PesuFoodsExecutor::getAccount(token);
        
        return std::make_unique<Num>(resp.statusCode);
    }
    
    std::string getName() const override {
        return "getAccount";
    }
};

// FUNCTION FACTORY

class PesuFoodsFunctionFactory : public APIFunctionFactory {
public:
    std::unique_ptr<APIFunction> getFunction(
        const std::string& funcName,
        const std::vector<std::unique_ptr<Expr>>& args
    ) override {
        std::unique_ptr<APIFunction> func;
        
        if (funcName == "signup") {
            func = std::make_unique<SignupFunction>();
        }
        else if (funcName == "login") {
            func = std::make_unique<LoginFunction>();
        }
        else if (funcName == "getAccount") {
            func = std::make_unique<GetAccountFunction>();
        }
        
        if (func) {
            func->setParameters(args);
        }
        
        return func;
    }
    
    bool hasFunction(const std::string& funcName) const override {
        return (funcName == "signup" || 
                funcName == "login" || 
                funcName == "getAccount");
    }
};
}

// Factory getter function - returns raw pointer to static instance
APIFunctionFactory* getPesuFoodsFunctionFactory() {
    static PesuFoods::PesuFoodsFunctionFactory factory;  // Static lifetime - lives for entire program
    std::cout << "[Factory] Returning static factory instance at " << &factory << std::endl;
    return &factory;
}
