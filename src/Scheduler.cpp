#include <stdexcept>

#include "Scheduler.h"

using namespace std;

// SchedulerWorker
bool SchedulerWorker::process(SchedulerWorkerMessage& msg) {
    if (msg.getType() == SchedulerWorkerMessage::Process) {
        scheduler.workerProcess(index, msg.getState(), msg.getMessage());
        scheduler.workerRelease(index, msg.getState());
        return true;
    }

    return false;
}

void SchedulerWorker::clearVars() {
    readVars.assign(varsCount, false);
    writeVars.assign(varsCount, false);
}

void SchedulerWorker::setVars(const std::vector<bool>& newReadVars, const std::vector<bool>& newWriteVars) {
    readVars.assign(newReadVars.begin(), newReadVars.end());
    writeVars.assign(newWriteVars.begin(), newWriteVars.end());
}

// Scheduler
Scheduler::Scheduler(std::shared_ptr<void> state, Type type, int workersCount, int varsCount) :
        currState(state), type(type), varsCount(varsCount) {
    for (int i = 0; i < workersCount; i++) {
        workers.push_back(new SchedulerWorker(*this, i, varsCount));
    }
}

Scheduler::~Scheduler() {
    for (auto worker : workers) delete worker;
}

void Scheduler::start() {
    Worker::start();
    for (auto worker : workers) worker->start();
}

void Scheduler::stop(bool wait) {
    send(SchedulerMessage(wait ? SchedulerMessage::LazyExit : SchedulerMessage::Exit, -1, shared_ptr<void>(), shared_ptr<void>()));
    join();

    for (auto worker : workers) worker->stop(wait);
}

bool Scheduler::process(SchedulerMessage& msg) {
    if (msg.getType() == SchedulerMessage::Process || msg.getType() == SchedulerMessage::Reprocess) {
        auto worker = getAvailableWorker();

        if (worker == NULL) {
            // reschedule message again
            reschedule(msg.getMessage());

            // wait for release or exit message here - because no worker are available so no need to try scheduling
            waitFor([&](const SchedulerMessage& m) {
                return m.getType() == SchedulerMessage::Release ||
                       m.getType() == SchedulerMessage::Exit ||
                       m.getType() == SchedulerMessage::LazyExit;
            });

            return true;
        }

        auto vars = getMessageVars(msg.getMessage());
        if (isSchedulable(vars.first, vars.second)) {
            worker->setAvailable(false);
            worker->setVars(vars.first, vars.second);
            worker->schedule(acquireState(currState), msg.getMessage());
        }
        else {
            // reschedule not-processed message
            reschedule(msg.getMessage());
        }

        return true;
    }
    else if (msg.getType() == SchedulerMessage::Release) {
        auto worker = workers[msg.getSenderIndex()];

        // merging state
        currState = mergeStates(currState, msg.getState(), worker->getWriteVars());

        // resetting worker
        worker->clearVars();
        worker->setAvailable(true);

        return true;
    }

    return false;
}

SchedulerWorker* Scheduler::getAvailableWorker() {
    for (auto worker : workers) {
        if (worker->isAvailable()) return worker;
    }
    return NULL;
}

bool Scheduler::isSchedulable(const std::vector<bool>& readVars, const std::vector<bool>& writeVars) {
    for (int i = 0; i < varsCount; i++) {
        bool locked = false;
        bool needLocked = false;

        if (type == RWLocking) {
            for (auto worker : workers) {
                if (worker->getWriteVars()[i]) {
                    locked = true;
                    break;
                }
            }
            needLocked = readVars[i] || writeVars[i];
        }
        else if (type == WLocking) {
            for (auto worker : workers) {
                if (worker->getWriteVars()[i]) {
                    locked = true;
                    break;
                }
            }
            needLocked = writeVars[i];
        }

        if (locked && needLocked) return false;
    }

    return true;
}
