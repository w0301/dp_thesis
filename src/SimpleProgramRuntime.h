#ifndef RUNTIME_H
#define RUNTIME_H

#include <string>
#include <memory>

#include "ProgramExecutor.h"

class SimpleProgramRuntime : public ProgramExecutor {
public:
    explicit SimpleProgramRuntime(std::string);

protected:
    std::shared_ptr<ExecObject> getReadGlobal() const override {
        return global;
    }

    std::shared_ptr<ExecObject> getWriteGlobal() const override {
        return global;
    }

private:
    std::shared_ptr<ExecObject> global;
};

#endif
