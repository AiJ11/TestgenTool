#ifndef TESTER_HH
#define TESTER_HH

#include <memory>
#include <string>
#include <vector>

#include "../ast.hh"
#include "../env.hh"
#include "../see/see.hh"
#include "../see/z3solver.hh"
using namespace std;
class Tester {
    private:
        SEE see;
        Z3Solver solver;
        vector<Expr*> pathConstraints;
        
        
    public:
        Tester(FunctionFactory* functionFactory) : see(functionFactory), solver(), pathConstraints() {}
        void generateTest();
        
        // Public methods for testing
        unique_ptr<Program> generateATC(unique_ptr<Spec>, vector<string>);
        unique_ptr<Program> generateCTC(unique_ptr<Program>, vector<Expr*> ConcreteVals, ValueEnvironment* ve);
        unique_ptr<Program> rewriteATC(unique_ptr<Program>&, vector<Expr*> ConcreteVals);
        
        // Getters for testing
        SEE& getSEE() { return see; }
        Z3Solver& getSolver() { return solver; }
        vector<Expr*>& getPathConstraints() { return pathConstraints; }
};
#endif
