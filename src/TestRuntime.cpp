#include <cmath>
#include <memory>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include "TestRuntime.h"

using namespace std;



// Message
TestMessage::TestMessage(int varsCount) {
    int readCount = 0;
    int writeCount = 0;

    for (int i = 0; i < varsCount; i++) {
        readVars.push_back((bool)(rand() % 2));
        if (readVars[i]) readCount += 1;

        writeVars.push_back((bool)(rand() % 2));
        if (writeVars[i]) writeCount += 1;
    }

    processTime = readCount * 2 + writeCount * 3;
}

// Scheduler
TestScheduler::TestScheduler(Scheduler::Type type, int workersCount, int varsCount)
        : Scheduler(shared_ptr<void>(), type, workersCount, varsCount) {
}

void TestScheduler::workerProcess(int index, shared_ptr<void> state, shared_ptr<void> msg) {
    int time = static_pointer_cast<TestMessage>(msg)->getProcessTime();
    cout << "Processing for " << time << " milliseconds on " << index << endl;

    // doing busy wait to better simulate processing!
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        auto curr = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = curr - start;
        if (elapsed.count() >= time) break;
    }
}

std::shared_ptr<void> TestScheduler::acquireState(std::shared_ptr<void> state) {
    return state;
}

std::shared_ptr<void> TestScheduler::mergeStates(std::shared_ptr<void>, std::shared_ptr<void> workerState, const std::vector<bool>&) {
    return workerState;
}

std::pair< std::vector<bool>, std::vector<bool> > TestScheduler::getMessageVars(std::shared_ptr<void> msg) {
    return std::make_pair(
            static_pointer_cast<TestMessage>(msg)->getReadVars(),
            static_pointer_cast<TestMessage>(msg)->getWriteVars()
    );
}

// Runtime
void TestRuntime::run() {
    int workers = 2, vars = 10;

    TestScheduler scheduler(Scheduler::WLocking, workers, vars);

    auto start = std::chrono::high_resolution_clock::now();

    scheduler.start();

    int totalProcessTime = 0;
    for (int i = 0; i < 500; i++) {
        auto msg = make_shared<TestMessage>(vars);
        totalProcessTime += msg->getProcessTime();

        scheduler.schedule(msg);
    }

    scheduler.stop(true);

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;
    cout << "Computation took: " << elapsed.count() << " milliseconds" << endl;
    cout << "Expected computation time: " << totalProcessTime << " milliseconds" << endl;
}
