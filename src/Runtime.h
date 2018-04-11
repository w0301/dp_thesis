#ifndef RUNTIME_H
#define RUNTIME_H

#include <memory>

#include "Program.h"

class Runtime {
public:
    Runtime(const char*);

private:
    std::shared_ptr<Program> program;
};

#endif
