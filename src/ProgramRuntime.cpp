#include <iostream>

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
        std::chrono::duration<double, std::milli> elapsedSinceLast = (currTime - lastResultTime);
        int roundsToFinish = (int)(elapsedSinceLast.count() / 1000.0);
        runtime.finishStatRounds(roundsToFinish);

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
        resultWorker(*this), variables(getProgram()->getFunction("main")->getAllVariables()),
        readonlyGlobal(make_shared<ExecObject>()) {
    for (auto& var : variables) {
        getReadGlobal()->ensureFieldPath(var, true);
        getWriteGlobal()->ensureFieldPath(var, true);
    }
}

void ProgramRuntime::run(int millis) {
    chrono::milliseconds duration(millis);

    auto startTime = chrono::high_resolution_clock::now();
    while (true) {
        auto currTime = chrono::high_resolution_clock::now();

        // sending any new messages
        for (auto& gen : messageGenerators) {
            if (gen.isGenerationNeeded(currTime)) schedule(gen.generate(currTime));
        }

        // waiting little bit
        this_thread::sleep_for(1ms);

        // finish on time elapsed
        std::chrono::duration<double, std::milli> elapsed = (currTime - startTime);
        if (elapsed > duration) break;
    }

    // waiting for are messages beying processed and stopping
    stop(true);


    // printing stats data
    cout << "===== Messages statistics =====" << endl;
    for (auto& gen : messageGenerators) {
        cout << gen.getName() << " - ";
        for (int c : gen.getCounters()) cout << c << " ";
        cout << endl;
    }
    cout << "===============================" << endl;
}

void ProgramRuntime::workerProcess(int index, shared_ptr<void> msg) {
    resultWorker.sendResult(
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
    // TODO : do some kind of caching here based on the msg pointer address

    // determine sets of variables according to expressions found by analyzer
    set<string> readVars;
    set<string> writeVars;
    auto mainFunction = getProgram()->getFunction("main");
    auto mainLocal = static_pointer_cast<ExecObject>(msg);
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
