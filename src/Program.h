#ifndef PROGRAM_H
#define PROGRAM_H

#include <set>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

extern std::string GLOBAL_PREFIX;
extern std::string LOCAL_PREFIX;

class Identifier {
public:
    bool isGlobal() const {
        return fullName.substr(0, GLOBAL_PREFIX.length()) == GLOBAL_PREFIX;
    }

    bool isLocal() const {
        return fullName.substr(0, LOCAL_PREFIX.length()) == LOCAL_PREFIX;
    }

    const std::string& getFullName() const {
        return fullName;
    }

    void setFullName(const std::string& fullName) {
        this->fullName = fullName;
    }

    std::string getName() const {
        if (isGlobal()) return fullName.substr(GLOBAL_PREFIX.length());
        return fullName.substr(LOCAL_PREFIX.length());
    }

private:
    std::string fullName;
};

class Value {
public:
    virtual ~Value() = default;
};

class ConstantValue : public Value {
};

template<class T> class ConstantTemplateValue : public ConstantValue {
public:
    const T& getValue() const {
        return value;
    }

    void setValue(const T& value) {
        ConstantTemplateValue::value = value;
    }

protected:
    T value;
};

class BooleanValue : public ConstantTemplateValue<bool> {
};

class IntegerValue : public ConstantTemplateValue<long long> {
};

class FloatValue : public ConstantTemplateValue<double> {
};

class CharValue : public ConstantTemplateValue<char32_t > {
};

class StringValue : public ConstantTemplateValue<std::u32string> {
};

class NullValue : public ConstantValue
{
};

class IdentifierValue : public Value {
public:
    std::shared_ptr<Identifier> getIdentifier() const {
        return identifier;
    }

    void setIdentifier(std::shared_ptr<Identifier> identifier)  {
        this->identifier = identifier;
    }

private:
    std::shared_ptr<Identifier> identifier;
};

class Statement {
public:
    virtual ~Statement() = default;
};

class Assignment : public Statement {
public:
    std::shared_ptr<Identifier> getTarget() const {
        return target;
    }

    void setTarget(std::shared_ptr<Identifier> target)  {
        this->target = target;
    }

private:
    std::shared_ptr<Identifier> target;
};

class CallAssignment : public Assignment {
public:
    const std::string& getFunctionName() const {
        return functionName;
    }

    void setFunctionName(const std::string& functionName)  {
        this->functionName = functionName;
    }

    const std::vector<std::shared_ptr<Value> >& getFunctionArgs() const {
        return functionArgs;
    }

    void addFunctionArg(std::shared_ptr<Value> functionArg) {
        functionArgs.push_back(functionArg);
    }

private:
    std::string functionName;
    std::vector<std::shared_ptr<Value> > functionArgs;
};

class IdentifierAssignment : public Assignment {
public:
    std::shared_ptr<IdentifierValue> getValue() const {
        return value;
    }

    void setValue(std::shared_ptr<IdentifierValue> value)  {
        this->value = value;
    }

private:
    std::shared_ptr<IdentifierValue> value;
};

class ConstantAssignment : public Assignment {
public:
    std::shared_ptr<ConstantValue> getValue() const {
        return value;
    }

    void setValue(std::shared_ptr<ConstantValue> value)  {
        this->value = value;
    }

private:
    std::shared_ptr<ConstantValue> value;
};

class Return : public Statement {
public:
    std::shared_ptr<Value> getValue() const {
        return value;
    }

    void setValue(std::shared_ptr<Value> value)  {
        this->value = value;
    }

private:
    std::shared_ptr<Value> value;
};

class Condition : public Statement {
public:
    std::shared_ptr<IdentifierValue> getConditionValue() const {
        return conditionValue;
    }

    void setConditionValue(std::shared_ptr<IdentifierValue> conditionValue)  {
        this->conditionValue = conditionValue;
    }

    const std::vector<std::shared_ptr<Statement> >& getThenStatements() const {
        return thenStatements;
    }

    void addThenStatement(std::shared_ptr<Statement> statement) {
        thenStatements.push_back(statement);
    }

    const std::vector<std::shared_ptr<Statement> >& getElseStatements() const {
        return elseStatements;
    }

    void addElseStatement(std::shared_ptr<Statement> statement) {
        elseStatements.push_back(statement);
    }

private:
    std::shared_ptr<IdentifierValue> conditionValue;
    std::vector<std::shared_ptr<Statement> > thenStatements;
    std::vector<std::shared_ptr<Statement> > elseStatements;
};

class Function {
public:
    explicit Function(const std::string& name) : name(name) {
    }

    bool isRecursive() const {
        return recursive;
    }

    void setRecursive(bool val) {
        recursive = val;
    }

    const std::set<std::string>& getReadVariables() const {
        return readVariables;
    }

    void setReadVariables(const std::set<std::string>& vars) {
        readVariables = vars;
    }

    const std::set<std::string>& getWriteVariables() const {
        return readVariables;
    }

    void setWriteVariables(const std::set<std::string>& vars) {
        writeVariables = vars;
    }

    std::set<std::string> getAllVariables() {
        std::set<std::string> res;
        for (auto& var : readVariables) res.insert(var.substr(GLOBAL_PREFIX.length()));
        for (auto& var : writeVariables) res.insert(var.substr(GLOBAL_PREFIX.length()));
        return res;
    }

    const std::string& getName() const {
        return name;
    }

    const std::vector<std::string>& getArguments() const {
        return arguments;
    }

    void addArgument(const std::string& argument) {
        arguments.push_back(argument);
    }

    const std::vector<std::shared_ptr<Statement> >& getStatements() const {
        return statements;
    }

    void addStatement(std::shared_ptr<Statement> statement) {
        statements.push_back(statement);
    }

private:
    bool recursive;
    std::set<std::string> readVariables;
    std::set<std::string> writeVariables;

    std::string name;
    std::vector<std::string> arguments;
    std::vector<std::shared_ptr<Statement> > statements;
};

class Program {
public:

    std::shared_ptr<Function> getFunction(std::string name) const {
        auto func = std::find_if(functions.begin(), functions.end(), [&](auto func) { return func->getName() == name; });
        if (func == functions.end()) return std::shared_ptr<Function>();

        return *func;
    }

    const std::vector<std::shared_ptr<Function> >& getFunctions() const {
        return functions;
    }

    void addFunction(std::shared_ptr<Function> function) {
        functions.push_back(function);
    }

private:
    std::vector<std::shared_ptr<Function> > functions;
};

#endif
