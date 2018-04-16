#include "ServerRuntime.h"

ServerRuntime::ServerRuntime(const std::string& filePath, Scheduler::Type type, int workers) :
        ProgramRuntime(filePath, type, workers) {
    // TODO : register messages
}
