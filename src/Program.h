#ifndef PROGRAM_H
#define PROGRAM_H

#include <set>
#include <map>
#include <vector>
#include <memory>
#include <string>
#include <locale>
#include <codecvt>
#include <algorithm>

extern std::string GLOBAL_PREFIX;
extern std::string LOCAL_PREFIX;

// Utils
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

// Values
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

// Expressions
class Expression {
public:
    virtual bool isUndetermined() const = 0;
    virtual std::string toString() const = 0;
};

class CallExpression : public Expression {
public:
    explicit CallExpression(std::string name, std::vector<std::shared_ptr<Expression> > arguments)
            : name(std::move(name)), arguments(std::move(arguments)) { }

    const std::string& getName() const {
        return name;
    }

    const std::vector<std::shared_ptr<Expression> >& getArguments() const {
        return arguments;
    }

    bool isUndetermined() const override {
        for (auto& arg : arguments) {
            if (!arg || arg->isUndetermined()) return true;
        }
        return false;
    }

    std::string toString() const override {
        std::string res;
        res += "(";
        res += "<" + name + ">";
        for (auto& arg : arguments) res += " " + arg->toString();
        res += ")";
        return res;
    }

private:
    std::string name;
    std::vector<std::shared_ptr<Expression> > arguments;
};

class ConditionExpression : public Expression {
public:
    ConditionExpression(std::shared_ptr<Expression> conditionExpression, std::shared_ptr<Expression> thenExpression,
                        std::shared_ptr<Expression> elseExpression) :
            conditionExpression(std::move(conditionExpression)), thenExpression(std::move(thenExpression)),
            elseExpression(std::move(elseExpression)) { }

    const std::shared_ptr<Expression>& getConditionExpression() const {
        return conditionExpression;
    }

    const std::shared_ptr<Expression>& getThenExpression() const {
        return thenExpression;
    }

    const std::shared_ptr<Expression>& getElseExpression() const {
        return elseExpression;
    }

    bool isUndetermined() const override {
        return !conditionExpression || !thenExpression || !elseExpression || conditionExpression->isUndetermined() ||
               thenExpression->isUndetermined() || elseExpression->isUndetermined();
    }

    std::string toString() const override {
        std::string res;
        res += "(<if> ";
        res += conditionExpression->toString();
        res += " ";
        res += thenExpression->toString();
        res += " ";
        res += elseExpression->toString();
        res += ")";
        return res;
    }

private:
    std::shared_ptr<Expression> conditionExpression;
    std::shared_ptr<Expression> thenExpression;
    std::shared_ptr<Expression> elseExpression;
};

class ValueExpression : public Expression {
public:
    explicit ValueExpression(std::shared_ptr<Value> value) : value(std::move(value)) { }

    const std::shared_ptr<Value>& getValue() const {
        return value;
    }

    bool isUndetermined() const override {
        return false;
    }

    std::string toString() const override {
        if (std::dynamic_pointer_cast<BooleanValue>(value)) {
            return std::dynamic_pointer_cast<BooleanValue>(value)->getValue() ? "true" : "false";
        }
        if (std::dynamic_pointer_cast<IntegerValue>(value)) {
            return std::to_string(std::dynamic_pointer_cast<IntegerValue>(value)->getValue());
        }
        if (std::dynamic_pointer_cast<FloatValue>(value)) {
            return std::to_string(std::dynamic_pointer_cast<FloatValue>(value)->getValue());
        }
        if (std::dynamic_pointer_cast<CharValue>(value)) {
            return "'" + utfConverter.to_bytes(std::dynamic_pointer_cast<CharValue>(value)->getValue()) + "'";
        }
        if (std::dynamic_pointer_cast<StringValue>(value)) {
            return "\"" + utfConverter.to_bytes(std::dynamic_pointer_cast<StringValue>(value)->getValue()) + "\"";
        }
        if (std::dynamic_pointer_cast<IdentifierValue>(value)) {
            return std::dynamic_pointer_cast<IdentifierValue>(value)->getIdentifier()->getFullName();
        }
        return "<value>";
    }

private:
    std::shared_ptr<Value> value;
    mutable std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utfConverter;
};

class UndeterminedExpression : public Expression {
public:
    bool isUndetermined() const override {
        return true;
    }

    std::string toString() const override {
        return "<undetermined>";
    }
};

// Statements
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

// TopLevel structures
class Function {
public:
    explicit Function(const std::string& name) : name(name), recursive(false) {
    }

    bool isUsingGlobal() const {
        return readVariables.size() > 0 || writeVariables.size() > 0;
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
        return writeVariables;
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

    std::map<std::string, std::set<std::shared_ptr<Expression> > >& getReadExpressions() {
        return readExpressions;
    }

    void setReadExpressions(const std::map<std::string, std::set<std::shared_ptr<Expression> > >& expressions) {
        readExpressions = expressions;
    }

    std::map<std::string, std::set<std::shared_ptr<Expression> > >& getWriteExpressions() {
        return writeExpressions;
    }

    void setWriteExpressions(const std::map<std::string, std::set<std::shared_ptr<Expression> > > &expressions) {
        writeExpressions = expressions;
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

    std::map<std::string, std::set<std::shared_ptr<Expression> > > readExpressions;
    std::map<std::string, std::set<std::shared_ptr<Expression> > > writeExpressions;

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
