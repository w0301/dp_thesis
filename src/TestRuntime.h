#ifndef TEST_RUNTIME_H
#define TEST_RUNTIME_H

#include <vector>
#include <memory>

#include "Scheduler.h"

class TestMessage {
public:
    TestMessage(int varsCount, double, double);

    const std::vector<bool> &getReadVars() const {
        return readVars;
    }

    const std::vector<bool> &getWriteVars() const {
        return writeVars;
    }

    int getProcessTime() const {
        return processTime;
    }

private:
    int processTime;
    std::vector<bool> readVars;
    std::vector<bool> writeVars;
};

class TestScheduler : public Scheduler {
public:
    TestScheduler(Scheduler::Type, int, int);

protected:
    void workerProcess(int i, std::shared_ptr<void> ptr, std::shared_ptr<void> shared_ptr) override;

    std::shared_ptr<void> acquireState(std::shared_ptr<void> g) override;
    std::shared_ptr<void> mergeStates(std::shared_ptr<void>, std::shared_ptr<void>, const std::vector<bool> &) override;
    std::pair<std::vector<bool>, std::vector<bool> > getMessageVars(std::shared_ptr<void>) override;
};

class TestRuntime {
public:
    void runTests();

    void prepare(int, int, double, double);
    void run(Scheduler::Type, int);

private:
    void runPreparedTests();

    int varsCount;
    double readLambda;
    double writeLambda;
    int totalProcessingTime;
    std::vector<std::shared_ptr<TestMessage> > messages;
};

#endif
