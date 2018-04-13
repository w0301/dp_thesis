#ifndef RUNTIME_H
#define RUNTIME_H

#include <memory>

#include "Program.h"
#include "ProgramExecutor.h"

class Runtime {
public:
    explicit Runtime(const char*);

    std::shared_ptr<Program> getProgram() const {
        return program;
    }

    std::shared_ptr<ExecValue> exec(std::shared_ptr<ExecValue> arg) {
        return ProgramExecutor(program).exec(std::move(arg));
    }

private:
    std::shared_ptr<Program> program;
};

#endif
