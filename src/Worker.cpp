#include <stdexcept>

#include "Worker.h"

Worker::Worker(int vars_count, std::mutex &mutex, std::condition_variable &cond) :
        varsCount(vars_count), schedulerMutex(mutex), schedulerCond(cond),
        readVars(vars_count, false), writeVars(vars_count, false) {
}

void Worker::start(const std::function<void(std::shared_ptr<void>, std::shared_ptr<void>)> &func) {
    running = true;
    processing = false;
    processConsumed = false;

    workerThread = std::thread(&Worker::workerMethod, std::ref(*this), func);
}

void Worker::stop() {
    {
        std::lock_guard<std::mutex> lock(workerMutex);
        running = false;
    }
    workerCond.notify_one();

    workerThread.join();
}

void Worker::clearVars() {
    readVars.assign(varsCount, false);
    writeVars.assign(varsCount, false);
}

void Worker::setVars(const std::vector<bool> &read, const std::vector<bool> &write) {
    if (readVars.size() != varsCount || writeVars.size() != varsCount) {
        throw std::runtime_error("Incorrect number of variables in the input vectors.");
    }

    readVars.assign(read.begin(), read.end());
    writeVars.assign(write.begin(), write.end());
}

void Worker::process(std::shared_ptr<void> state, std::shared_ptr<void> msg) {
    {
        std::lock_guard<std::mutex> lock(workerMutex);

        workerState = state;
        workerMessage = msg;
    }
    workerCond.notify_one();
}

std::shared_ptr<void> Worker::consumeProcess() {
    std::lock_guard<std::mutex> lock(workerMutex);
    if (processing) return std::shared_ptr<void>();

    processConsumed = true;
    return workerState;
}

void Worker::workerMethod(const std::function<void(std::shared_ptr<void>, std::shared_ptr<void>)> &func) {
    while (running) {
        std::unique_lock<std::mutex> lock(workerMutex);
        workerCond.wait(lock, [&] { return !running || (workerState && workerMessage); });
        if (!running) break;

        {
            std::lock_guard<std::mutex> lock(schedulerMutex);

            processing = true;
            processConsumed = false;
        }

        func(workerState, workerMessage);

        {
            std::lock_guard<std::mutex> lock(schedulerMutex);

            processing = false;
        }
        schedulerCond.notify_one();
    }
}
