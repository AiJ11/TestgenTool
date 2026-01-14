#include "symvar.hh"

unsigned int SymVar::count = 0;

SymVar::SymVar(unsigned int n ) : Expr(ExprType::SYMVAR), num(n) {}

unique_ptr<SymVar> SymVar::getNewSymVar() {
    unique_ptr<SymVar> var = make_unique<SymVar>(count);
    count++;
    return var;
}

void SymVar::accept(ASTVisitor& visitor) {}

bool SymVar::operator == (SymVar& var) {
    return num == var.num;
}
