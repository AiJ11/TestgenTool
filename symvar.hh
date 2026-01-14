#ifndef SYMVAR_HH
#define SYMVAR_HH

#include <memory>
#include "ast.hh"

using namespace std;

class SymVar : public Expr {
    private:
        static unsigned int count;
        unsigned int num;
    public:
        SymVar(unsigned int);
        static unique_ptr<SymVar>getNewSymVar();
        virtual void accept(ASTVisitor&);
        bool operator == (SymVar&);
        unsigned int getNum() const { return num; }
};
#endif