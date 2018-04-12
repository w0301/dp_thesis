#include <memory>
#include <locale>
#include <codecvt>
#include <iostream>

#include "antlr4-runtime.h"

#include "LangLexer.h"
#include "LangParser.h"
#include "LangParserBaseVisitor.h"

#include "Runtime.h"
#include "Program.h"

using namespace std;
using namespace antlr;
using namespace antlr4;

// helper class for creating AST
class ParserVisitor : public LangParserBaseVisitor {
public:
    antlrcpp::Any visitFile(LangParser::FileContext *ctx) override {
        auto program = make_shared<Program>();
        for (auto func : ctx->function()) program->addFunction(visitFunction(func));
        return program;
    }

    antlrcpp::Any visitFunction(LangParser::FunctionContext *ctx) override {
        auto function = make_shared<Function>(ctx->functionName()->getText());
        for (auto arg : ctx->functionArgName()) function->addArgument(arg->getText());
        for (auto stat : ctx->statements()->statement()) function->addStatement(visitStatement(stat));
        return function;
    }

    antlrcpp::Any visitStatement(LangParser::StatementContext *ctx) override {
        if (ctx->assignmentStatement()) {
            if (ctx->assignmentStatement()->functionCall()) {
                auto assignment = make_shared<CallAssignment>();
                assignment->setTarget(visitIdentifier(ctx->assignmentStatement()->identifier()));
                assignment->setFunctionName(ctx->assignmentStatement()->functionCall()->functionName()->getText());
                for (auto arg : ctx->assignmentStatement()->functionCall()->functionArg()) {
                    assignment->addFunctionArg(visitValue(arg->value()));
                }
                return dynamic_pointer_cast<Statement>(assignment);
            }
            else if (ctx->assignmentStatement()->value()->constantValue()) {
                auto assignment = make_shared<ConstantAssignment>();
                assignment->setTarget(visitIdentifier(ctx->assignmentStatement()->identifier()));
                assignment->setValue(visitConstantValue(ctx->assignmentStatement()->value()->constantValue()));
                return dynamic_pointer_cast<Statement>(assignment);
            }
            else if (ctx->assignmentStatement()->value()->identifier()) {
                auto assignment = make_shared<IdentifierAssignment>();
                assignment->setTarget(visitIdentifier(ctx->assignmentStatement()->identifier()));
                assignment->setValue(dynamic_pointer_cast<IdentifierValue, Value>(visitValue(ctx->assignmentStatement()->value())));
                return dynamic_pointer_cast<Statement>(assignment);
            }
        }
        else if (ctx->returnStatement()) {
            auto ret = make_shared<Return>();
            ret->setValue(visitValue(ctx->returnStatement()->value()));
            return dynamic_pointer_cast<Statement>(ret);
        }
        else if (ctx->conditionStatement()) {
            auto cond = make_shared<Condition>();

            auto condIden = make_shared<IdentifierValue>();
            condIden->setIdentifier(visitIdentifier(ctx->conditionStatement()->identifier()));
            cond->setConditionValue(condIden);

            for (auto stat : ctx->conditionStatement()->thenStatements()->statement()) {
                cond->addThenStatement(visitStatement(stat));
            }

            if (ctx->conditionStatement()->statement()) {
                cond->addElseStatement(visitStatement(ctx->conditionStatement()->statement()));
            }
            else if (ctx->conditionStatement()->elseStatements()) {
                for (auto stat : ctx->conditionStatement()->elseStatements()->statement()) {
                    cond->addElseStatement(visitStatement(stat));
                }
            }

            return dynamic_pointer_cast<Statement>(cond);
        }

        return NULL;
    }

    antlrcpp::Any visitIdentifier(LangParser::IdentifierContext *ctx) override {
        auto identifier = make_shared<Identifier>();
        identifier->setFullName(ctx->getText());
        return identifier;
    }

    antlrcpp::Any visitValue(LangParser::ValueContext *ctx) override {
        if (ctx->identifier()) {
            auto identifierVal = make_shared<IdentifierValue>();
            identifierVal->setIdentifier(visitIdentifier(ctx->identifier()));
            return dynamic_pointer_cast<Value>(identifierVal);
        }
        else if (ctx->constantValue()) {
            return dynamic_pointer_cast<Value, ConstantValue>(visitConstantValue(ctx->constantValue()));
        }

        return NULL;
    }

    antlrcpp::Any visitConstantValue(LangParser::ConstantValueContext *ctx) override {
        if (ctx->boolValue()) {
            auto val = make_shared<BooleanValue>();
            val->setValue(ctx->boolValue()->getText() == "true");
            return dynamic_pointer_cast<ConstantValue>(val);
        }
        else if (ctx->integerValue()) {
            auto val = make_shared<IntegerValue>();
            val->setValue(stoll(ctx->integerValue()->getText()));
            return dynamic_pointer_cast<ConstantValue>(val);
        }
        else if (ctx->floatValue()) {
            auto val = make_shared<FloatValue>();
            val->setValue(stod(ctx->floatValue()->getText()));
            return dynamic_pointer_cast<ConstantValue>(val);
        }
        else if (ctx->charValue()) {
            auto val = make_shared<CharValue>();

            char32_t ch = utfConverter.from_bytes(ctx->charValue()->getText()).substr(1, 1)[0];
            val->setValue(ch);

            return dynamic_pointer_cast<ConstantValue>(val);
        }
        else if (ctx->stringValue()) {
            auto val = make_shared<StringValue>();

            u32string str = utfConverter.from_bytes(ctx->stringValue()->getText());
            val->setValue(str.substr(1, str.length() - 2));

            return dynamic_pointer_cast<ConstantValue>(val);
        }
        else if (ctx->objValue()) {
            return dynamic_pointer_cast<ConstantValue>(make_shared<NullValue>());
        }

        return NULL;
    }

private:
    wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utfConverter;
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
    program = visitor.visitFile(parser.file());
}
