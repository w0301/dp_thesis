#ifndef PROGRAM_ANALYZER_H
#define PROGRAM_ANALYZER_H

#include <map>

#include "Program.h"

class ProgramAnalyzer {
public:
    explicit ProgramAnalyzer(std::shared_ptr<Program> program) : program(std::move(program)) { };

    void analyze();

private:
    bool isFunctionRecursive(std::shared_ptr<Function>, std::shared_ptr<Function>, std::set<std::shared_ptr<Function> >&);
    bool isFunctionStatementRecursive(std::shared_ptr<Function>, std::shared_ptr<Statement>, std::set<std::shared_ptr<Function> >&);

    void determineFunctionVariables(std::shared_ptr<Function>, std::set<std::string>&, std::set<std::string>&, std::set<std::shared_ptr<Function> >&);
    void determineFunctionStatementVariables(std::shared_ptr<Statement>, std::set<std::string>&, std::set<std::string>&, std::set<std::shared_ptr<Function> >&);

    void determineFunctionExpressions(std::shared_ptr<Function>, std::shared_ptr<Expression>,
                                      std::map<std::string, std::shared_ptr<Expression> >&,
                                      std::map<std::string, std::vector<std::shared_ptr<Expression> > >&,
                                      std::map<std::string, std::vector<std::shared_ptr<Expression> > >&);
    void determineFunctionStatementExpressions(std::shared_ptr<Statement>, std::shared_ptr<Expression>,
                                               std::map<std::string, std::shared_ptr<Expression> >&,
                                               std::map<std::string, std::vector<std::shared_ptr<Expression> > >&,
                                               std::map<std::string, std::vector<std::shared_ptr<Expression> > >&);

    std::shared_ptr<Program> program;
};


#endif
