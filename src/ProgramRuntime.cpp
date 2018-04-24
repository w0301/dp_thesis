#include <iostream>
#include <algorithm>

#include "ProgramRuntime.h"

using namespace std;

bool hasPrefixInSet(const string& var, const set<string>& prefixes) {
    for (auto& prefix : prefixes) {
        if (var.find(prefix + ".") == 0 || var == prefix) {
            return true;
        }
    }
    return false;
}

// ResultWorker
bool ResultWorker::process(ResultWorkerMessage& msg) {
    if (msg.getType() == ResultWorkerMessage::Result) {
        auto currTime = chrono::high_resolution_clock::now();

        // finishing old rounds
        std::chrono::duration<double, std::milli> elapsedSinceLast = (currTime - lastRoundTime);
        int roundsToFinish = (int)(elapsedSinceLast.count() / 1000.0);
        if (roundsToFinish > 0) {
            runtime.finishStatRounds(roundsToFinish);
            lastRoundTime = currTime;
        }

        // inc counter for message
        runtime.incStatCounter(msg.getResult());

        lastResultTime = currTime;
        return true;
    }

    return false;
}

// MessageGenerator
MessageGenerator::MessageGenerator(const string &name, int interval,
                                   const function<shared_ptr<ExecValue>()> &generateFunc,
                                   const function<bool(shared_ptr<ExecValue>)> &isMessageResultFunc) :
        name(name), interval(interval), generateFunc(generateFunc), isMessageResultFunc(isMessageResultFunc) {
}

// Runtime
ProgramRuntime::ProgramRuntime(string filePath, Scheduler::Type type, int workers) :
        SimpleProgramRuntime(move(filePath)),
        Scheduler(type, workers, (int)getProgram()->getFunction("main")->getAllVariables().size()),
        variables(getProgram()->getFunction("main")->getAllVariables()),
        readonlyGlobal(make_shared<ExecObject>()) {
    resultWorker = make_shared<ResultWorker>(*this);

    for (auto& var : variables) {
        getReadGlobal()->ensureFieldPath(var, true);
        getWriteGlobal()->ensureFieldPath(var, true);
    }
}

void ProgramRuntime::run(int millis) {
    start();

    auto initMsg = createInitMessage();
    if (initMsg) schedule(initMsg);

    chrono::milliseconds duration(millis);

    vector<MessageGenerator*> generators;
    for (auto& gen : messageGenerators) generators.push_back(&gen);

    auto startTime = chrono::high_resolution_clock::now();
    while (true) {
        auto currTime = chrono::high_resolution_clock::now();

        // sending any new messages
        random_shuffle(generators.begin(), generators.end());
        for (auto& gen : generators) {
            if (gen->isGenerationNeeded(currTime)) schedule(gen->generate(currTime));
        }

        // waiting little bit
        this_thread::sleep_for(1ms);

        // finish on time elapsed
        std::chrono::duration<double, std::milli> elapsed = (currTime - startTime);
        if (elapsed > duration) break;
    }

    // killing the execution
    stop(false);

    // finishing last statistics round
    finishStatRounds();

    // printing final global state
    cout << "======== Global state =========" << endl;
    cout << getWriteGlobal()->toString() << endl;
    cout << "===============================" << endl << endl;

    // printing stats data
    cout << "===== Messages statistics =====" << endl;
    int totalDoneMessages = 0;
    for (auto& gen : messageGenerators) {
        cout << gen.getName() << ":" << endl;

        int total = 0, min = INT32_MAX, max = 0;
        for (int c : gen.getCounters()) {
            total += c;
            if (c < min) min = c;
            if (c > max) max = c;
        }
        totalDoneMessages += total;

        cout << "  - total seconds: " << millis / 1000.0 << endl;
        cout << "  - total generated: " << gen.getTotalGenerated() << endl;
        cout << "  - total done: " << total << endl;
        cout << "  - avg. per second: " << total / (double)gen.getCounters().size() << endl;
        cout << "  - min. per second: " << min << endl;
        cout << "  - max. per second: " << max << endl;
    }
    cout << "-------------------------------" << endl;
    cout << "  - absolute total done: " << totalDoneMessages << endl;
    cout << "  - absolute avg. per second: " << totalDoneMessages / (millis / 1000.0) << endl;
    cout << "===============================" << endl << endl;
}

void ProgramRuntime::workerProcess(int index, shared_ptr<void> msg) {
    resultWorker->sendResult(
            exec(static_pointer_cast<ExecValue>(msg))
    );
}

void ProgramRuntime::updateReadonlyState(const std::vector<bool> &writes) {
    if (getType() == WLocking) {
        // reading only written vars, others are dangerous to read because are not locked!!!
        auto res = static_pointer_cast<ExecObject>(readonlyGlobal->clone());

        int i = 0;
        for (auto& var : variables) {
            if (writes[i]) res->setFieldByPath(var, getWriteGlobal()->getFieldByPath(var));
            i += 1;
        }

        atomic_store(&readonlyGlobal, res);
    }
}

std::pair< std::vector<bool>, std::vector<bool> > ProgramRuntime::getMessageVars(std::shared_ptr<void> msg) {
    if (getWorkersCount() == 1) {
        return make_pair(vector<bool>(getVarsCount(), false), vector<bool>(getVarsCount(), false));
    }

    // setting up data for executor
    auto mainLocal = make_shared<ExecObject>();
    auto mainFunction = getProgram()->getFunction("main");
    for (auto& arg : mainFunction->getArguments()) mainLocal->setFieldByPath(arg, static_pointer_cast<ExecObject>(msg));

    // determine sets of variables according to expressions found by analyzer
    set<string> readVars;
    set<string> writeVars;
    for (auto& var : variables) {
        // handling read expressions
        for (auto& exp : mainFunction->getReadExpressions()[var]) {
            if (dynamic_pointer_cast<ExecBoolean>(execExpression(exp, mainLocal))->getValue()) {
                readVars.insert(var);
                break;
            }
        }

        // handling write expressions
        for (auto& exp : mainFunction->getWriteExpressions()[var]) {
            if (dynamic_pointer_cast<ExecBoolean>(execExpression(exp, mainLocal))->getValue()) {
                writeVars.insert(var);
                break;
            }
        }
    }

    // creating final binary vectors
    auto res = make_pair(vector<bool>(variables.size(), false), vector<bool>(variables.size(), false));

    int i = 0;
    for (auto& var : variables) {
        // it is needed to fill in any dependent vars!!!
        if (hasPrefixInSet(var, readVars)) res.first[i] = true;
        if (hasPrefixInSet(var, writeVars)) res.second[i] = true;

        i += 1;
    }

    return res;
}
