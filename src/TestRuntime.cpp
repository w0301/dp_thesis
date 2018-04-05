#include "TestRuntime.h"

// State


// Message


// Scheduler
TestScheduler::TestScheduler(std::shared_ptr<TestState> s, scheduler_type type, int workersCount, int varsCount)
    : scheduler(s, type, workersCount, varsCount) {
}

std::shared_ptr<TestState> TestScheduler::acquire_state(std::shared_ptr<TestState> state) {
  // TODO : base on type
  return state;
}

std::shared_ptr<TestState> TestScheduler::merge_states(std::shared_ptr<TestState> globalState, std::shared_ptr<TestState> workerState, const std::vector<bool>& writeVars) {
  // TODO : base on type
  return workerState;
}

void TestScheduler::process_worker(std::shared_ptr<TestState> s, std::shared_ptr<TestMessage> msg) {
  // TODO
}

std::pair< std::vector<bool>, std::vector<bool> > TestScheduler::get_message_vars(std::shared_ptr<TestMessage> msg) {
  return std::make_pair(msg->getReadVars(), msg->getWriteVars());
}
