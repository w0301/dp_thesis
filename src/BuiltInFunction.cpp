#include <chrono>
#include <thread>
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
    // equality checking
    addBuiltInFunction("_eq", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecBoolean>(0) && args.get<ExecBoolean>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecBoolean>(0)->getValue() == args.get<ExecBoolean>(1)->getValue())
            );
        }
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecInteger>(0)->getValue() == args.get<ExecInteger>(1)->getValue())
            );
        }
        if (args.get<ExecFloat>(0) && args.get<ExecFloat>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecFloat>(0)->getValue() == args.get<ExecFloat>(1)->getValue())
            );
        }
        if (args.get<ExecChar>(0) && args.get<ExecChar>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecChar>(0)->getValue() == args.get<ExecChar>(1)->getValue())
            );
        }
        if (args.get<ExecString>(0) && args.get<ExecString>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecString>(0)->getValue() == args.get<ExecString>(1)->getValue())
            );
        }

        return static_pointer_cast<ExecValue>(make_shared<ExecBoolean>(false));
    });
    addBuiltInFunction("_neq", 2, [](const BuiltInArguments& args) {
        auto res = dynamic_pointer_cast<ExecBoolean>(BUILT_IN_FUNCTIONS["_eq"](args));
        if (!res) return static_pointer_cast<ExecValue>(make_shared<ExecBoolean>(false));
        return static_pointer_cast<ExecValue>(make_shared<ExecBoolean>(!res->getValue()));
    });

    // scalar types comparing
    addBuiltInFunction("_lt", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecInteger>(0)->getValue() < args.get<ExecInteger>(1)->getValue())
            );
        }
        if (args.get<ExecFloat>(0) && args.get<ExecFloat>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecFloat>(0)->getValue() < args.get<ExecFloat>(1)->getValue())
            );
        }
        if (args.get<ExecChar>(0) && args.get<ExecChar>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecChar>(0)->getValue() < args.get<ExecChar>(1)->getValue())
            );
        }

        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_gt", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecInteger>(0)->getValue() > args.get<ExecInteger>(1)->getValue())
            );
        }
        if (args.get<ExecFloat>(0) && args.get<ExecFloat>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecFloat>(0)->getValue() > args.get<ExecFloat>(1)->getValue())
            );
        }
        if (args.get<ExecChar>(0) && args.get<ExecChar>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecChar>(0)->getValue() > args.get<ExecChar>(1)->getValue())
            );
        }

        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_leqt", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecInteger>(0)->getValue() <= args.get<ExecInteger>(1)->getValue())
            );
        }
        if (args.get<ExecFloat>(0) && args.get<ExecFloat>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecFloat>(0)->getValue() <= args.get<ExecFloat>(1)->getValue())
            );
        }
        if (args.get<ExecChar>(0) && args.get<ExecChar>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecChar>(0)->getValue() <= args.get<ExecChar>(1)->getValue())
            );
        }

        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_geqt", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecInteger>(0)->getValue() >= args.get<ExecInteger>(1)->getValue())
            );
        }
        if (args.get<ExecFloat>(0) && args.get<ExecFloat>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecFloat>(0)->getValue() >= args.get<ExecFloat>(1)->getValue())
            );
        }
        if (args.get<ExecChar>(0) && args.get<ExecChar>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecChar>(0)->getValue() >= args.get<ExecChar>(1)->getValue())
            );
        }

        return shared_ptr<ExecValue>();
    });

    // integer-only arithmetic
    addBuiltInFunction("_mod", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecInteger>(0)->getValue() % args.get<ExecInteger>(1)->getValue())
            );
        }
        return shared_ptr<ExecValue>();
    });

    // integer / floating-point arithmetic
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
    addBuiltInFunction("_and", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecBoolean>(0) && args.get<ExecBoolean>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecBoolean>(0)->getValue() && args.get<ExecBoolean>(1)->getValue())
            );
        }
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecInteger>(0)->getValue() & args.get<ExecInteger>(1)->getValue())
            );
        }

        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_or", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecBoolean>(0) && args.get<ExecBoolean>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecBoolean>(0)->getValue() || args.get<ExecBoolean>(1)->getValue())
            );
        }
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecInteger>(0)->getValue() | args.get<ExecInteger>(1)->getValue())
            );
        }

        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_xor", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecBoolean>(0) && args.get<ExecBoolean>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(args.get<ExecBoolean>(0)->getValue() ^ args.get<ExecBoolean>(1)->getValue())
            );
        }
        if (args.get<ExecInteger>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecInteger>(0)->getValue() ^ args.get<ExecInteger>(1)->getValue())
            );
        }

        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_neg", 1, [](const BuiltInArguments& args) {
        if (args.get<ExecBoolean>(0)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecBoolean>(!args.get<ExecBoolean>(0)->getValue())
            );
        }
        if (args.get<ExecInteger>(0)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(~args.get<ExecInteger>(0)->getValue())
            );
        }

        return shared_ptr<ExecValue>();
    });

    // string manipulation
    addBuiltInFunction("_length", 1, [](const BuiltInArguments& args) {
        if (args.get<ExecString>(0)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecString>(0)->getValue().length())
            );
        }

        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_ch", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecString>(0) && args.get<ExecInteger>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecChar>(args.get<ExecString>(0)->getValue()[args.get<ExecInteger>(1)->getValue()])
            );
        }

        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_con", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecString>(0) && args.get<ExecString>(1)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecString>(args.get<ExecString>(0)->getValue() + args.get<ExecString>(1)->getValue())
            );
        }

        return shared_ptr<ExecValue>();
    });

    // conversion and utilities
    addBuiltInFunction("_integer", 1, [](const BuiltInArguments& args) {
        if (args.get<ExecBoolean>(0)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecBoolean>(0)->getValue())
            );
        }
        if (args.get<ExecInteger>(0)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecInteger>(0)->getValue())
            );
        }
        if (args.get<ExecFloat>(0)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecFloat>(0)->getValue())
            );
        }
        if (args.get<ExecChar>(0)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecChar>(0)->getValue())
            );
        }

        return shared_ptr<ExecValue>();
    });
    addBuiltInFunction("_float", 1, [](const BuiltInArguments& args) {
        if (args.get<ExecInteger>(0)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecInteger>(0)->getValue())
            );
        }
        if (args.get<ExecFloat>(0)) {
            return static_pointer_cast<ExecValue>(
                    make_shared<ExecInteger>(args.get<ExecFloat>(0)->getValue())
            );
        }

        return shared_ptr<ExecValue>();
    });

    addBuiltInFunction("_sleep", 2, [](const BuiltInArguments& args) {
        if (args.get<ExecInteger>(0)) {
            // doing busy wait
            long long int time = args.get<ExecInteger>(0)->getValue();

            // doing busy wait to better simulate processing!
            auto start = chrono::high_resolution_clock::now();
            while (true) {
                auto curr = chrono::high_resolution_clock::now();
                chrono::duration<double, milli> elapsed = curr - start;
                if (elapsed.count() >= time) break;
            }

            // returning passed value
            return args.get<ExecValue>(1);
        }

        return shared_ptr<ExecValue>();
    });
}
