#include <iostream>

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

    // just some debug prints
    for (auto& function : program->getFunctions()) {
        cout << "function " << function->getName() << ":" << endl;
        cout << "  - recursive: " << function->isRecursive() << endl;

        cout << "  - readVars: ";
        for (auto& var : function->getReadVariables()) cout << var << " ";
        cout << endl;

        cout << "  - writeVars: ";
        for (auto& var : function->getWriteVariables()) cout << var << " ";
        cout << endl;
    }
}

bool ProgramAnalyzer::isFunctionRecursive(shared_ptr<Function> origFunction, shared_ptr<Function> currFunction,
                                          set<shared_ptr<Function> >& visitedFunctions) {
    visitedFunctions.insert(currFunction);

    for (auto& stat : currFunction->getStatements()) {
        if (isFunctionStatementRecursive(origFunction, stat, visitedFunctions)) return true;
    }
    return false;
}

bool ProgramAnalyzer::isFunctionStatementRecursive(shared_ptr<Function> origFunction, shared_ptr<Statement> currStatement,
                                                   set<std::shared_ptr<Function> >& visitedFunctions) {
    if (dynamic_pointer_cast<CallAssignment>(currStatement)) {
        auto callFunction = program->getFunction(dynamic_pointer_cast<CallAssignment>(currStatement)->getFunctionName());
        if (callFunction == origFunction) return true;

        if (callFunction && visitedFunctions.find(callFunction) == visitedFunctions.end()) {
            if (isFunctionRecursive(origFunction, callFunction, visitedFunctions)) return true;
        }
    }
    else if (dynamic_pointer_cast<Condition>(currStatement)) {
        for (auto& stat : dynamic_pointer_cast<Condition>(currStatement)->getThenStatements()) {
            if (isFunctionStatementRecursive(origFunction, stat, visitedFunctions)) return true;
        }
        for (auto& stat : dynamic_pointer_cast<Condition>(currStatement)->getElseStatements()) {
            if (isFunctionStatementRecursive(origFunction, stat, visitedFunctions)) return true;
        }
    }

    return false;
}

void ProgramAnalyzer::determineFunctionVariables(shared_ptr<Function> currFunction,
                                                 set<string>& readVars, set<string>& writeVars,
                                                 set<shared_ptr<Function> >& visitedFunctions) {
    visitedFunctions.insert(currFunction);

    for (auto& stat : currFunction->getStatements()) {
        determineFunctionStatementVariables(stat, readVars, writeVars, visitedFunctions);
    }
}

void ProgramAnalyzer::determineFunctionStatementVariables(shared_ptr<Statement> currStatement,
                                                          set<string>& readVars, set<string>& writeVars,
                                                          set<shared_ptr<Function> >& visitedFunctions) {
    if (dynamic_pointer_cast<Assignment>(currStatement)) {
        auto target = dynamic_pointer_cast<Assignment>(currStatement)->getTarget();
        if (target->isGlobal()) writeVars.insert(target->getName());

        if (dynamic_pointer_cast<CallAssignment>(currStatement)) {
            auto assignment = dynamic_pointer_cast<CallAssignment>(currStatement);
            for (auto& val : assignment->getFunctionArgs()) {
                if (dynamic_pointer_cast<IdentifierValue>(val)) {
                    if (dynamic_pointer_cast<IdentifierValue>(val)->getIdentifier()->isGlobal()) {
                        readVars.insert(dynamic_pointer_cast<IdentifierValue>(val)->getIdentifier()->getName());
                    }
                }
            }

            auto callFunction = program->getFunction(assignment->getFunctionName());
            if (callFunction && visitedFunctions.find(callFunction) == visitedFunctions.end()) {
                determineFunctionVariables(callFunction, readVars, writeVars, visitedFunctions);
            }
        }
        else if (dynamic_pointer_cast<IdentifierAssignment>(currStatement)) {
            if (dynamic_pointer_cast<IdentifierAssignment>(currStatement)->getValue()->getIdentifier()->isGlobal()) {
                readVars.insert(dynamic_pointer_cast<IdentifierAssignment>(currStatement)->getValue()->getIdentifier()->getName());
            }
        }
    }
    else if (dynamic_pointer_cast<Return>(currStatement)) {
        auto val = dynamic_pointer_cast<Return>(currStatement)->getValue();
        if (dynamic_pointer_cast<IdentifierValue>(val)) {
            if (dynamic_pointer_cast<IdentifierValue>(val)->getIdentifier()->isGlobal()) {
                readVars.insert(dynamic_pointer_cast<IdentifierValue>(val)->getIdentifier()->getName());
            }
        }
    }
    else if (dynamic_pointer_cast<Condition>(currStatement)) {
        for (auto& stat : dynamic_pointer_cast<Condition>(currStatement)->getThenStatements()) {
            determineFunctionStatementVariables(stat, readVars, writeVars, visitedFunctions);
        }
        for (auto& stat : dynamic_pointer_cast<Condition>(currStatement)->getElseStatements()) {
            determineFunctionStatementVariables(stat, readVars, writeVars, visitedFunctions);
        }
    }
}
