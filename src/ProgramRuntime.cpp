#include "ProgramRuntime.h"

using namespace std;

ProgramRuntime::ProgramRuntime(string filePath, Scheduler::Type type, int workers) :
        SimpleProgramRuntime(filePath), Scheduler(type, workers, 0), readonlyGlobal(make_shared<ExecObject>()) {
    // TODO : run analyzer here for the program

}

void ProgramRuntime::workerProcess(int index, shared_ptr<void> msg) {
    exec(static_pointer_cast<ExecValue>(msg));
}

void ProgramRuntime::updateReadonlyState(const std::vector<bool> &writes) {
    if (getType() == WLocking) {
        // TODO : reading only written vars, others are dangerous to read because are not locked!!!
        atomic_store(&readonlyGlobal, static_pointer_cast<ExecObject>(getWriteGlobal()->clone()));
    }
}

std::pair< std::vector<bool>, std::vector<bool> > ProgramRuntime::getMessageVars(std::shared_ptr<void> msg) {

}
