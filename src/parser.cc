#include "../include/parser.h"
#include "../include/utils.h"

#include <cstdio>
#include <cmath>
#include <functional>
#include <optional>
#include <utility>

namespace scriptlang::parser {

using namespace utils;

Parser::Parser(std::string_view source, ErrorReporter* reporter)
    : lex_(source),
      reporter_(reporter){
    advance(); // get first token

    // Register parser rules

    registerInfix(TokenType::Assign, Precedence::Assignment, &Parser::assignmentExpression);
    registerInfix(TokenType::Slash, Precedence::Factor, &Parser::binaryExpression);
    registerInfix(TokenType::Star, Precedence::Factor, &Parser::binaryExpression);
    registerInfix(TokenType::Exponent, Precedence::Exponent, &Parser::binaryExpression);

    registerInfix(TokenType::Less, Precedence::Comparison, &Parser::binaryExpression);
    registerInfix(TokenType::Greater, Precedence::Comparison, &Parser::binaryExpression);
    registerInfix(TokenType::GreaterEqual, Precedence::Comparison, &Parser::binaryExpression);
    registerInfix(TokenType::LessEqual, Precedence::Comparison, &Parser::binaryExpression);
    registerInfix(TokenType::NotEqual, Precedence::Equality, &Parser::binaryExpression);
    registerInfix(TokenType::Equal, Precedence::Equality, &Parser::binaryExpression);

    registerInfix(TokenType::AndKeyword, Precedence::LogicAnd, &Parser::binaryExpression);
    registerInfix(TokenType::OrKeyword, Precedence::LogicOr, &Parser::binaryExpression);

    registerRule(TokenType::Plus, Precedence::Term, &Parser::unaryExpression, &Parser::binaryExpression);
    registerRule(TokenType::Minus, Precedence::Term, &Parser::unaryExpression, &Parser::binaryExpression);
    registerRule(TokenType::LeftParen, Precedence::Call, &Parser::primaryExpression, &Parser::callExpression);
    
    registerPrefix(TokenType::NotKeyword, Precedence::Unary, &Parser::unaryExpression);

    registerPrefix(TokenType::Identifier, Precedence::Primary, &Parser::primaryExpression);
    registerPrefix(TokenType::NumberLiteral, Precedence::Primary, &Parser::primaryExpression);
    registerPrefix(TokenType::StringLiteral, Precedence::Primary, &Parser::primaryExpression);
    registerPrefix(TokenType::TrueKeyword, Precedence::Primary, &Parser::primaryExpression);
    registerPrefix(TokenType::FalseKeyword, Precedence::Primary, &Parser::primaryExpression);
    registerPrefix(TokenType::NilKeyword, Precedence::Primary, &Parser::primaryExpression);
}

auto Parser::parseSoruce() -> std::vector<StatementPtr> {

    std::vector<StatementPtr> statements;

    while(!isAtEnd()) {
        start_ = peek();

        statements.push_back(declaration());

        if(panicMode_){
            synchronize();
        }
    }

    return statements;
}

auto Parser::declaration() -> StatementPtr {

    if(match(TokenType::LetKeyword)){
        return variableDeclaration();
    } else if(match(TokenType::DefunKeyword)) {
        return functionDeclaration();
    }

    return statement();
}

auto Parser::variableDeclaration() -> StatementPtr {
    auto name = consume(TokenType::Identifier, "Expect variable name after 'let' keyword.");
    
    if(!name.has_value()) return nullptr;
    
    consume(TokenType::Assign, "Expect '=' after variable name.");
    ExpressionPtr initializer = expression();
    consume(TokenType::Semicolon, "Expect ';' at end of let statement.");

    return makeStatement<VariableDeclaration>(currentSourceRange(), name.value(), initializer);
}

auto Parser::functionDeclaration() -> StatementPtr {

    std::vector<Token> parameters;
    
    auto name = consume(TokenType::Identifier, "Expect function name after 'defun' keyword.");
    if(!name.has_value()) return nullptr;

    consume(TokenType::LeftParen, "Expect '(' after function name.");

    if(!match(TokenType::RightParen)) {
        do{
            auto param = consume(TokenType::Identifier, "Expect parameter name.");
            if(!param.has_value()) return nullptr;

            parameters.push_back(param.value());
        } while(match(TokenType::Comma));

        consume(TokenType::RightParen, "Expect ')' after parameters.");
    }

    consume(TokenType::LeftBrace, "Expect '{' before function body.");
    StatementPtr body = block();

    return makeStatement<FunctionDeclaration>(currentSourceRange(), name.value(), parameters, body);
}

auto Parser::statement() -> StatementPtr {

    
    if(match(TokenType::IfKeyword)){
        return ifStatement();
    } else if(match(TokenType::WhileKeyword)){
        return whileStatement();
    } else if(match(TokenType::PrintKeyword)) {
        return printStatement();
    } else if(match(TokenType::ReturnKeyword)) {
        return returnStatement();
    } else if(match(TokenType::ContinueKeyword)) {
        return continueStatement();
    } else if(match(TokenType::BreakKeyword)) {
        return breakStatement();
    } else if(match(TokenType::LeftBrace)) {
        return block();
    }

    return expressionStatement();
}

auto Parser::block() -> StatementPtr {
    std::vector<StatementPtr> statements;
 
    while(!check(TokenType::RightBrace) && !isAtEnd()){
        statements.push_back(declaration());
    }

    consume(TokenType::RightBrace, "Expect '}' after block.");
   
    return makeStatement<Block>(currentSourceRange(), statements);
}

auto Parser::whileStatement() -> StatementPtr {
    ExpressionPtr condition = expression();

    consume(TokenType::LeftBrace, "Expect '{' before then branch.");
    StatementPtr body = block();
    
    return makeStatement<WhileStatement>(currentSourceRange(), condition, body);
}

    
auto Parser::ifStatement() -> StatementPtr {
    ExpressionPtr condition = expression();

    consume(TokenType::LeftBrace, "Expect '{' before then branch.");
    StatementPtr thenBranch = block();
    
    StatementPtr elseBranch = nullptr;
    if(match(TokenType::ElseKeyword)){
        consume(TokenType::LeftBrace, "Expect '{' before else branch.");
        elseBranch = block();
    }

    return makeStatement<IfStatement>(currentSourceRange(), condition, thenBranch, elseBranch);
}

auto Parser::expressionStatement() -> StatementPtr {
    ExpressionPtr expr = expression();
    consume(TokenType::Semicolon, "Expect ';' after expression.");

    return makeStatement<ExpressionStatement>(currentSourceRange(), expr);
}

auto Parser::continueStatement() -> StatementPtr {
    consume(TokenType::Semicolon, "Expect ';' after continue statement.");
    return makeStatement<ContinueStatement>(currentSourceRange());
}

auto Parser::breakStatement() -> StatementPtr {
    consume(TokenType::Semicolon, "Expect ';' after break statement.");
    return makeStatement<BreakStatement>(currentSourceRange());
}

auto Parser::returnStatement() -> StatementPtr {
    ExpressionPtr return_value = nullptr;

    if(!match(TokenType::Semicolon)){
        return_value = expression();
    }

    consume(TokenType::Semicolon, "Expect ';' at end of return statement.");
    return makeStatement<ReturnStatement>(currentSourceRange(), return_value);
}

auto Parser::printStatement() -> StatementPtr {
    ExpressionPtr expr = expression();
    consume(TokenType::Semicolon, "Expect ';' at end of print statement.");
    
    return makeStatement<PrintStatement>(currentSourceRange(), expr);
}

auto Parser::getParseRules(TokenType type) -> ParseRule {
    try {
        return rules_.at(type);
    } catch(...) {
    }

    return {Precedence::None, nullptr,  nullptr };
}

auto Parser::parsePrecedence(Precedence prec) -> ExpressionPtr {
    advance();
    Token op = previous();

    ParsePrefix prefix = getParseRules(op.type).prefix;

    if(nullptr == prefix){
        error("Expect an expression.");
        return nullptr;
    }

    ExpressionPtr left = std::invoke(prefix, this);

    while(prec < getParseRules(peek().type).prec){
        advance();
        op = previous();

        ParseInfix infix = getParseRules(op.type).infix;

        if(infix == nullptr) break;
        left = std::invoke(infix, this, left);
    }
    

    return left;
}

auto Parser::expression() -> ExpressionPtr {
    return parsePrecedence(Precedence::None);
}

auto Parser::assignmentExpression(ExpressionPtr& left) -> ExpressionPtr {

    if(!instanceof<Expression, VariableExpression>(left.get())){
        error("Expect an lvalue.");
        return nullptr;
    }

    constexpr auto precedence = static_cast<Precedence>(Precedence::Assignment - 1);
    ExpressionPtr right = parsePrecedence(precedence);

    const auto expr = static_cast<VariableExpression*>(left.get());

    return makeExpression<AssignmentExpression>(currentSourceRange(), expr->name(), right);
}

auto Parser::binaryExpression(ExpressionPtr& left) -> ExpressionPtr {
    Token op = previous();

    auto precedence = static_cast<Precedence>(getParseRules(op.type).prec);
    ExpressionPtr right = parsePrecedence(precedence);

    return makeExpression<BinaryExpression>(currentSourceRange(), op, left, right);
}

auto Parser::unaryExpression() -> ExpressionPtr {
    Token op = previous();
    ExpressionPtr right = parsePrecedence(Precedence::Unary);
    return makeExpression<UnaryExpression>(currentSourceRange(), op, right);
}

auto Parser::callExpression(ExpressionPtr& left) -> ExpressionPtr {
    
    std::vector<ExpressionPtr> args;

    if(!match(TokenType::RightParen)){
        do{
            ExpressionPtr expr = expression();
        
            if(expr == nullptr) {
                error("Invalid argument.");
                return nullptr;
            }

            args.push_back(std::move(expr));
        } while(match(TokenType::Comma));

        consume(TokenType::RightParen, "Expect ')' after arguments.");
    }

    return makeExpression<CallExpression>(currentSourceRange(), left, args);
}

auto Parser::primaryExpression() -> ExpressionPtr {
    Token token = previous();

    switch(token.type) {
        case TokenType::LeftParen: {
            ExpressionPtr expr = expression();
            consume(TokenType::RightParen, "Expect ')' after a grouping expression.");
            return makeExpression<GroupingExpression>(currentSourceRange(), expr);
        }
        case TokenType::StringLiteral:
            return makeExpression<LiteralExpression>(currentSourceRange(), 
                                                     token.lexeme.substr(1, token.lexeme.size() - 2));
        case TokenType::NumberLiteral: {
            double number = std::strtod(token.lexeme.data(), nullptr);
            return makeExpression<LiteralExpression>(currentSourceRange(), number);
        }
        case TokenType::TrueKeyword:
            return makeExpression<LiteralExpression>(currentSourceRange(), true);
        case TokenType::FalseKeyword:
            return makeExpression<LiteralExpression>(currentSourceRange(), false);
        case TokenType::Identifier:
            return makeExpression<VariableExpression>(currentSourceRange(), token);
        case TokenType::NilKeyword:
            return makeExpression<LiteralExpression>(currentSourceRange());
        default:
            break;
    }

    error("Expect a literal or grouping expression.");

    return nullptr;
}

auto Parser::currentSourceRange() const -> SourceRange {
    return SourceRange { start_.position.start, curr_.position.end };
}

template<typename... Args>
auto Parser::error(const char* fmt, Args&&... args) -> void {
   
    if(reporter_ != nullptr) {
        reporter_->error(currentSourceRange(), fmt, std::forward<Args>(args)...);
    }

    panicMode_ = true;
}

auto Parser::synchronize() -> void {

    while(!isAtEnd()){
        switch(peek().type){
            case TokenType::DefunKeyword:
                [[fallthrough]];
            case TokenType::LetKeyword:
                [[fallthrough]];
            case TokenType::IfKeyword:
                [[fallthrough]];
            case TokenType::WhileKeyword:
                [[fallthrough]];
            case TokenType::BreakKeyword:
                [[fallthrough]];
            case TokenType::ContinueKeyword:
                [[fallthrough]];
            case TokenType::ReturnKeyword:
                return;
            default:
                advance();
                break;
        }
    }
    
    panicMode_ = false;
}

auto Parser::advance() -> void {
    if(isAtEnd()) return;
    prev_ = curr_;
    curr_ = lex_.next();
}

auto Parser::check(TokenType type) const -> bool {
    return peek().type == type;
}

auto Parser::match(TokenType type) -> bool {
    return check(type)
        ? (advance(), true)
        : false;
}

auto Parser::consume(TokenType type, const char* errorMessage) -> std::optional<Token> {
    
    if(match(type)){
        return previous();
    }
    
    error(errorMessage);
    return std::nullopt;
}

}
