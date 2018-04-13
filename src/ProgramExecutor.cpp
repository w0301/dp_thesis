#include <stdexcept>

#include "ProgramExecutor.h"
#include "BuiltInFunction.h"

using namespace std;

string paddNewLines(const string& str, const string& padder) {
    string res = str;

    size_t pos = 0;
    while((pos = res.find("\n", pos == 0 ? 0 : pos + 1)) != string::npos) res.replace(pos, 1, "\n" + padder);

    return res;
}

// Objects
void ExecObject::ensureFieldPath(const string& path) {
    size_t dotPos = path.find('.');
    if (dotPos == string::npos) {
        // just reassigning to ensure that pointer (maybe NULL) is created
        fields[path] = fields[path];
    }
    else {
        string field = path.substr(0, dotPos);
        string subPath = path.substr(dotPos + 1);

        // recursively creating objects
        auto subObj = make_shared<ExecObject>();
        subObj->ensureFieldPath(subPath);
        fields[field] = subObj;
    }
}

shared_ptr<ExecValue> ExecObject::getFieldByPath(const string& path) const {
    size_t dotPos = path.find('.');
    if (dotPos == string::npos) return fields[path];

    string field = path.substr(0, dotPos);
    string subPath = path.substr(dotPos + 1);

    auto subObj = fields[field];
    if (subObj) {
        if (dynamic_pointer_cast<ExecObject>(subObj)) {
            return dynamic_pointer_cast<ExecObject>(subObj)->getFieldByPath(subPath);
        }
        else {
            throw logic_error("Bad return -> non-object value exists in the path.");
        }
    }

    // Note: this is not a logic error like above!!!
    return std::shared_ptr<ExecValue>();
}

void ExecObject::setFieldByPath(const string& path, shared_ptr<ExecValue> val) {
    // we do store special null values!
    if (dynamic_pointer_cast<ExecNull>(val)) val = shared_ptr<ExecValue>();

    // TODO : ensure needed sub-paths in the val here!!!

    size_t dotPos = path.find('.');
    if (dotPos == string::npos) {
        fields[path] = val;
        return;
    }

    string field = path.substr(0, dotPos);
    string subPath = path.substr(dotPos + 1);

    auto subObj = fields[field];
    if (!subObj) {
        subObj = make_shared<ExecObject>();
        fields[field] = subObj;
    }

    if (dynamic_pointer_cast<ExecObject>(subObj)) {
        dynamic_pointer_cast<ExecObject>(subObj)->setFieldByPath(subPath, val);
    }
    else {
        throw logic_error("Bad assignment -> non-object value exists in the path.");
    }
}

shared_ptr<ExecValue> ExecObject::clone() const {
    auto res = make_shared<ExecObject>();
    for (auto& val : fields) res->setField(val.first, val.second ? val.second->clone() : shared_ptr<ExecValue>());
    return res;
}

string ExecObject::toString() const {
    string res = "{\n";
    for (auto& val : fields) {
        res += "  " + val.first + ": ";
        res += paddNewLines(val.second ? val.second->toString() : "null", "  ") + "\n";
    }
    return res + "}";
}

// Executor
ProgramExecutor::ProgramExecutor(shared_ptr<Program> program) :
        program(move(program)), global(make_shared<ExecObject>()) {
    initBuiltInFunctions();
}

shared_ptr<ExecValue> ProgramExecutor::exec(shared_ptr<ExecValue> arg) {
    auto mainFunction = program->getFunction("main");
    if (!mainFunction) throw logic_error("No function with name 'main' defined.");

    // setup local context for the function
    auto local = make_shared<ExecObject>();
    for (auto& argName : mainFunction->getArguments()) local->setField(argName, arg->clone());

    // execute function within the context
    return execFunction(mainFunction, local);
}

shared_ptr<ExecValue> ProgramExecutor::execFunction(shared_ptr<Function> function, shared_ptr<ExecObject> local) {
    for (auto& statement : function->getStatements()) {
        auto res = execStatement(statement, local);
        if (res) return res;
    }
    return shared_ptr<ExecValue>();
}

shared_ptr<ExecValue> ProgramExecutor::execStatement(shared_ptr<Statement> statement, shared_ptr<ExecObject> local) {
    if (dynamic_pointer_cast<Return>(statement)) {
        return execValue(dynamic_pointer_cast<Return>(statement)->getValue(), local);
    }
    else if (dynamic_pointer_cast<Condition>(statement)) {
        auto cond = dynamic_pointer_cast<Condition>(statement);
        auto condValue = dynamic_pointer_cast<ExecBoolean>(execValue(cond->getConditionValue(), local));
        if (!condValue) throw logic_error("Condition value does not evaluate to boolean.");

        if (condValue->getValue()) {
            for (auto& condStatement : cond->getThenStatements()) {
                auto res = execStatement(condStatement, local);
                if (res) return res;
            }
        }
        else {
            for (auto& condStatement : cond->getElseStatements()) {
                auto res = execStatement(condStatement, local);
                if (res) return res;
            }
        }
    }
    else if (dynamic_pointer_cast<ConstantAssignment>(statement)) {
        auto assign = dynamic_pointer_cast<ConstantAssignment>(statement);
        auto value = execValue(assign->getValue(), local);

        if (assign->getTarget()->isGlobal()) global->setFieldByPath(assign->getTarget()->getName(), value);
        else local->setFieldByPath(assign->getTarget()->getName(), value);
    }
    else if (dynamic_pointer_cast<IdentifierAssignment>(statement)) {
        auto assign = dynamic_pointer_cast<IdentifierAssignment>(statement);
        auto value = execValue(assign->getValue(), local);

        if (assign->getTarget()->isGlobal()) global->setFieldByPath(assign->getTarget()->getName(), value);
        else local->setFieldByPath(assign->getTarget()->getName(), value);
    }
    else if (dynamic_pointer_cast<CallAssignment>(statement)) {
        auto assign = dynamic_pointer_cast<CallAssignment>(statement);

        auto value = shared_ptr<ExecValue>();
        auto function = program->getFunction(assign->getFunctionName());
        if (function) {
            if (function->getArguments().size() != assign->getFunctionArgs().size()) {
                throw logic_error("Bad arguments count for the function named '" + function->getName() + "'.");
            }

            auto funcLocal = make_shared<ExecObject>();
            for (int i = 0; i < function->getArguments().size(); i++) {
                funcLocal->setField(function->getArguments()[i], execValue(assign->getFunctionArgs()[i], local));
            }
            value = execFunction(function, funcLocal);
        }
        else if (BUILT_IN_FUNCTIONS[assign->getFunctionName()].isDefined()) {
            vector<shared_ptr<ExecValue> > args;
            for (int i = 0; i < assign->getFunctionArgs().size(); i++) {
                args.push_back(execValue(assign->getFunctionArgs()[i], local));
            }
            value = BUILT_IN_FUNCTIONS[assign->getFunctionName()](BuiltInArguments(args));
        }
        else {
            throw logic_error("Function with the name '" + assign->getFunctionName() + "' does not exist.");
        }

        if (assign->getTarget()->isGlobal()) global->setFieldByPath(assign->getTarget()->getName(), value);
        else local->setFieldByPath(assign->getTarget()->getName(), value);
    }

    return shared_ptr<ExecValue>();
}

shared_ptr<ExecValue> ProgramExecutor::execValue(shared_ptr<Value> value, shared_ptr<ExecObject> local) {
    if (dynamic_pointer_cast<IdentifierValue>(value)) {
        auto identifier = dynamic_pointer_cast<IdentifierValue>(value);

        auto res = shared_ptr<ExecValue>();
        if (identifier->getIdentifier()->isGlobal()) {
            res = global->getFieldByPath(identifier->getIdentifier()->getName());
        }
        else {
            res = local->getFieldByPath(identifier->getIdentifier()->getName());
        }

        // returning only if not null!
        if (res) return res->clone();
    }
    else if (dynamic_pointer_cast<BooleanValue>(value)) {
        auto res = make_shared<ExecBoolean>();
        res->setValue(dynamic_pointer_cast<BooleanValue>(value)->getValue());
        return res;
    }
    else if (dynamic_pointer_cast<IntegerValue>(value)) {
        auto res = make_shared<ExecInteger>();
        res->setValue(dynamic_pointer_cast<IntegerValue>(value)->getValue());
        return res;
    }
    else if (dynamic_pointer_cast<FloatValue>(value)) {
        auto res = make_shared<ExecFloat>();
        res->setValue(dynamic_pointer_cast<FloatValue>(value)->getValue());
        return res;
    }
    else if (dynamic_pointer_cast<CharValue>(value)) {
        auto res = make_shared<ExecChar>();
        res->setValue(dynamic_pointer_cast<CharValue>(value)->getValue());
        return res;
    }
    else if (dynamic_pointer_cast<StringValue>(value)) {
        auto res = make_shared<ExecString>();
        res->setValue(dynamic_pointer_cast<StringValue>(value)->getValue());
        return res;
    }

    // returning special null value!
    return make_shared<ExecNull>();
}