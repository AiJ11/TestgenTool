#pragma once
#include "ast.hpp"  // 
#include <string>
#include <vector>
#include <memory>

/**
 * Base class for API function implementations
 */
class APIFunction {
public:
    virtual ~APIFunction() = default;
    
    virtual void setParameters(const std::vector<std::unique_ptr<Expr>>& params) = 0;
    virtual std::unique_ptr<Expr> execute() = 0;
    virtual std::string getName() const = 0;
};

/**
 * Function Factory
 */
class APIFunctionFactory {
public:
    virtual ~APIFunctionFactory() = default;
    
    virtual std::unique_ptr<APIFunction> getFunction(
        const std::string& funcName,
        const std::vector<std::unique_ptr<Expr>>& args
    ) = 0;
    
    virtual bool hasFunction(const std::string& funcName) const = 0;
};
