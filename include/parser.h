#ifndef _PARSER_H_p
#define _PARSER_H_

#include "ast.h"
#include "error_reporter.h"
#include "lexer.h"
#include "source_position.h"
#include "token.h"

#include <optional>
#include <type_traits>
#include <utility>

namespace scriptlang::parser {

using namespace ast;
using namespace error;

class Parser {

    typedef auto (Parser::*ParseInfix)(ExpressionPtr&) -> ExpressionPtr;
    typedef auto (Parser::*ParsePrefix)() -> ExpressionPtr;

    enum Precedence {
        None,
        Assignment,
        LogicOr,
        LogicAnd,
        Equality,
        Comparison,
        Term,
        Factor,
        Unary,
        Exponent,
        Call,
        Primary
    };

    struct ParseRule {
        Precedence prec;
        ParseInfix infix;
        ParsePrefix prefix;
    };

public:
    Parser(std::string_view source, ErrorReporter* reporter = nullptr);

    [[nodiscard]]
    auto parseSoruce() -> std::vector<StatementPtr>;

private:

    auto declaration() -> StatementPtr;

    auto variableDeclaration() -> StatementPtr;
    auto functionDeclaration() -> StatementPtr;
    auto typeDeclaration() -> StatementPtr;

    auto statement() -> StatementPtr;
    
    auto block() -> StatementPtr;
    auto ifStatement() -> StatementPtr;
    auto whileStatement() -> StatementPtr;
    auto expressionStatement() -> StatementPtr;
    auto continueStatement() -> StatementPtr;
    auto breakStatement() -> StatementPtr;
    auto returnStatement() -> StatementPtr;
    auto printStatement() -> StatementPtr;

    auto getParseRules(TokenType type) -> ParseRule;
    auto parsePrecedence(Precedence prec) -> ExpressionPtr;

    inline auto registerRule(TokenType type, Precedence prec, 
                             ParsePrefix prefix, ParseInfix infix) -> void {
        rules_[type] = {prec, infix, prefix};
    }
    
    inline auto registerInfix(TokenType type, Precedence prec, ParseInfix infix) -> void {
        registerRule(type, prec, nullptr, infix);
    }

    inline auto registerPrefix(TokenType type, Precedence prec, ParsePrefix prefix) -> void {
        registerRule(type, prec, prefix, nullptr);
    }

    auto expression() -> ExpressionPtr;

    auto assignmentExpression(ExpressionPtr& left) -> ExpressionPtr;
    auto binaryExpression(ExpressionPtr& left) -> ExpressionPtr;
    auto callExpression(ExpressionPtr& left) -> ExpressionPtr;
    auto unaryExpression() -> ExpressionPtr;
    auto primaryExpression() -> ExpressionPtr;

private:

    auto currentSourceRange() const -> SourceRange;

    template<typename... Args>
    auto error(const char* fmt, Args&&... args) -> void;

    auto synchronize() -> void;

    auto advance() -> void;
    auto check(TokenType type) const -> bool;
    auto match(TokenType type) -> bool;

    auto consume(TokenType type, const char *errorMessage) -> std::optional<Token>;

    auto inline previous() const -> Token {
        return prev_;
    }

    auto inline peek() const -> Token {
        return curr_;
    }

    auto inline isAtEnd() const -> bool {
        return !lex_.hasNext();
    }

private:
    Lexer lex_;

    Token curr_;
    Token prev_;

    Token start_;

    std::map<TokenType, ParseRule> rules_;

    bool panicMode_ = false;
    ErrorReporter* reporter_;

};

}

#endif

