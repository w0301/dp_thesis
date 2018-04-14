#ifndef SCHEDULER_RUNTIME_H
#define SCHEDULER_RUNTIME_H

#include <atomic>
#include <string>
#include <memory>

#include "SimpleProgramRuntime.h"
#include "Scheduler.h"

class ProgramRuntime : public SimpleProgramRuntime, public Scheduler {
public:
    explicit ProgramRuntime(std::string, Scheduler::Type, int);

protected:
    std::shared_ptr<ExecObject> getReadGlobal() const override {
        return getType() == RWLocking ? SimpleProgramRuntime::getReadGlobal() : std::atomic_load(&readonlyGlobal);
    }

    void workerProcess(int, std::shared_ptr<void>) override;
    void updateReadonlyState(const std::vector<bool> &) override;
    std::pair<std::vector<bool>, std::vector<bool> > getMessageVars(std::shared_ptr<void>) override;

private:
    std::shared_ptr<ExecObject> readonlyGlobal;
};


#endif
