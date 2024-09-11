#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "ast.h"
#include "error_reporter.h"
#include "objects.h"
#include "types.h"
#include "vm.h"

namespace scriptlang::compiler {

using namespace ast;
using namespace runtime;
using namespace error;
using namespace types;

class Compiler : private AstVisitor {

    struct Local {
        Token name;
        int depth;
    };

    struct Loop {
        Loop* enclosing;

        int scopeDepth;
        std::uint32_t start;
        std::uint32_t end;
    };

public:
    static constexpr auto MAX_LOCALS = BYTE_MAX;

    enum class FunctionType {
        Function,
        Script
    };

    Compiler(FunctionType type, ErrorReporter* reporter, bool debugMode = false)
        : type_(type),
          reporter_(reporter),
          debugMode_(debugMode) {};

    auto compile(const std::vector<StatementPtr>& ast) -> ObjectFunction;

private:

    inline auto compileExpression(const ExpressionPtr& expr) -> void {
        currentNodeLocation_ = expr->location();
        expr->accept(*this);
    }

    inline auto compileStatement(const StatementPtr& stmt) -> void {
        currentNodeLocation_ = stmt->location();
        stmt->accept(*this);
    }

    template<typename T,
             typename = std::enable_if_t<std::is_same_v<T, Byte> || 
                                         std::is_same_v<T, OpCode>>>
    inline auto emit(T byte) -> void {
        currentChunk().write(byte, currentNodeLocation_.start.line);
    }

    inline auto emitJump(OpCode instruction) -> Short {
        emit(instruction);

        emit(Byte(0xff));
        emit(Byte(0xff));

        return currentChunk().size() - 2;
    }

    auto beginLoop(Loop* loop) -> void;
    auto endLoop() -> void;
    auto emitLoop(int start) -> void;

    auto patchLoopBreaks() -> void;

    inline auto patchJump(int offset) -> void {

        const int jump = currentChunk().size() - offset - 2;

        if(jump > SHORT_MAX){
            emitError("Too long jump.");
            return;
        }

        currentChunk()[offset] = (jump >> 8) & 0xff;
        currentChunk()[offset+1] = jump & 0xff;
    }

    constexpr auto currentChunk() -> Chunk& {
        return compilingFunction_.chunk;
    }

    constexpr auto beginScope() -> void {
        scopeDepth_++;
    }

    inline auto endScope() -> void {
        scopeDepth_--;

        const int currentScope = scopeDepth_;
        while(localsCount_ > 0 && locals_[localsCount_ - 1].depth > currentScope) {
            emit(OpCode::Pop);
            localsCount_--;
        }
    }

    auto addLocal(const Token& name) -> std::uint8_t;
    auto declareVariable(const Token& name) -> void;
    auto defineVariable(const Token& name) -> void;
    auto markVariableAsDefined() -> void;
    auto resolveVariableName(const Token& name) -> int;

    template<typename... Args>
    auto emitError(const char* fmt, Args&&... args) -> void;

private:
    auto visitVariableDeclaration(const VariableDeclaration& decl) -> void;
    auto visitFunctionDeclaration(const FunctionDeclaration& decl) -> void;

    auto visitBlock(const Block& block) -> void;
    auto visitWhileStatement(const WhileStatement& stmt) -> void;
    auto visitIfStatement(const IfStatement& stmt) -> void;
    auto visitExpressionStatement(const ExpressionStatement& stmt) -> void;
    auto visitContinueStatement(const ContinueStatement& stmt) -> void;
    auto visitBreakStatement(const BreakStatement& stmt) -> void;
    auto visitReturnStatement(const ReturnStatement& stmt) -> void;
    auto visitPrintStatement(const PrintStatement& stmt) -> void;

    auto visitAssignmentExpression(const AssignmentExpression& expr) -> void;
    auto visitBinaryExpression(const BinaryExpression& expr) -> void;
    auto visitUnaryExpression(const UnaryExpression& expr) -> void;
    auto visitCallExpression(const CallExpression& expr) -> void;
    auto visitGroupingExpression(const GroupingExpression& expr) -> void;
    auto visitVariableExpression(const VariableExpression& expr) -> void;
    auto visitLiteralExpression(const LiteralExpression& expr) -> void;

private:

    FunctionType type_;
    ErrorReporter* reporter_;
    bool debugMode_;

    SourceRange currentNodeLocation_;

    ObjectFunction compilingFunction_;
    Loop* loop_ = nullptr;

    Short scopeDepth_ = 0;

    Local locals_[MAX_LOCALS];
    int localsCount_ = 1;
    
};


}


#endif
