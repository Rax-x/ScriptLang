#include "../include/compiler.h"
#include "../include/disassembler.h"
#include "../include/utils.h"

#include <iostream>

namespace scriptlang::compiler {

using scriptlang::utils::instanceof;
using scriptlang::disassembler::Disassembler;

constexpr Byte BREAK_PLACEHOLDER = 0xBB;

auto Compiler::compile(const std::vector<StatementPtr>& ast) -> ObjectFunction {

    for(const auto& stmt : ast){
        compileStatement(stmt);
    }

    emit(OpCode::Nil);
    emit(OpCode::Return);

    if(debugMode_){
        Disassembler disassembler(std::cout);

        const char* name = !compilingFunction_.name.empty()
            ? compilingFunction_.name.c_str()
            : "<script>";

        disassembler.disassembleChunk(name, compilingFunction_.chunk);
    }

    return compilingFunction_;
}

auto Compiler::addLocal(const Token& name) -> std::uint8_t {
    locals_[localsCount_++] = Local { name, -1 };
    return localsCount_ - 1;
}

auto Compiler::declareVariable(const Token& name) -> void {

    if(scopeDepth_ == 0) return;

    if(localsCount_ > MAX_LOCALS){
        emitError("Each scope can have maximun 256 locals.");
        return;
    }

    const Local* local = nullptr;
    for(int i = localsCount_ - 1; i >= 0; i--){
        local = &locals_[i];
        if(local->depth != -1 && local->depth < scopeDepth_) break;

        if(name.lexeme == local->name.lexeme) {
            emitError("Variable already declared.");
            break;
        }
    }

    addLocal(name);
}

auto Compiler::defineVariable(const Token& name) -> void {

    if(scopeDepth_ > 0){
        markVariableAsDefined();
        return;
    }

    Byte index = currentChunk().addConstant(std::string(name.lexeme));
    emit(OpCode::DefineGlobal);
    emit(index);
}

auto Compiler::markVariableAsDefined() -> void {
    locals_[localsCount_ - 1].depth = scopeDepth_;
}

auto Compiler::resolveVariableName(const Token& name) -> int {

    const Local* local = nullptr;
    for(int i = localsCount_ - 1; i >= 0; i--){
        local = &locals_[i];

        if(name.lexeme == local->name.lexeme) {

            if(local->depth == -1) {
                emitError("You can't use a variable in it's own initializer.");
            }

            return i;
        }
    }

    return -1;
}

auto Compiler::visitVariableDeclaration(const VariableDeclaration& decl) -> void { 
    declareVariable(decl.name());

    compileExpression(decl.initializer());
    defineVariable(decl.name());
}

auto Compiler::visitFunctionDeclaration(const FunctionDeclaration& decl) -> void {
    
    if(type_ == FunctionType::Function){
        emitError("Can't declare a function inside another function.");
        return;
    }

    Compiler compiler(FunctionType::Function, this->reporter_, debugMode_);
    compiler.compilingFunction_.name = decl.name().lexeme;

    compiler.beginScope();

    for(const auto& param : decl.params()){
        compiler.declareVariable(param);
        compiler.defineVariable(param);
    }

    if(!instanceof<Statement, Block>(decl.body().get())){
        emitError("Invalid function body.");
        return;
    }

    const auto& bodyAst = static_cast<Block*>(decl.body().get())->statements();
    ObjectFunction function = compiler.compile(bodyAst);

    function.arity = decl.params().size();

    emit(OpCode::PushConstant);

    Byte index = currentChunk().addConstant(std::move(function));
    emit(index);

    defineVariable(decl.name());
}

auto Compiler::visitBlock(const Block& block) -> void { 
    beginScope();

    for(const auto& stmt : block.statements()){
        compileStatement(stmt);
    }

    endScope();
}

auto Compiler::beginLoop(Loop* loop) -> void {

    loop->enclosing = loop_;
    loop->scopeDepth = scopeDepth_;
    loop->start = currentChunk().size();

    loop_ = loop;
}

auto Compiler::patchLoopBreaks() -> void {

    std::uint32_t i = loop_->start;
    while(i < loop_->end){
        if(currentChunk()[i] == BREAK_PLACEHOLDER){

            const Short offset = loop_->end - i;

            currentChunk()[i] = OpCode::Jump;
            currentChunk()[i+1] = (offset >> 8) & 0xff;
            currentChunk()[i+2] = offset & 0xff;
            i += 3;
        } else {
            switch(currentChunk()[i]){
                case OpCode::Return:
                    [[fallthrough]];
                case OpCode::True:
                    [[fallthrough]];
                case OpCode::False:
                    [[fallthrough]];
                case OpCode::Nil:
                    [[fallthrough]];
                case OpCode::Pop:
                    [[fallthrough]];
                case OpCode::Add:
                    [[fallthrough]];
                case OpCode::Sub:
                    [[fallthrough]];
                case OpCode::Div:
                    [[fallthrough]];
                case OpCode::Mult:
                    [[fallthrough]];
                case OpCode::Less:
                    [[fallthrough]];
                case OpCode::Greater:
                    [[fallthrough]];
                case OpCode::Equal:
                    [[fallthrough]];
                case OpCode::Pow:
                    [[fallthrough]];
                case OpCode::Not:
                    [[fallthrough]];
                case OpCode::Negate:
                    [[fallthrough]];
                case OpCode::Print:
                    i++;
                    break;
                case OpCode::JumpIfFalse:
                    [[fallthrough]];
                case OpCode::Jump:
                    [[fallthrough]];
                case OpCode::Loop:
                    i += 3;
                    break;
                case OpCode::PushConstant:
                    [[fallthrough]];
                case OpCode::GetLocal:
                    [[fallthrough]];
                case OpCode::SetLocal:
                    [[fallthrough]];
                case OpCode::DefineGlobal:
                    [[fallthrough]];
                case OpCode::GetGlobal:
                    [[fallthrough]];
                case OpCode::SetGlobal:
                    [[fallthrough]];
                case OpCode::Call:
                    i += 2;
                    break;
                default:
                    i++;
                    break;
            }
        }
    }

}

auto Compiler::endLoop() -> void {

    patchLoopBreaks();

    loop_ = loop_->enclosing;
}

auto Compiler::emitLoop(int start) -> void {
 
    loop_->end = currentChunk().size();

    emit(OpCode::Loop);
    const int offset = currentChunk().size() - start + 2;
        
    if(offset > UINT16_MAX){
        emitError("Loop body too large.");
        return;
    }
        
    currentChunk().write((offset >> 8) & 0xff, currentNodeLocation_.start.line);
    currentChunk().write(offset & 0xff, currentNodeLocation_.start.line);
}

auto Compiler::visitWhileStatement(const WhileStatement& stmt) -> void {

    Loop loop;
    beginLoop(&loop);

    compileExpression(stmt.condition());

    const int exitJump = emitJump(OpCode::JumpIfFalse);
    emit(OpCode::Pop);

    compileStatement(stmt.body());

    emitLoop(loop.start);

    patchJump(exitJump);
    endLoop();

    emit(OpCode::Pop);
}

auto Compiler::visitIfStatement(const IfStatement& stmt) -> void { 

    compileExpression(stmt.condition());
    
    const int thenJump = emitJump(OpCode::JumpIfFalse);
    emit(OpCode::Pop);

    compileStatement(stmt.thenBranch());

    const int elseJump = emitJump(OpCode::Jump);

    patchJump(thenJump);
    emit(OpCode::Pop);

    if(stmt.haveElseBranch()){
        compileStatement(stmt.elseBranch());
    }

    patchJump(elseJump);
}

auto Compiler::visitExpressionStatement(const ExpressionStatement& stmt) -> void { 
    compileExpression(stmt.expression());
    emit(OpCode::Pop);
}

auto Compiler::visitContinueStatement([[maybe_unused]] const ContinueStatement& stmt) -> void {
    
    if(loop_ == nullptr){
        emitError("Can't use 'continue' outside a loop.");
    }

    for(int i = localsCount_ - 1; i >= 0 && locals_[i].depth > loop_->scopeDepth; i--){
        emit(OpCode::Pop);
    }

    emitLoop(loop_->start);
}

auto Compiler::visitBreakStatement([[maybe_unused]] const BreakStatement& stmt) -> void {

    if(loop_ == nullptr){
        emitError("Can't use 'break' outside a loop.");
    }
 

    for(int i = localsCount_ - 1; i >= 0 && locals_[i].depth > loop_->scopeDepth; i--){
        emit(OpCode::Pop);
    }

    emit(BREAK_PLACEHOLDER);
    emit(Byte(0xff));
    emit(Byte(0xff));
}

auto Compiler::visitReturnStatement(const ReturnStatement& stmt) -> void {
    if(type_ == FunctionType::Script){
        emitError("Can't return from top-level.");
        return;
    }

    stmt.haveExpression()
        ? compileExpression(stmt.expression())
        : emit(OpCode::Nil);

    emit(OpCode::Return);
}

auto Compiler::visitPrintStatement(const PrintStatement& stmt) -> void {
    compileExpression(stmt.expression());
    emit(OpCode::Print);
}

auto Compiler::visitAssignmentExpression(const AssignmentExpression& expr) -> void { 

    compileExpression(expr.value());
    
    int index = resolveVariableName(expr.name());

    if(index == -1){
        index = currentChunk().addConstant(std::string(expr.name().lexeme));
        emit(OpCode::SetGlobal);
    } else {
        emit(OpCode::SetLocal);
    }

    emit(static_cast<Byte>(index));
}

auto Compiler::visitBinaryExpression(const BinaryExpression& expr) -> void { 

    const TokenType operatorType = expr.op().type;

    if(operatorType == TokenType::AndKeyword){

        compileExpression(expr.left());

        const int jump = emitJump(OpCode::JumpIfFalse);
        emit(OpCode::Pop);

        compileExpression(expr.right());
        patchJump(jump);

        return;
    }

    if(operatorType == TokenType::OrKeyword){

        compileExpression(expr.left());
        const int elseJump = emitJump(OpCode::JumpIfFalse);
        const int endJump = emitJump(OpCode::Jump);

        patchJump(elseJump);
        emit(OpCode::Pop);
        compileExpression(expr.right());

        patchJump(endJump);

        return;
    }

    compileExpression(expr.left());
    compileExpression(expr.right());

    switch(operatorType){
        case TokenType::Minus:
            emit(OpCode::Sub);
            break;
        case TokenType::Plus:
            emit(OpCode::Add);
            break;
        case TokenType::Star:
            emit(OpCode::Mult);
            break;
        case TokenType::Slash:
            emit(OpCode::Div);
            break;
        case TokenType::Exponent:
            emit(OpCode::Pow);
            break;
        case TokenType::Less:
            emit(OpCode::Less);
            break;
        case TokenType::Greater:
            emit(OpCode::Greater);
            break;
        case TokenType::LessEqual:
            emit(OpCode::Greater);
            emit(OpCode::Not);
            break;
        case TokenType::GreaterEqual:
            emit(OpCode::Less);
            emit(OpCode::Not);
            break;
        case TokenType::Equal:
            emit(OpCode::Equal);
            break;
        case TokenType::NotEqual:
            emit(OpCode::Equal);
            emit(OpCode::Not);
            break;
        default:
            emitError("Unkown operator '%.*s'.", expr.op().lexeme.size(), expr.op().lexeme.data());
            break;
    }
}

auto Compiler::visitUnaryExpression(const UnaryExpression& expr) -> void { 
    compileExpression(expr.right());

    switch(expr.op().type){
        case TokenType::Minus:
            emit(OpCode::Negate);
            break;
        case TokenType::NotKeyword:
            emit(OpCode::Not);
            break;
        case TokenType::Plus:
            break;
        default:
            emitError("Invalid unary operator '%.*s'.", expr.op().lexeme.size(), expr.op().lexeme.data());
            break;
    }

}

auto Compiler::visitCallExpression(const CallExpression& expr) -> void {

    compileExpression(expr.callee());

    for(const auto& arg :  expr.arguments()){
        compileExpression(arg);
    }

    emit(OpCode::Call);
    emit(static_cast<Byte>(expr.arguments().size()));
}

auto Compiler::visitGroupingExpression(const GroupingExpression& expr) -> void { 
    compileExpression(expr.expression());
}

auto Compiler::visitVariableExpression(const VariableExpression& expr) -> void { 

    int index = resolveVariableName(expr.name());

    if(index == -1){
        index = currentChunk().addConstant(std::string(expr.name().lexeme));
        emit(OpCode::GetGlobal);
    } else {
        emit(OpCode::GetLocal);
    }

    emit(static_cast<Byte>(index));
}

auto Compiler::visitLiteralExpression(const LiteralExpression& expr) -> void { 

    std::uint8_t index;

    if(expr.isBoolean()){
        emit(expr.asBoolean() ? OpCode::True : OpCode::False);
    } else if(expr.isNumber()){
        emit(OpCode::PushConstant);
        index = currentChunk().addConstant(expr.asNumber());
        emit(index);
    } else if(expr.isString()){
        emit(OpCode::PushConstant);
        index = currentChunk().addConstant(expr.asString());
        emit(index);
    } else if(expr.isNil()){
        emit(OpCode::Nil);
    }
}

template<typename... Args>
auto Compiler::emitError(const char* fmt, Args&&... args) -> void {
    if(reporter_ != nullptr){
        reporter_->error(currentNodeLocation_, fmt, std::forward<Args>(args)...);
    }
}

}
