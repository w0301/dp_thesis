#include <memory>

#include "SchedulerRuntime.h"

using namespace std;

// Scheduler
ProgramScheduler::ProgramScheduler(Scheduler::Type type, int workersCount, SchedulerRuntime& runtime)
        : Scheduler(shared_ptr<ExecObject>(), type, workersCount, runtime.getVarsCount()), runtime(runtime) {
}

void ProgramScheduler::workerProcess(int index, shared_ptr<void> state, shared_ptr<void> msg) {
    
}

void ProgramScheduler::updateReadonlyState(std::shared_ptr<void>, const std::vector<bool> &) {

}

std::pair< std::vector<bool>, std::vector<bool> > ProgramScheduler::getMessageVars(std::shared_ptr<void> msg) {

}

// Runtime
SchedulerRuntime::SchedulerRuntime(const char *path, Scheduler::Type type, int workers) : Runtime(path) {
    // TODO : run analyzer here and populate varsCount

    scheduler = make_shared<ProgramScheduler>(type, workers, *this);
}
