#include <iostream>

#include "antlr4-runtime.h"

#include "LangLexer.h"
#include "LangParser.h"
#include "LangParserBaseVisitor.h"

#include "Runtime.h"

using namespace std;
using namespace antlr;
using namespace antlr4;


// helper class for creating AST
class ParserVisitor : public LangParserBaseVisitor {
public:
    antlrcpp::Any visitFunction(LangParser::FunctionContext *ctx) override {
        cout << "Function definition: " << ctx->functionName()->getText() << endl;
        return LangParserBaseVisitor::visitFunction(ctx);
    }
};


// Runtime class
Runtime::Runtime(const char *file) {
    ifstream stream;
    stream.exceptions(ifstream::failbit);
    stream.open(file);

    ANTLRInputStream input(stream);

    LangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    LangParser parser(&tokens);

    // creating AST
    ParserVisitor visitor;
    auto AST = visitor.visitFile(parser.file());

    // TODO : save AST to field for further use
    
    // printing parse tree
    tokens.reset();
    tree::ParseTree* tree = parser.file();
    cout << tree->toStringTree(&parser) << endl;
}
