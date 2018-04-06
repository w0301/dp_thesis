#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <deque>
#include <thread>
#include <vector>
#include <memory>
#include <utility>
#include <functional>

#include "Worker.h"

class Scheduler;

class SchedulerWorkerMessage {
public:
    enum Type {
        Exit = 100, Process = 10, LazyExit = 1
    };

    SchedulerWorkerMessage(Type type, std::shared_ptr<void> state, std::shared_ptr<void> message) :
            type(type), state(std::move(state)), message(std::move(message)) { };

    Type getType() const {
        return type;
    }

    std::shared_ptr<void> getState() const {
        return state;
    }

    std::shared_ptr<void> getMessage() const {
        return message;
    }

    friend bool operator<(const SchedulerWorkerMessage& l, const SchedulerWorkerMessage& r) {
        return l.getType() < r.getType();
    }

private:
    Type type;
    std::shared_ptr<void> state;
    std::shared_ptr<void> message;
};

class SchedulerWorker : public Worker<SchedulerWorkerMessage> {
public:
    SchedulerWorker(Scheduler& scheduler, int index, int varsCount) :
            scheduler(scheduler), available(true), index(index), varsCount(varsCount),
            readVars(varsCount, false), writeVars(varsCount, false) { }

    void stop(bool wait) {
        send(SchedulerWorkerMessage(wait ? SchedulerWorkerMessage::LazyExit : SchedulerWorkerMessage::Exit, std::shared_ptr<void>(), std::shared_ptr<void>()));
        join();
    }

    void schedule(std::shared_ptr<void> state, std::shared_ptr<void> message) {
        send(SchedulerWorkerMessage(SchedulerWorkerMessage::Process, std::move(state), std::move(message)));
    }

    bool isAvailable() const {
        return available;
    }

    void setAvailable(bool val) {
        available = val;
    }

    const std::vector<bool> getReadVars() const {
        return readVars;
    }

    const std::vector<bool> getWriteVars() const {
        return writeVars;
    }

    void clearVars();
    void setVars(const std::vector<bool>&, const std::vector<bool>&);

protected:
    bool process(SchedulerWorkerMessage& msg) override;

private:
    Scheduler& scheduler;

    bool available;
    int index, varsCount;
    std::vector<bool> readVars;
    std::vector<bool> writeVars;
};

class SchedulerMessage {
public:
    enum Type {
        Exit = 1000, Release = 100, Process = 10, Reprocess = 5, LazyExit = 1
    };

    SchedulerMessage(Type type, int index, std::shared_ptr<void> state, std::shared_ptr<void> message) :
            type(type), senderIndex(index), state(std::move(state)), message(std::move(message)) { };

    Type getType() const {
        return type;
    }

    int getSenderIndex() const {
        return senderIndex;
    }

    std::shared_ptr<void> getState() const {
        return state;
    }

    std::shared_ptr<void> getMessage() const {
        return message;
    }

    friend bool operator<(const SchedulerMessage& l, const SchedulerMessage& r) {
        return l.getType() < r.getType();
    }

private:
    Type type;
    int senderIndex;
    std::shared_ptr<void> state;
    std::shared_ptr<void> message;
};

class Scheduler : public Worker<SchedulerMessage> {
public:
    enum Type {
        RWLocking, WLocking
    };

    Scheduler(std::shared_ptr<void>, Type, int, int);
    ~Scheduler();

    void start() override;
    void stop(bool);

    void schedule(std::shared_ptr<void> message) {
        send(SchedulerMessage(SchedulerMessage::Process, -1, std::shared_ptr<void>(), std::move(message)));
    }

    Type getType() const {
        return type;
    }

    int getVarsCount() const {
        return varsCount;
    }

protected:
    void reschedule(std::shared_ptr<void> message) {
        send(SchedulerMessage(SchedulerMessage::Reprocess, -1, std::shared_ptr<void>(), std::move(message)));
    }

    friend class SchedulerWorker;

    bool process(SchedulerMessage& msg) override;

    void workerRelease(int index, std::shared_ptr<void> state) {
        send(SchedulerMessage(SchedulerMessage::Release, index, std::move(state), std::shared_ptr<void>()));
    }

    virtual void workerProcess(int, std::shared_ptr<void>, std::shared_ptr<void>) = 0;

    virtual std::shared_ptr<void> acquireState(std::shared_ptr<void> g) = 0;
    virtual std::shared_ptr<void> mergeStates(std::shared_ptr<void>, std::shared_ptr<void>, const std::vector<bool> &) = 0;

    virtual std::pair<std::vector<bool>, std::vector<bool> > getMessageVars(std::shared_ptr<void>) = 0;

private:
    SchedulerWorker* getAvailableWorker();
    bool isSchedulable(const std::vector<bool>&, const std::vector<bool>&);

    Type type;
    int varsCount;
    std::shared_ptr<void> currState;
    std::vector<SchedulerWorker*> workers;
};

#endif
