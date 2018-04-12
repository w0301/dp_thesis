#ifndef BUILTIN_FUNCTION_H
#define BUILTIN_FUNCTION_H

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <functional>

class ExecValue;

class BuiltInArguments {
public:
    explicit BuiltInArguments(std::vector<std::shared_ptr<ExecValue> > args) : args(std::move(args)) { }

    size_t size() const {
        return args.size();
    }

    template<class T> std::shared_ptr<T> get(size_t index) const {
        return std::dynamic_pointer_cast<T>(args[index]);
    }

private:
    std::vector<std::shared_ptr<ExecValue> > args;
};

class BuiltInFunction {
public:
    BuiltInFunction() = default;
    BuiltInFunction(std::string, int, std::function<std::shared_ptr<ExecValue>(const BuiltInArguments&)>);

    std::shared_ptr<ExecValue> operator()(const BuiltInArguments&) const;

    bool isDefined() const {
        return (bool)func;
    }

    const std::string& getName() const {
        return name;
    }

    int getArgsCount() const {
        return argsCount;
    }

private:
    std::string name;
    int argsCount;
    std::function<std::shared_ptr<ExecValue>(const BuiltInArguments&)> func;
};

void initBuiltInFunctions();
extern std::map<std::string, BuiltInFunction> BUILT_IN_FUNCTIONS;

#endif
