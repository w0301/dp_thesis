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

vector<shared_ptr<TestMessage> > TestMessage::generateMessages(int count, int varsCount, double generationParam) {
    vector<shared_ptr<TestMessage> > messages;

    int totalProcessingTime = 0;

    int totalReadsCount = 0;
    int totalWritesCount = 0;
    for (int i = 0; i < count; i++) {
        auto msg = make_shared<TestMessage>(varsCount, generationParam);

        totalReadsCount += msg->getReadVarsCount();
        totalWritesCount += msg->getWriteVarsCount();
        totalProcessingTime += msg->getProcessTime();

        messages.push_back(msg);
    }

    cout << "-----------------------------------" << endl;
    cout << "Generated " << count << " messages." << endl;
    cout << "Avg. reads count " << totalReadsCount / (double)count << " per message." << endl;
    cout << "Avg. writes count " << totalWritesCount / (double)count << " per message." << endl;
    cout << "-----------------------------------" << endl << endl;

    return messages;
}

// SimpleProgramRuntime
TestRuntime::TestRuntime(Scheduler::Type type, int workersCount, int varsCount, vector<shared_ptr<TestMessage> > messages)
        : Scheduler(type, workersCount, varsCount), messages(move(messages)) {
}

void TestRuntime::workerProcess(int index, shared_ptr<void> msg) {
    int time = static_pointer_cast<TestMessage>(msg)->getProcessTime();

    // doing busy wait to better simulate processing!
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        auto curr = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = curr - start;
        if (elapsed.count() >= time) break;
    }
}

void TestRuntime::updateReadonlyState(const std::vector<bool> &) {
    // nothing is needed here, because we have no state
}

std::pair< std::vector<bool>, std::vector<bool> > TestRuntime::getMessageVars(std::shared_ptr<void> msg) {
    return std::make_pair(
            static_pointer_cast<TestMessage>(msg)->getReadVars(),
            static_pointer_cast<TestMessage>(msg)->getWriteVars()
    );
}

double TestRuntime::run(double refTime) {
    int expectedTime = 0;
    auto startTime = std::chrono::high_resolution_clock::now();
    {
        start();
        for (auto msg : messages) {
            expectedTime += msg->getProcessTime();

            schedule(msg);

            // we limit speed of messages - better mimic real world
            this_thread::sleep_for(1ms);
        }
        stop(true);
    }
    auto endTime = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

    stringstream lineStream;

    lineStream << "========= ";
    lineStream << messages.size() << "*(" << getVarsCount() << ") ";
    lineStream << "on " << (getType() == Scheduler::RWLocking ? "RW" : "W") << "x";
    lineStream << getWorkersCount();
    lineStream << " =========";

    cout << lineStream.str() << endl;
    cout << "Computation took: " << elapsed.count() << " milliseconds" << endl;
    cout << "Total computation cost: " << expectedTime << " milliseconds" << endl;

    double gain = (refTime - elapsed.count()) / refTime;
    cout << "Performance gain: " << (refTime <= 0 ? 0.0 : gain * 100.0) << "%" << endl;

    cout << string(lineStream.str().size(), '=') << endl << endl;

    return elapsed.count();
}
