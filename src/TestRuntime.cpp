#include "TestRuntime.h"

using namespace std;

// State


// Message


// Scheduler
TestScheduler::TestScheduler(std::shared_ptr<TestState> s, SchedulerType type, int workersCount, int varsCount)
        : Scheduler(s, type, workersCount, varsCount) {
}

std::shared_ptr<void> TestScheduler::acquireState(std::shared_ptr<void> state) {
    // TODO : base on type
    return state;
}

std::shared_ptr<void> TestScheduler::mergeStates(std::shared_ptr<void> globalState, std::shared_ptr<void> workerState, const std::vector<bool>& writeVars) {
    // TODO : base on type
    return workerState;
}

void TestScheduler::processWorker(std::shared_ptr<void> s, std::shared_ptr<void> msg) {
    // TODO
}

std::pair< std::vector<bool>, std::vector<bool> > TestScheduler::getMessageVars(std::shared_ptr<void> msg) {
    return std::make_pair(
            static_pointer_cast<TestMessage>(msg)->getReadVars(),
            static_pointer_cast<TestMessage>(msg)->getWriteVars()
    );
}
