#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <deque>
#include <thread>
#include <vector>
#include <memory>
#include <utility>
#include <functional>

#include "Worker.h"

enum SchedulerType {
    RWLocking, WLocking
};

class BaseScheduler {
    public:
        BaseScheduler(std::shared_ptr<void>, SchedulerType, int, int);
        ~BaseScheduler();

        void start();
        void stop();

        bool isRunning() const {
            return running;
        };

        void schedule(std::shared_ptr<void>);

    protected:
        virtual std::shared_ptr<void> acquireState(std::shared_ptr<void> g) = 0;
        virtual std::shared_ptr<void> mergeStates(std::shared_ptr<void>, std::shared_ptr<void>,
                const std::vector<bool>&) = 0;

        virtual void processWorker(std::shared_ptr<void>, std::shared_ptr<void>) = 0;
        virtual std::pair< std::vector<bool>, std::vector<bool> > getMessageVars(std::shared_ptr<void>) = 0;

        void processMain();

        Worker* getFreeWorker();
        std::shared_ptr<void> getSchedulableMessage();

        bool isVarReadLocked(int);
        bool isVarWriteLocked(int);
        bool isSchedulable(std::shared_ptr<void>);
        bool isSchedulable(const std::vector<bool>&, const std::vector<bool>&);

        bool running;
        std::mutex mainMutex;
        std::thread mainThread;
        std::condition_variable mainCond;
        std::deque< std::shared_ptr<void> > queue;

        SchedulerType type;
        int workersCount, varsCount;
        std::vector< Worker* > workers;

        std::shared_ptr<void> state;
};

template <class TState, class TMsg> class Scheduler : public BaseScheduler {
    public:
        Scheduler(std::shared_ptr<TState> s, SchedulerType type, int workersCount, int varsCount) :
            BaseScheduler(std::static_pointer_cast<TState, void>(s), type, workersCount, varsCount) { };
};

#endif
