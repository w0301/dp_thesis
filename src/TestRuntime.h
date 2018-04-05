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

class TestScheduler : public Scheduler<TestState, TestMessage> {
    public:
        TestScheduler(std::shared_ptr<TestState>, SchedulerType, int, int);

    protected:
        std::shared_ptr<void> acquireState(std::shared_ptr<void> g) override;
        std::shared_ptr<void> mergeStates(std::shared_ptr<void>, std::shared_ptr<void>,
                                              const std::vector<bool>&) override;

        void processWorker(std::shared_ptr<void>, std::shared_ptr<void>) override;
        std::pair< std::vector<bool>, std::vector<bool> > getMessageVars(std::shared_ptr<void>) override;
};

class TestRuntime {

};

#endif
