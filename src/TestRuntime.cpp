#include <cmath>
#include <memory>
#include <thread>
#include <chrono>
#include <random>
#include <string>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <functional>

#include "TestRuntime.h"

using namespace std;

// Message
int probGenerate(vector<bool>& res, const function<double(int)>& probFunc) {
    int varsCount = (int)res.size();
    if (varsCount == 0) return 0;

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

TestMessage::TestMessage(int varsCount, double readLambda, double writeLambda) :
        readVars(varsCount, false), writeVars(varsCount, false) {
    int readsCount = probGenerate(readVars, [&](int c) {
        return exp(-1.0 * readLambda * ((double)c / (double)varsCount));
    });

    int writesCount = probGenerate(writeVars, [&](int c) {
        return exp(-1.0 * writeLambda * ((double)c / (double)varsCount));
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

void TestScheduler::updateReadonlyState(std::shared_ptr<void>, const std::vector<bool> &) {
    // nothing is needed here, because we have no state
}

std::pair< std::vector<bool>, std::vector<bool> > TestScheduler::getMessageVars(std::shared_ptr<void> msg) {
    return std::make_pair(
            static_pointer_cast<TestMessage>(msg)->getReadVars(),
            static_pointer_cast<TestMessage>(msg)->getWriteVars()
    );
}

// Runtime
void TestRuntime::runTests() {
    prepare(1000, 0, 2.0, 4.0);
    runPreparedTests();

    prepare(1000, 5, 2.0, 4.0);
    runPreparedTests();

    prepare(1000, 10, 2.0, 4.0);
    runPreparedTests();

    prepare(1000, 20, 2.0, 4.0);
    runPreparedTests();
}

void TestRuntime::runPreparedTests() {
    run(Scheduler::RWLocking, 1);

    run(Scheduler::RWLocking, 2);
    run(Scheduler::RWLocking, 4);

    run(Scheduler::WLocking, 2);
    run(Scheduler::WLocking, 4);
}

void TestRuntime::prepare(int msgsCount, int newVarsCount, double newReadLambda, double newWriteLambda) {
    messages.clear();

    totalProcessingTime = 0;
    varsCount = newVarsCount;
    readLambda = newReadLambda;
    writeLambda = newWriteLambda;

    for (int i = 0; i < msgsCount; i++) {
        auto msg = make_shared<TestMessage>(varsCount, readLambda, writeLambda);
        totalProcessingTime += msg->getProcessTime();
        messages.push_back(msg);
    }
}


void TestRuntime::run(Scheduler::Type type, int workers) {
    TestScheduler scheduler(type, workers, varsCount);

    auto start = std::chrono::high_resolution_clock::now();
    {
        scheduler.start();
        for (auto msg : messages) {
            scheduler.schedule(msg);

            // we limit speed of messages - better mimic real world
            this_thread::sleep_for(1ms);
        }
        scheduler.stop(true);
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;

    stringstream lineStream;

    lineStream << "========= ";
    lineStream << messages.size() << "*(" << varsCount << ", " << readLambda << ", " << writeLambda << ") ";
    lineStream << "on " << (type == Scheduler::RWLocking ? "RW" : "W") << "x";
    lineStream << workers;
    lineStream << " =========";

    cout << lineStream.str() << endl;
    cout << "Computation took: " << elapsed.count() << " milliseconds" << endl;
    cout << "Total computation cost: " << totalProcessingTime << " milliseconds" << endl;
    cout << string(lineStream.str().size(), '=') << endl << endl;
}
