#ifndef PROGRAM_EXECUTOR_H
#define PROGRAM_EXECUTOR_H

#include <map>
#include <string>
#include <memory>
#include <locale>
#include <codecvt>
#include <functional>

#include "Program.h"

// Objects
class ExecValue {
public:
    virtual std::shared_ptr<ExecValue> clone() const = 0;
    virtual std::string toString() const = 0;
};

class ExecObject : public ExecValue {
public:
    void ensureFieldPath(const std::string&, bool);

    std::shared_ptr<ExecValue> getFieldByPath(const std::string&) const;
    void setFieldByPath(const std::string&, std::shared_ptr<ExecValue>);

    std::shared_ptr<ExecValue> clone() const override;
    std::string toString() const override;

    std::shared_ptr<ExecValue> getField(const std::string& name) {
        return fields[name];
    }

    void setField(const std::string& name, std::shared_ptr<ExecValue> val) {
        fields[name] = val;
    }

private:
    std::vector<std::string> stickyFieldPaths;
    mutable std::map<std::string, std::shared_ptr<ExecValue> > fields;
};

class ExecPrimitive : public ExecValue {
};

class ExecNull : public ExecPrimitive {
public:
    std::shared_ptr<ExecValue> clone() const override {
        return std::make_shared<ExecNull>();
    }

    std::string toString() const override {
        return "null";
    }
};

template<class T> class ExecPrimitiveTemplate : public ExecPrimitive {
public:
    explicit ExecPrimitiveTemplate(T val) : value(std::move(val)) { };

    const T& getValue() const {
        return value;
    }

    void setValue(const T& value) {
        ExecPrimitiveTemplate::value = value;
    }

private:
    T value;
};

class ExecBoolean : public ExecPrimitiveTemplate<bool> {
public:
    ExecBoolean() : ExecPrimitiveTemplate(false) { };
    explicit ExecBoolean(bool val) : ExecPrimitiveTemplate(val) { }

    std::shared_ptr<ExecValue> clone() const override {
        auto res = std::make_shared<ExecBoolean>();
        res->setValue(getValue());
        return res;
    }

    std::string toString() const override {
        return getValue() ? "true" : "false";
    }
};

class ExecInteger : public ExecPrimitiveTemplate<long long> {
public:
    ExecInteger() : ExecPrimitiveTemplate(0) { }
    explicit ExecInteger(long long val) : ExecPrimitiveTemplate(val) { }

    std::shared_ptr<ExecValue> clone() const override {
        auto res = std::make_shared<ExecInteger>();
        res->setValue(getValue());
        return res;
    }

    std::string toString() const override {
        return std::to_string(getValue()) ;
    }
};

class ExecFloat : public ExecPrimitiveTemplate<double> {
public:
    ExecFloat() : ExecPrimitiveTemplate(0.0) { }
    explicit ExecFloat(double val) : ExecPrimitiveTemplate(val) { }

    std::shared_ptr<ExecValue> clone() const override {
        auto res = std::make_shared<ExecFloat>();
        res->setValue(getValue());
        return res;
    }

    std::string toString() const override {
        return std::to_string(getValue());
    }
};

class ExecChar : public ExecPrimitiveTemplate<char32_t > {
public:
    ExecChar() : ExecPrimitiveTemplate('\0') { }
    explicit ExecChar(char32_t val) : ExecPrimitiveTemplate(val) { }

    std::shared_ptr<ExecValue> clone() const override {
        auto res = std::make_shared<ExecChar>();
        res->setValue(getValue());
        return res;
    }

    std::string toString() const override {
        return "'" + utfConverter.to_bytes(getValue()) + "'";
    }

private:
    mutable std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utfConverter;
};

class ExecString : public ExecPrimitiveTemplate<std::u32string> {
public:
    ExecString() : ExecPrimitiveTemplate(U"") { }
    explicit ExecString(const std::u32string& val) : ExecPrimitiveTemplate(val) { };

    void setValueUTF8(const std::string& value) {
        setValue(utfConverter.from_bytes(value));
    }

    std::shared_ptr<ExecValue> clone() const override {
        auto res = std::make_shared<ExecString>();
        res->setValue(getValue());
        return res;
    }

    std::string toString() const override {
        return "\"" + utfConverter.to_bytes(getValue()) + "\"";
    }

private:
    mutable std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utfConverter;
};

// Executor
class ProgramExecutor {
public:
    explicit ProgramExecutor(std::shared_ptr<Program>);

    std::shared_ptr<ExecValue> exec(std::shared_ptr<ExecValue>);
    std::shared_ptr<ExecValue> execExpression(std::shared_ptr<Expression>);

    std::shared_ptr<Program> getProgram() {
        return program;
    }

protected:
    virtual std::shared_ptr<ExecObject> getReadGlobal() const = 0;
    virtual std::shared_ptr<ExecObject> getWriteGlobal() const = 0;

private:
    std::shared_ptr<ExecValue> execFunction(std::shared_ptr<Function>,
            std::shared_ptr<ExecObject>, std::shared_ptr<ExecObject>, std::shared_ptr<ExecObject>);

    std::shared_ptr<ExecValue> execStatement(std::shared_ptr<Statement>,
            std::shared_ptr<ExecObject>, std::shared_ptr<ExecObject>, std::shared_ptr<ExecObject>);

    std::shared_ptr<ExecValue> execValue(std::shared_ptr<Value>,
            std::shared_ptr<ExecObject>, std::shared_ptr<ExecObject>, std::shared_ptr<ExecObject>);

    std::shared_ptr<Program> program;
};

#endif
