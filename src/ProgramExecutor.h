#ifndef PROGRAMEXECUTOR_H
#define PROGRAMEXECUTOR_H

#include <map>
#include <string>
#include <memory>

#include "Program.h"

// Objects
class ExecValue {

};

class ExecObject : public ExecValue {
public:
    std::shared_ptr<ExecValue> getFieldByPath(const std::string&) const;
    void setFieldByPath(const std::string&, std::shared_ptr<ExecValue>);

    std::shared_ptr<ExecValue> getField(const std::string& name) {
        return fields[name];
    }

    void setField(const std::string& name, std::shared_ptr<ExecValue> val) {
        fields[name] = val;
    }

private:
    std::map<std::string, std::shared_ptr<ExecValue> > fields;
};

class ExecPrimitive : public ExecValue {
};

template<class T> class ExecPrimitiveTemplate : public ExecPrimitive {
public:
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
};

class ExecInteger : public ExecPrimitiveTemplate<long long> {
};

class ExecFloat : public ExecPrimitiveTemplate<float> {
};

class ExecChar : public ExecPrimitiveTemplate<char32_t > {
};

class ExecString : public ExecPrimitiveTemplate<std::u32string> {
};

// Executor
class ProgramExecutor {
public:
    ProgramExecutor(std::shared_ptr<Program> program) :
            program(std::move(program)), global(std::make_shared<ExecObject>()) { }

    std::shared_ptr<ExecValue> exec(std::shared_ptr<ExecObject>);

private:
    std::shared_ptr<ExecValue> execFunction(std::shared_ptr<Function>, std::shared_ptr<ExecObject>);

    std::shared_ptr<Program> program;
    std::shared_ptr<ExecObject> global;

};


#endif
