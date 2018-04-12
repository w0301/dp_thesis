#include <stdexcept>

#include "BuiltInFunction.h"
#include "ProgramExecutor.h"

using namespace std;

BuiltInFunction::BuiltInFunction(string name, int argsCount, function<shared_ptr<ExecValue>(const BuiltInArguments&)> func) :
        name(move(name)), argsCount(argsCount), func(move(func)) {
};

shared_ptr<ExecValue> BuiltInFunction::operator()(const BuiltInArguments& args) const {
    if (args.size() != argsCount) {
        throw logic_error("Wrong arguments count for the function '" + name + "'.");
    }

    auto res = func(args);
    if (!res) throw logic_error("Wrong arguments types for the function '" + name + "'.");

    return res;
}

std::map<std::string, BuiltInFunction> BUILT_IN_FUNCTIONS;

void addBuiltInFunction(const std::string& name, int argsCount, std::function<std::shared_ptr<ExecValue>(const BuiltInArguments&)> func) {
    BUILT_IN_FUNCTIONS[name] = BuiltInFunction(name, argsCount, std::move(func));
}

void initBuiltInFunctions() {
    // integer / floating-point arithmetics
    addBuiltInFunction("_add", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecInteger>(0)->getValue() + args.get<ExecInteger>(1)->getValue())
            );
        }
        if (args.get<ExecFloat>(0) && args.get<ExecFloat>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecFloat>(args.get<ExecFloat>(0)->getValue() + args.get<ExecFloat>(1)->getValue())
            );
        }
        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_sub", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecInteger>(0)->getValue() - args.get<ExecInteger>(1)->getValue())
            );
        }
        if (args.get<ExecFloat>(0) && args.get<ExecFloat>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecFloat>(args.get<ExecFloat>(0)->getValue() - args.get<ExecFloat>(1)->getValue())
            );
        }
        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_mul", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecInteger>(0)->getValue() * args.get<ExecInteger>(1)->getValue())
            );
        }
        if (args.get<ExecFloat>(0) && args.get<ExecFloat>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecFloat>(args.get<ExecFloat>(0)->getValue() * args.get<ExecFloat>(1)->getValue())
            );
        }
        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_div", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecInteger>(0)->getValue() / args.get<ExecInteger>(1)->getValue())
            );
        }
        if (args.get<ExecFloat>(0) && args.get<ExecFloat>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecFloat>(args.get<ExecFloat>(0)->getValue() / args.get<ExecFloat>(1)->getValue())
            );
        }
        return shared_ptr<ExecValue>();
    });

    // boolean logic

}
