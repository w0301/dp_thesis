#ifndef GUI_RUNTIME_H
#define GUI_RUNTIME_H

#include "ProgramRuntime.h"

class GuiRuntime : public ProgramRuntime {
public:
    explicit GuiRuntime(const std::string& filePath, Scheduler::Type type, int workers);

protected:
    std::shared_ptr<ExecObject> createInitMessage() const override;
};


#endif
