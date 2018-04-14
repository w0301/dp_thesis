#ifndef TEST_RUNTIME_H
#define TEST_RUNTIME_H

#include <vector>
#include <memory>

#include "Scheduler.h"

class TestMessage {
public:
    TestMessage(int varsCount, double);

    const std::vector<bool> &getReadVars() const {
        return readVars;
    }

    const std::vector<bool> &getWriteVars() const {
        return writeVars;
    }

    int getProcessTime() const {
        return processTime;
    }

    int getReadVarsCount() const {
        return readVarsCount;
    }

    int getWriteVarsCount() const {
        return writeVarsCount;
    }

    static std::vector<std::shared_ptr<TestMessage> > generateMessages(int, int, double);

private:
    int processTime;
    int readVarsCount;
    int writeVarsCount;
    std::vector<bool> readVars;
    std::vector<bool> writeVars;
};

class TestRuntime : public Scheduler {
public:
    TestRuntime(Scheduler::Type, int, int, std::vector<std::shared_ptr<TestMessage> >);

    double run(double);

protected:
    void workerProcess(int, std::shared_ptr<void>) override;
    void updateReadonlyState(const std::vector<bool> &) override;
    std::pair<std::vector<bool>, std::vector<bool> > getMessageVars(std::shared_ptr<void>) override;

private:
    std::vector<std::shared_ptr<TestMessage> > messages;
};

#endif
