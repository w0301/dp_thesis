#ifndef PROGRAM_ANALYZER_H
#define PROGRAM_ANALYZER_H

#include "Program.h"

class ProgramAnalyzer {
public:
    explicit ProgramAnalyzer(std::shared_ptr<Program> program) : program(std::move(program)) { };

    void analyze();

private:
    std::shared_ptr<Program> program;
};


#endif
