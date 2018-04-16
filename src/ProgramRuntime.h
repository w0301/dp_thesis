#ifndef SCHEDULER_RUNTIME_H
#define SCHEDULER_RUNTIME_H

#include <atomic>
#include <string>
#include <memory>
#include <chrono>
#include <functional>

#include "Scheduler.h"
#include "SimpleProgramRuntime.h"

class ProgramRuntime;

class ResultWorkerMessage {
public:
    enum Type { Exit = 10, Result = 5, LazyExit = 1 };

    ResultWorkerMessage(Type type, std::shared_ptr<ExecValue> result) : type(type), result(std::move(result)) { }

    Type getType() const {
        return type;
    }

    std::shared_ptr<ExecValue> getResult() const {
        return result;
    }

    friend bool operator<(const ResultWorkerMessage& l, const ResultWorkerMessage& r) {
        return l.getType() < r.getType();
    }

private:
    Type type;
    std::shared_ptr<ExecValue> result;
};

class ResultWorker : public Worker<ResultWorkerMessage> {
public:
    explicit ResultWorker(ProgramRuntime& runtime) : runtime(runtime) { }

    void stop(bool wait) {
        send(ResultWorkerMessage(wait ? ResultWorkerMessage::LazyExit : ResultWorkerMessage::Exit, std::shared_ptr<ExecValue>()));
        join();
    }

    void sendResult(std::shared_ptr<ExecValue> result) {
        send(ResultWorkerMessage(ResultWorkerMessage::Result, std::move(result)));
    }

protected:
    bool process(ResultWorkerMessage&) override;

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> lastResultTime;
    ProgramRuntime& runtime;
};

class MessageGenerator {
public:
    MessageGenerator(const std::string &name, int interval,
                     const std::function<std::shared_ptr<ExecValue>()> &generateFunc,
                     const std::function<bool(std::shared_ptr<ExecValue>)> &isMessageResultFunc);

    const std::string& getName() const {
        return name;
    }

    const std::vector<int>& getCounters() const {
        return counters;
    }

    bool isGenerationNeeded(std::chrono::time_point<std::chrono::high_resolution_clock> time) const {
        return time >= lastGenerateTime + interval;
    }

    std::shared_ptr<ExecValue> generate(std::chrono::time_point<std::chrono::high_resolution_clock> time) {
        lastGenerateTime = time;
        return generateFunc();
    }

    void incCounterIfNeeded(std::shared_ptr<ExecValue> msg) {
        if (isMessageResultFunc(std::move(msg))) currCounter += 1;
    }

    void finishCounter() {
        counters.push_back(currCounter);
        currCounter = 0;
    }

private:
    std::string name;
    std::chrono::milliseconds interval;
    std::function<std::shared_ptr<ExecValue>()> generateFunc;
    std::function<bool(std::shared_ptr<ExecValue>)> isMessageResultFunc;

    std::chrono::time_point<std::chrono::high_resolution_clock> lastGenerateTime;

    int currCounter = 0;
    std::vector<int> counters;
};

class ProgramRuntime : public SimpleProgramRuntime, public Scheduler {
public:
    explicit ProgramRuntime(std::string, Scheduler::Type, int);

    void run(int);

    void start() override {
        resultWorker.start();
        Scheduler::start();
    }

    void stop(bool wait) override {
        Scheduler::stop(wait);
        resultWorker.stop(wait);
    }

    void incStatCounter(std::shared_ptr<ExecValue> msg) {
        for (auto& gen : messageGenerators) gen.incCounterIfNeeded(msg);
    }

    void finishStatRounds(int count = 1) {
        for (int i = 0; i < count; i++) {
            for (auto &gen : messageGenerators) gen.finishCounter();
        }
    }

protected:
    void registerMessageGenerator(const std::string &name, int interval,
                                  const std::function<std::shared_ptr<ExecValue>()> &generateFunc,
                                  const std::function<bool(std::shared_ptr<ExecValue>)> &isMessageFunc) {
        messageGenerators.emplace_back(MessageGenerator(name, interval, generateFunc, isMessageFunc));
    }

    std::shared_ptr<ExecObject> getReadGlobal() const override {
        return getType() == RWLocking ? SimpleProgramRuntime::getReadGlobal() : std::atomic_load(&readonlyGlobal);
    }

    void workerProcess(int, std::shared_ptr<void>) override;
    void updateReadonlyState(const std::vector<bool> &) override;
    std::pair<std::vector<bool>, std::vector<bool> > getMessageVars(std::shared_ptr<void>) override;

private:
    ResultWorker resultWorker;
    std::vector<MessageGenerator> messageGenerators;

    std::shared_ptr<ExecObject> readonlyGlobal;

    std::set<std::string> variables;
};


#endif
