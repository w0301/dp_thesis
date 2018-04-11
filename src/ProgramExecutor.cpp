#include "ProgramExecutor.h"

using namespace std;

// Objects
shared_ptr<ExecValue> ExecObject::getFieldByPath(const string& path) const {
    // TODO
    return std::shared_ptr<ExecValue>();
}

void ExecObject::setFieldByPath(const string& path, shared_ptr<ExecValue> val) {
    // TODO
}

// Executor
shared_ptr<ExecValue> ProgramExecutor::exec(shared_ptr<ExecObject> args) {
    return execFunction(program->getFunction("main"), std::move(args));
}

shared_ptr<ExecValue> ProgramExecutor::execFunction(shared_ptr<Function> function, shared_ptr<ExecObject> local) {
    // TODO
    return shared_ptr<ExecValue>();
}
