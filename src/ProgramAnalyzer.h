#ifndef PROGRAM_ANALYZER_H
#define PROGRAM_ANALYZER_H

#include "Program.h"

class ProgramAnalyzer {
public:
    explicit ProgramAnalyzer(std::shared_ptr<Program> program) : program(std::move(program)) { };

    void analyze();

private:
    bool isFunctionRecursive(std::shared_ptr<Function>, std::shared_ptr<Function>, std::set<std::shared_ptr<Function> >&);
    void determineFunctionVariables(std::shared_ptr<Function>, std::set<std::string>&, std::set<std::string>&, std::set<std::shared_ptr<Function> >&);

    std::shared_ptr<Program> program;
};


#endif
