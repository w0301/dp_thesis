#include <iostream>

#include "ProgramAnalyzer.h"

using namespace std;

void ProgramAnalyzer::analyze() {
    auto trueValue = make_shared<BooleanValue>();
    trueValue->setValue(true);
    auto trueExpression = make_shared<ValueExpression>(trueValue);

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

        // extract expressions for each variable
        visitedFunctions.clear();
        map<string, shared_ptr<Expression> > globalExpressions, localExpressions;
        map<string, set<shared_ptr<Expression> > > readExpressions, writeExpressions;

        determineFunctionExpressions(function, trueExpression, globalExpressions, localExpressions, readExpressions, writeExpressions, visitedFunctions);
        function->setReadExpressions(readExpressions);
        function->setWriteExpressions(writeExpressions);
    }

    // printing analysis
    cout << "======== Code analysis ========" << endl;
    for (auto& function : program->getFunctions()) {
        cout << "function " << function->getName() << ":" << endl;
        cout << "  - recursive: " << function->isRecursive() << endl;

        cout << "  - readVars: ";
        for (auto& var : function->getReadVariables()) cout << var << " ";
        cout << endl;

        cout << "  - writeVars: ";
        for (auto& var : function->getWriteVariables()) cout << var << " ";
        cout << endl;

        cout << "  - readExpressions:" << endl;
        for (auto& exps : function->getReadExpressions()) {
            cout << "    - " << exps.first << ": " << endl;
            for (auto& exp : exps.second) {
                cout << "      - " << exp->toString() << "" << endl;
            }
        }

        cout << "  - writeExpressions:" << endl;
        for (auto& exps : function->getWriteExpressions()) {
            cout << "    - " << exps.first << ": " << endl;
            for (auto& exp : exps.second) {
                cout << "      - " << exp->toString() << "" << endl;
            }
        }
    }
    cout << "===============================" << endl << endl;
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

void ProgramAnalyzer::determineFunctionExpressions(shared_ptr<Function> currFunction, shared_ptr<Expression> currCond,
                                                   map<string, shared_ptr<Expression> >& globalExpressions,
                                                   map<string, shared_ptr<Expression> >& localExpressions,
                                                   map<string, set<shared_ptr<Expression> > >& readExpressions,
                                                   map<string, set<shared_ptr<Expression> > >& writeExpressions,
                                                   set<shared_ptr<Function> >& visitedFunctions) {
    visitedFunctions.insert(currFunction);

    for (auto& stat : currFunction->getStatements()) {
        determineFunctionStatementExpressions(stat, currCond, globalExpressions, localExpressions, readExpressions, writeExpressions, visitedFunctions);
    }
}

void ProgramAnalyzer::determineFunctionStatementExpressions(shared_ptr<Statement> currStatement, shared_ptr<Expression> currCond,
                                                            map<string, shared_ptr<Expression> >& globalExpressions,
                                                            map<string, shared_ptr<Expression> >& localExpressions,
                                                            map<string, set<shared_ptr<Expression> > >& readExpressions,
                                                            map<string, set<shared_ptr<Expression> > >& writeExpressions,
                                                            set<shared_ptr<Function> >& visitedFunctions) {
    auto trueValue = make_shared<BooleanValue>();
    trueValue->setValue(true);
    auto trueExpression = make_shared<ValueExpression>(trueValue);

    auto nullValue = make_shared<NullValue>();
    auto nullExpression = make_shared<ValueExpression>(nullValue);

    if (dynamic_pointer_cast<Return>(currStatement)) {
        auto value = dynamic_pointer_cast<Return>(currStatement)->getValue();
        shared_ptr<Expression> valueExp;

        if (dynamic_pointer_cast<IdentifierValue>(value)) {
            if (dynamic_pointer_cast<IdentifierValue>(value)->getIdentifier()->isGlobal()) {
                auto name = dynamic_pointer_cast<IdentifierValue>(value)->getIdentifier()->getName();
                valueExp = globalExpressions[name];

                // also adding currCond expression to reads if it can be determined!
                if (!currCond->isUndetermined()) readExpressions[name].insert(currCond);
                else {
                    readExpressions[name].clear();
                    readExpressions[name].insert(trueExpression);
                }
            }
            else {
                // local variables can be read even when not determined!!!
                auto oldExp = localExpressions[dynamic_pointer_cast<IdentifierValue>(value)->getIdentifier()->getName()];
                valueExp = (oldExp ? oldExp : static_pointer_cast<Expression>(make_shared<ValueExpression>(value)));
            }
        }
        else {
            valueExp = make_shared<ValueExpression>(value);
        }

        // setting actual return expression
        if (!localExpressions["&return&"]) {
            localExpressions["&return&"] = make_shared<ConditionExpression>(currCond, valueExp, nullExpression);
        }
        else {
            auto oldReturn = static_pointer_cast<ConditionExpression>(localExpressions["&return&"]);

            localExpressions["&return&"] = make_shared<ConditionExpression>(
                    oldReturn->getConditionExpression(),
                    oldReturn->getThenExpression(),
                    make_shared<ConditionExpression>(currCond, valueExp, oldReturn->getElseExpression())
            );
        }
    }
    else if (dynamic_pointer_cast<Condition>(currStatement)) {
        auto cond = dynamic_pointer_cast<Condition>(currStatement);

        shared_ptr<Expression> condExp;
        if (cond->getConditionValue()->getIdentifier()->isGlobal()) {
            condExp = globalExpressions[cond->getConditionValue()->getIdentifier()->getName()];

            // also adding currCond expression to reads if it can be determined!
            if (!currCond->isUndetermined()) readExpressions[cond->getConditionValue()->getIdentifier()->getName()].insert(currCond);
            else {
                readExpressions[cond->getConditionValue()->getIdentifier()->getName()].clear();
                readExpressions[cond->getConditionValue()->getIdentifier()->getName()].insert(trueExpression);
            }
        }
        else {
            // local variables can be read even when not determined!!!
            auto oldExp = localExpressions[cond->getConditionValue()->getIdentifier()->getName()];
            condExp = (oldExp ? oldExp : static_pointer_cast<Expression>(make_shared<ValueExpression>(cond->getConditionValue())));
        }

        // building expressions for conditions
        vector<shared_ptr<Expression> > args;
        args.push_back(currCond);
        args.push_back(condExp);
        auto thenExp = make_shared<CallExpression>("_and", args);

        args.clear();
        args.push_back(currCond);
        args.push_back(make_shared<CallExpression>("_neg", vector<shared_ptr<Expression> >(1, condExp)));
        auto elseExp = make_shared<CallExpression>("_and", args);

        // executing then branch
        map<string, shared_ptr<Expression> > thenGlobalExpressions(globalExpressions);
        map<string, shared_ptr<Expression> > thenLocalExpressions(localExpressions);
        for (auto& stat : cond->getThenStatements()) {
            determineFunctionStatementExpressions(stat, thenExp, thenGlobalExpressions, thenLocalExpressions, readExpressions, writeExpressions, visitedFunctions);
        }

        // executing else branch
        map<string, shared_ptr<Expression> > elseGlobalExpressions(globalExpressions);
        map<string, shared_ptr<Expression> > elseLocalExpressions(localExpressions);
        for (auto& stat : cond->getElseStatements()) {
            determineFunctionStatementExpressions(stat, elseExp, elseGlobalExpressions, elseLocalExpressions, readExpressions, writeExpressions, visitedFunctions);
        }

        // merging of the expressions into the former expression maps is needed!
        set<string> globalExpressionsKeys;
        for (auto& pair : thenGlobalExpressions) globalExpressionsKeys.insert(pair.first);
        for (auto& pair : elseGlobalExpressions) globalExpressionsKeys.insert(pair.first);

        for (auto& key : globalExpressionsKeys) {
            if (thenGlobalExpressions[key] != elseGlobalExpressions[key]) {
                globalExpressions[key] = make_shared<ConditionExpression>(condExp, thenGlobalExpressions[key], elseGlobalExpressions[key]);
            }
        }

        set<string> localExpressionsKeys;
        for (auto& pair : thenLocalExpressions) localExpressionsKeys.insert(pair.first);
        for (auto& pair : elseLocalExpressions) localExpressionsKeys.insert(pair.first);

        for (auto& key : localExpressionsKeys) {
            if (!thenLocalExpressions[key]) thenLocalExpressions[key] = nullExpression;
            if (!elseLocalExpressions[key]) elseLocalExpressions[key] = nullExpression;

            if (thenLocalExpressions[key] != elseLocalExpressions[key]) {
                localExpressions[key] = make_shared<ConditionExpression>(condExp, thenLocalExpressions[key], elseLocalExpressions[key]);
            }
        }
    }
    else if (dynamic_pointer_cast<Assignment>(currStatement)) {
        auto assign = dynamic_pointer_cast<Assignment>(currStatement);
        auto target = assign->getTarget();

        shared_ptr<Expression> valueExp;
        if (dynamic_pointer_cast<CallAssignment>(assign)) {
            // building args expressions
            vector<shared_ptr<Expression> > args;
            for (auto& arg : dynamic_pointer_cast<CallAssignment>(assign)->getFunctionArgs()) {
                if (dynamic_pointer_cast<IdentifierValue>(arg)) {
                    if (dynamic_pointer_cast<IdentifierValue>(arg)->getIdentifier()->isGlobal()) {
                        auto name = dynamic_pointer_cast<IdentifierValue>(arg)->getIdentifier()->getName();
                        args.push_back(globalExpressions[name]);

                        // also adding currCond expression to reads if it can be determined!
                        if (!currCond->isUndetermined()) readExpressions[name].insert(currCond);
                        else {
                            readExpressions[name].clear();
                            readExpressions[name].insert(trueExpression);
                        }
                    }
                    else {
                        // local variables can be read even when not determined!!!
                        auto oldExp = localExpressions[dynamic_pointer_cast<IdentifierValue>(arg)->getIdentifier()->getName()];
                        args.push_back(oldExp ? oldExp : static_pointer_cast<Expression>(make_shared<ValueExpression>(arg)));
                    }
                }
                else {
                    args.emplace_back(make_shared<ValueExpression>(arg));
                }
            }

            // building top expression
            bool canUseInExpression = false;
            auto function = program->getFunction(dynamic_pointer_cast<CallAssignment>(assign)->getFunctionName());

            if (function && !function->isUsingGlobal()) {
                canUseInExpression = true;
            }
            else if (function && !function->isRecursive()) {
                if (visitedFunctions.find(function) != visitedFunctions.end()) return;

                if (function->getArguments().size() != args.size()) {
                    throw logic_error("Function " + function->getName() + " called with wrong number of arguments.");
                }

                // building new local context
                map<string, shared_ptr<Expression> > newLocalExpressions;
                int i = 0;
                for (auto& argName : function->getArguments()) {
                    newLocalExpressions[argName] = args[i];

                    // adding also subexpressions
                    for (auto& arg : dynamic_pointer_cast<CallAssignment>(assign)->getFunctionArgs()) {
                        if (dynamic_pointer_cast<IdentifierValue>(arg)) {
                            auto ident = dynamic_pointer_cast<IdentifierValue>(arg)->getIdentifier();

                            if (!ident->isGlobal()) {
                                for (auto& expr : localExpressions) {
                                    if (expr.first.find(ident->getName() + ".") == 0) {
                                        newLocalExpressions[argName + expr.first.substr(ident->getName().length())] = expr.second;
                                    }
                                }
                            }
                        }
                    }

                    i += 1;
                }

                // calling recursively
                determineFunctionExpressions(function, currCond, globalExpressions, newLocalExpressions, readExpressions, writeExpressions, visitedFunctions);

                // setting return value
                valueExp = newLocalExpressions["&return&"];
            }
            else if (function && function->isRecursive()) {
                for (auto& var : function->getReadVariables()) {
                    if (!currCond->isUndetermined()) readExpressions[var].insert(currCond);
                    else {
                        readExpressions[var].clear();
                        readExpressions[var].insert(trueExpression);
                    }
                }
                for (auto& var : function->getWriteVariables()) {
                    if (!currCond->isUndetermined()) writeExpressions[var].insert(currCond);
                    else {
                        writeExpressions[var].clear();
                        writeExpressions[var].insert(trueExpression);
                    }
                }
                valueExp = make_shared<UndeterminedExpression>();
            }
            else canUseInExpression = true;

            if (canUseInExpression) {
                valueExp = make_shared<CallExpression>(dynamic_pointer_cast<CallAssignment>(assign)->getFunctionName(), args);
            }
        }
        else if (dynamic_pointer_cast<ConstantAssignment>(assign)) {
            valueExp = make_shared<ValueExpression>(dynamic_pointer_cast<ConstantAssignment>(assign)->getValue());
        }
        else if (dynamic_pointer_cast<IdentifierAssignment>(assign)) {
            auto value = dynamic_pointer_cast<IdentifierAssignment>(assign)->getValue();

            if (value->getIdentifier()->isGlobal()) {
                valueExp = globalExpressions[value->getIdentifier()->getName()];

                // also adding currCond expression to reads if it can be determined!
                if (!currCond->isUndetermined()) readExpressions[value->getIdentifier()->getName()].insert(currCond);
                else {
                    readExpressions[value->getIdentifier()->getName()].clear();
                    readExpressions[value->getIdentifier()->getName()].insert(trueExpression);
                }
            }
            else {
                // local variables can be read even when not determined!!!
                auto oldExp = localExpressions[value->getIdentifier()->getName()];
                valueExp = (oldExp ? oldExp : static_pointer_cast<Expression>(make_shared<ValueExpression>(value)));
            }

            valueExp = make_shared<ValueExpression>(dynamic_pointer_cast<IdentifierAssignment>(assign)->getValue());
        }

        // setting value
        if (target->isGlobal()) {
            globalExpressions[target->getName()] = valueExp;

            // also adding currCond expression to writes if it can be determined!
            if (!currCond->isUndetermined()) writeExpressions[target->getName()].insert(currCond);
            else {
                writeExpressions[target->getName()].clear();
                writeExpressions[target->getName()].insert(trueExpression);
            }
        }
        else {
            localExpressions[target->getName()] = valueExp;
        }
    }
}
