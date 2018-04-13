#ifndef SCHEDULER_RUNTIME_H
#define SCHEDULER_RUNTIME_H

#include <memory>

#include "Runtime.h"
#include "Scheduler.h"

class SchedulerRuntime;

class ProgramScheduler : public Scheduler {
public:
    ProgramScheduler(Scheduler::Type, int, SchedulerRuntime&);

protected:
    void workerProcess(int i, std::shared_ptr<void> ptr, std::shared_ptr<void> shared_ptr) override;
    void updateReadonlyState(std::shared_ptr<void>, const std::vector<bool> &) override;

    std::pair<std::vector<bool>, std::vector<bool> > getMessageVars(std::shared_ptr<void>) override;

private:
    SchedulerRuntime& runtime;
};

class SchedulerRuntime : public Runtime {
public:
    explicit SchedulerRuntime(const char *, Scheduler::Type, int);

    int getVarsCount() const {
        return varsCount;
    }

private:
    int varsCount;
    std::shared_ptr<ProgramScheduler> scheduler;
};


#endif
