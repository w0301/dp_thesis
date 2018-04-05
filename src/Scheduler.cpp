#include <stdexcept>

#include "Scheduler.h"

using namespace std;

BaseScheduler::BaseScheduler(std::shared_ptr<void> state, SchedulerType type, int workers_count, int vars_count) :
        running(false), type(type), workersCount(workers_count), varsCount(vars_count),
        workers(workers_count, new Worker(vars_count, mainMutex, mainCond)), state(state) {
}

BaseScheduler::~BaseScheduler() {
    for (Worker *w : workers) delete w;
    workers.clear();
}

void BaseScheduler::start() {
    running = true;
    mainThread = thread(&BaseScheduler::processMain, ref(*this));
    for (Worker *w : workers)
        w->start(std::bind(&BaseScheduler::processWorker, this, std::placeholders::_1, std::placeholders::_2));
}

void BaseScheduler::stop() {
    for (Worker *w : workers) w->stop();

    {
        std::lock_guard<std::mutex> lock(mainMutex);
        running = false;
    }
    mainCond.notify_one();

    mainThread.join();
}

void BaseScheduler::schedule(std::shared_ptr<void> msg) {
    {
        std::lock_guard<std::mutex> lock(mainMutex);
        queue.push_back(msg);
    }
    mainCond.notify_one();
}

void BaseScheduler::processMain() {
    while (running) {
        Worker *freeWorker = NULL;
        std::shared_ptr<void> schedulableMsg;

        std::unique_lock<std::mutex> lock(mainMutex);
        mainCond.wait(lock, [&] {
            if (!running) return true;

            freeWorker = getFreeWorker();
            schedulableMsg = getSchedulableMessage();

            return freeWorker || schedulableMsg;
        });
        if (!running) break;

        // consume Worker results
        if (freeWorker && !freeWorker->isProcessConsumed()) {
            auto workerState = freeWorker->consumeProcess();
            state = mergeStates(state, workerState, freeWorker->getWriteVars());

            freeWorker->clearVars();
        }

        // schedule message
        if (schedulableMsg) {
            // locking variables
            auto vars = getMessageVars(schedulableMsg);
            freeWorker->setVars(vars.first, vars.second);

            // acquiring state
            auto workerState = acquireState(state);

            // and processing messsage after that
            freeWorker->process(workerState, schedulableMsg);

            // delete msg
            for (auto it = queue.begin(); it != queue.end(); it++) {
                if (schedulableMsg == *it) {
                    queue.erase(it);
                    break;
                }
            }
        }
    }
}

Worker *BaseScheduler::getFreeWorker() {
    for (auto worker : workers) {
        if (!worker->isProcessing() && !worker->isProcessConsumed()) return worker;
    }
    for (auto worker : workers) {
        if (!worker->isProcessing()) return worker;
    }
    return NULL;
}

std::shared_ptr<void> BaseScheduler::getSchedulableMessage() {
    for (auto msg : queue) {
        if (isSchedulable(msg)) return msg;
    }
    return std::shared_ptr<void>();
}

bool BaseScheduler::isVarReadLocked(int var_index) {
    for (int i = 0; i < workersCount; i++) {
        if (workers[i]->isReading(var_index)) return true;
    }
    return false;
}

bool BaseScheduler::isVarWriteLocked(int var_index) {
    for (int i = 0; i < workersCount; i++) {
        if (workers[i]->isWriting(var_index)) return true;
    }
    return false;
}

bool BaseScheduler::isSchedulable(std::shared_ptr<void> msg) {
    auto vars = getMessageVars(msg);
    return isSchedulable(vars.first, vars.second);
}

bool BaseScheduler::isSchedulable(const std::vector<bool> &read_vars, const std::vector<bool> &write_vars) {
    if (read_vars.size() != varsCount || write_vars.size() != varsCount) {
        throw runtime_error("Incorrect number of variables in the input vectors.");
    }

    for (int i = 0; i < varsCount; i++) {
        bool locked = false;
        bool need_locked = false;

        if (type == SchedulerType::RWLocking) {
            locked = isVarReadLocked(i) || isVarWriteLocked(i);
            need_locked = read_vars[i] || write_vars[i];
        } else if (type == SchedulerType::WLocking) {
            locked = isVarWriteLocked(i);
            need_locked = write_vars[i];
        }

        if (locked && need_locked) return false;
    }

    return true;
}
