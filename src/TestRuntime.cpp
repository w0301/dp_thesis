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
std::random_device randomDevice{};
std::mt19937 randomEngine{randomDevice()};

int probGenerate(vector<bool>& reads, vector<bool>& writes, normal_distribution<double> distribution) {
    int varsCount = (int)reads.size();
    if (varsCount == 0) return 0;

    // building roulette count of variables
    double randVal = distribution(randomEngine);
    int count = max(0, min(2 * varsCount, (int)round(randVal)));

    // distributing randomly
    uniform_int_distribution<> uniDistribution(0, varsCount * 2);
    for (int i = 0; i < count; i++) {
        while (true) {
            int index = uniDistribution(randomEngine);
            if (index >= varsCount) {
                index -= varsCount;
                if (!writes[index]) {
                    writes[index] = true;
                    break;
                }
            }
            else {
                if (!reads[index]) {
                    reads[index] = true;
                    break;
                }
            }
        }
    }

    return count;
}

TestMessage::TestMessage(int varsCount, double param) : readVars(varsCount, false), writeVars(varsCount, false) {
    probGenerate(readVars, writeVars, normal_distribution<double>(varsCount / 2.0, sqrt(varsCount) / param));

    readVarsCount = 0;
    for (bool val : readVars) readVarsCount += val;

    writeVarsCount = 0;
    for (bool val : writeVars) writeVarsCount += val;

    // set process time based on message impact
    processTime = max(readVarsCount, 1) * 2 + max(writeVarsCount, 1) * 4;
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
    int count = 10000;
    double param = 1.0;

    prepare(count, 0, param);
    runPreparedTests();

    prepare(count, 5, param);
    runPreparedTests();

    prepare(count, 10, param);
    runPreparedTests();

    prepare(count, 20, param);
    runPreparedTests();
}

void TestRuntime::runPreparedTests() {
    double ref = run(Scheduler::RWLocking, 1, -1);

    run(Scheduler::RWLocking, 2, ref);
    run(Scheduler::RWLocking, 4, ref);

    run(Scheduler::WLocking, 2, ref);
    run(Scheduler::WLocking, 4, ref);
}

void TestRuntime::prepare(int msgsCount, int newVarsCount, double param) {
    messages.clear();

    totalProcessingTime = 0;
    varsCount = newVarsCount;
    generationParam = param;

    int totalReadsCount = 0;
    int totalWritesCount = 0;
    for (int i = 0; i < msgsCount; i++) {
        auto msg = make_shared<TestMessage>(varsCount, param);

        totalReadsCount += msg->getReadVarsCount();
        totalWritesCount += msg->getWriteVarsCount();
        totalProcessingTime += msg->getProcessTime();

        messages.push_back(msg);

    }

    cout << "-----------------------------------" << endl;
    cout << "Generated " << msgsCount << " messages." << endl;
    cout << "Avg. reads count " << totalReadsCount / (double)msgsCount << " per message." << endl;
    cout << "Avg. writes count " << totalWritesCount / (double)msgsCount << " per message." << endl;
    cout << "-----------------------------------" << endl << endl;
}


double TestRuntime::run(Scheduler::Type type, int workers, double refTime) {
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
    lineStream << messages.size() << "*(" << varsCount << ", " << generationParam << ") ";
    lineStream << "on " << (type == Scheduler::RWLocking ? "RW" : "W") << "x";
    lineStream << workers;
    lineStream << " =========";

    cout << lineStream.str() << endl;
    cout << "Computation took: " << elapsed.count() << " milliseconds" << endl;
    cout << "Total computation cost: " << totalProcessingTime << " milliseconds" << endl;

    double gain = (refTime - elapsed.count()) / refTime;
    cout << "Performance gain: " << (refTime <= 0 ? 0.0 : gain * 100.0) << "%" << endl;

    cout << string(lineStream.str().size(), '=') << endl << endl;

    return elapsed.count();
}
