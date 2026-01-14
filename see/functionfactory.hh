#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>


class Expr;

using namespace std;
class Function {
    protected:
        const vector<Expr*> arguments;
    public:
        virtual ~Function() = default;  
        virtual unique_ptr<Expr> execute() = 0;
};

class FunctionFactory {
    public:
        virtual unique_ptr<Function> getFunction(string fname, vector<Expr*> args) = 0;

    protected:
};

template <typename DerivedType, typename BaseType>
unique_ptr<DerivedType> dynamic_pointer_cast(std::unique_ptr<BaseType>&);
