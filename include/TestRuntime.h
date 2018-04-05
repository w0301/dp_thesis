#ifndef TEST_RUNTIME_H
#define TEST_RUNTIME_H

#include <vector>

#include "Scheduler.h"

class TestState {
  public:
    TestState(int varsCount);

  private:
    std::vector<int> readTimes;
    std::vector<int> writeTimes;
};

class TestMessage {
  public:
    TestMessage(int varsCount, int processTime);

    const std::vector<bool>& getReadVars() const {
      return readVars;
    };

    const std::vector<bool>& getWriteVars() const {
      return writeVars;
    };

  private:
    int processTime;
    std::vector<bool> readVars;
    std::vector<bool> writeVars;
};

class TestScheduler : public scheduler<TestState, TestMessage> {
  public:
    TestScheduler(std::shared_ptr<TestState>, scheduler_type, int, int);

  protected:
    std::shared_ptr<TestState> acquire_state(std::shared_ptr<TestState>) override;
    std::shared_ptr<TestState> merge_states(std::shared_ptr<TestState>, std::shared_ptr<TestState>, const std::vector<bool>&) override;
    void process_worker(std::shared_ptr<TestState>, std::shared_ptr<TestMessage>) override;
    std::pair< std::vector<bool>, std::vector<bool> > get_message_vars(std::shared_ptr<TestMessage>) override;
};

class TestRuntime {

};

#endif
