#include "ProgramAnalyzer.h"

using namespace std;

void ProgramAnalyzer::analyze() {
    // running analyzers for the functions
    for (auto& function : program->getFunctions()) {
        // first recursive functions determination
        set<shared_ptr<Function> > visitedFunctions;
        function->setRecursive(isFunctionRecursive(function, function, visitedFunctions));

        // then variables determination
        visitedFunctions.clear();
        set<string> readVars, writeVars;
        determineFunctionVariables(function, readVars, writeVars, visitedFunctions);
        function->setReadVariables(readVars);
        function->setWriteVariables(writeVars);

        // TODO : other analysis
    }
}

bool ProgramAnalyzer::isFunctionRecursive(shared_ptr<Function> origFunction,
                                          shared_ptr<Function> currFunction,
                                          set<shared_ptr<Function> >& visitedFunctions) {
    // TODO
    return false;
}

void ProgramAnalyzer::determineFunctionVariables(shared_ptr<Function> currFunction,
                                                 set<string>& readVars,
                                                 set<string>& writeVars,
                                                 set<shared_ptr<Function> >& visitedFunctions) {
    // TODO
}