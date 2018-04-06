#include <cmath>
#include <memory>
#include <thread>
#include <chrono>
#include <random>
#include <cstdlib>
#include <iostream>
#include <functional>

#include "TestRuntime.h"

using namespace std;

// Message
int probGenerate(vector<bool>& res, const function<double(int)>& probFunc) {
    int varsCount = (int)res.size();

    // building roulette count of variables
    double rouletteSum = 0;
    vector<double> roulette;
    for (int i = 0; i <= varsCount; i++) {
        double prob = probFunc(i);
        rouletteSum += prob;
        roulette.push_back(prob);
    }

    // normalizing roulette
    for (int i = 0; i <= varsCount; i++) {
        roulette[i] = roulette[i] / rouletteSum;
    }

    // using roulette
    int count = 0;
    double sum = 0;
    double randVal = rand() / (double)RAND_MAX;
    for ( ; count <= varsCount; count++) {
        sum += roulette[count];
        if (randVal < sum) break;
    }

    // distributing randomly
    for (int i = 0; i < count; i++) {
        int index = 0;
        do {
            index = rand() % varsCount;
        }
        while (res[index]);
        res[index] = true;
    }

    return count;
}

TestMessage::TestMessage(int varsCount) :
        readVars(varsCount, false), writeVars(varsCount, false) {
    double lambda = 2;

    int readsCount = probGenerate(readVars, [&](int c) {
        return exp(-1.0 * lambda * ((double)c / (double)varsCount));
    });

    int writesCount = probGenerate(writeVars, [&](int c) {
        return exp(-1.0 * lambda * ((double)c / (double)varsCount));
    });

    // set process time based on message impact
    processTime = max(readsCount, 1) * 2 + max(writesCount, 1) * 4;
}

// Scheduler
TestScheduler::TestScheduler(Scheduler::Type type, int workersCount, int varsCount)
        : Scheduler(shared_ptr<void>(), type, workersCount, varsCount) {
}

void TestScheduler::workerProcess(int index, shared_ptr<void> state, shared_ptr<void> msg) {
    int time = static_pointer_cast<TestMessage>(msg)->getProcessTime();

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
void TestRuntime::runTests() {
    prepare(2, 1000);
    runPreparedTests();

    prepare(5, 1000);
    runPreparedTests();

    prepare(10, 1000);
    runPreparedTests();
}

void TestRuntime::runPreparedTests() {
    cout << varsCount << " vars and " << messages.size() << " messages:" << endl;
    {
        run(Scheduler::RWLocking, 1);

        run(Scheduler::RWLocking, 2);
        run(Scheduler::RWLocking, 4);

        run(Scheduler::WLocking, 2);
        run(Scheduler::WLocking, 4);
    }
    cout << endl;
}

void TestRuntime::prepare(int newVarsCount, int msgCount) {
    messages.clear();
    totalProcessingTime = 0;
    varsCount = newVarsCount;

    for (int i = 0; i < msgCount; i++) {
        auto msg = make_shared<TestMessage>(varsCount);
        totalProcessingTime += msg->getProcessTime();
        messages.push_back(msg);
    }
}


void TestRuntime::run(Scheduler::Type type, int workers) {
    TestScheduler scheduler(type, workers, varsCount);

    auto start = std::chrono::high_resolution_clock::now();
    {
        scheduler.start();
        for (auto msg : messages) scheduler.schedule(msg);
        scheduler.stop(true);
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;
    cout << "================== " << (type == Scheduler::RWLocking ? "RW " : "W  ") << workers << " ====================" << endl;
    cout << "Computation took: " << elapsed.count() << " milliseconds" << endl;
    cout << "Total computation cost: " << totalProcessingTime << " milliseconds" << endl;
    cout << "============================================" << endl;
}
