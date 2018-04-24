#ifndef SERVER_RUNTIME_H
#define SERVER_RUNTIME_H

#include "ProgramRuntime.h"

class ServerRuntime : public ProgramRuntime {
public:
    explicit ServerRuntime(const std::string& filePath, Scheduler::Type type, int workers);

protected:
    std::shared_ptr<ExecObject> createInitMessage() const override;
};


#endif
