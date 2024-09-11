#ifndef _AST_H_
#define _AST_H_

#include "token.h"
#include "source_position.h"

#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>
#include <variant>
#include <functional>

namespace scriptlang::ast {

using namespace lexer;

// Forward
class VariableDeclaration;
class FunctionDeclaration;
class Block;
class WhileStatement;
class IfStatement;
class ExpressionStatement;
class ContinueStatement;
class BreakStatement;
class ReturnStatement;
class PrintStatement;
class AssignmentExpression;
class BinaryExpression;
class UnaryExpression;
class CallExpression;
class GroupingExpression;
class VariableExpression;
class LiteralExpression;

class AstVisitor {
public:
    AstVisitor() = default;
    virtual ~AstVisitor() = default;
    
    virtual auto visitVariableDeclaration(const VariableDeclaration& decl) -> void = 0;
    virtual auto visitFunctionDeclaration(const FunctionDeclaration& decl) -> void = 0;

    virtual auto visitBlock(const Block& block) -> void = 0;
    virtual auto visitWhileStatement(const WhileStatement& stmt) -> void = 0;
    virtual auto visitIfStatement(const IfStatement& stmt) -> void = 0;
    virtual auto visitExpressionStatement(const ExpressionStatement& stmt) -> void = 0;
    virtual auto visitContinueStatement(const ContinueStatement& stmt) -> void = 0;
    virtual auto visitBreakStatement(const BreakStatement& stmt) -> void = 0;
    virtual auto visitReturnStatement(const ReturnStatement& stmt) -> void = 0;
    virtual auto visitPrintStatement(const PrintStatement& stmt) -> void = 0;

    virtual auto visitAssignmentExpression(const AssignmentExpression& expr) -> void = 0;
    virtual auto visitBinaryExpression(const BinaryExpression& expr) -> void = 0;
    virtual auto visitUnaryExpression(const UnaryExpression& expr) -> void = 0;
    virtual auto visitCallExpression(const CallExpression& expr) -> void = 0;
    virtual auto visitGroupingExpression(const GroupingExpression& expr) -> void = 0;
    virtual auto visitVariableExpression(const VariableExpression& expr) -> void = 0;
    virtual auto visitLiteralExpression(const LiteralExpression& expr) -> void = 0;
};

class Expression {
public:
    constexpr Expression(SourceRange location)
        : location_(location) {}

    virtual ~Expression() = default;

    virtual auto accept(AstVisitor& visitor) -> void = 0;

    inline auto location() const -> SourceRange {
        return location_;
    }

private:
    SourceRange location_;
};

class Statement {
public:
    constexpr Statement(SourceRange location)
        : location_(location) {}

    virtual ~Statement() = default;

    virtual auto accept(AstVisitor& visitor) -> void = 0;

    inline auto location() const -> SourceRange {
        return location_;
    }

private:
    SourceRange location_;
};

using ExpressionPtr = std::unique_ptr<Expression>;
using StatementPtr = std::unique_ptr<Statement>;

template<typename T, typename... Args,
         typename = std::enable_if_t<std::is_base_of_v<Statement, T>>>
inline auto makeStatement(Args&&... args) -> StatementPtr {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args,
         typename = std::enable_if_t<std::is_base_of_v<Expression, T>>>
inline auto makeExpression(Args&&... args) -> ExpressionPtr {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// Statements

class VariableDeclaration : public Statement {
public:
    VariableDeclaration(SourceRange location ,Token name, ExpressionPtr& initializer)
        : Statement(location),
          name_(std::move(name)), 
          initializer_(std::move(initializer)) {}

    inline auto name() const -> const Token& {
        return name_;
    }

    inline auto initializer() const -> const ExpressionPtr& {
        return initializer_;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitVariableDeclaration(*this);
    }

private:
    Token name_;
    ExpressionPtr initializer_;
};

class FunctionDeclaration : public Statement {
public:
    FunctionDeclaration(SourceRange location,
                        Token name, 
                        std::vector<Token>& params,
                        StatementPtr& body)
        : Statement(location),
          name_(std::move(name)),
          body_(std::move(body)),
          parameters_(std::move(params)) {}

    inline auto name() const -> const Token& {
        return name_;
    }

    inline auto params() const -> const std::vector<Token>& {
        return parameters_;
    }

    inline auto body() const -> const StatementPtr& {
        return body_;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitFunctionDeclaration(*this);
    }

private:
    Token name_;
    StatementPtr body_;
    std::vector<Token> parameters_;
};

class Block : public Statement {
public:
    Block(SourceRange location, std::vector<StatementPtr>& stmts)
        : Statement(location), statements_(std::move(stmts)) {} 
    
    auto statements() const -> const std::vector<StatementPtr>& {
        return statements_;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitBlock(*this);
    }

private:
    std::vector<StatementPtr> statements_;
};

class WhileStatement : public Statement {
public:
    WhileStatement(SourceRange location,
                ExpressionPtr& condition,
                StatementPtr& body)
        : Statement(location),
          condition_(std::move(condition)),
          body_(std::move(body)) {}

    inline auto condition() const -> const ExpressionPtr& {
        return condition_;
    }

    inline auto body() const -> const StatementPtr& {
        return body_;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitWhileStatement(*this);
    }

private:
    ExpressionPtr condition_;
    StatementPtr body_;
};

class IfStatement : public Statement {
public:
    IfStatement(SourceRange location,
                ExpressionPtr& condition,
                StatementPtr& thenBranch,
                StatementPtr& elseBranch)
        : Statement(location),
          condition_(std::move(condition)),
          thenBranch_(std::move(thenBranch)),
          elseBranch_(std::move(elseBranch)) {}

    inline auto condition() const -> const ExpressionPtr& {
        return condition_;
    }

    inline auto thenBranch() const -> const StatementPtr& {
        return thenBranch_;
    }

    inline auto elseBranch() const -> const StatementPtr& {
        return elseBranch_;
    }

    auto haveElseBranch() const -> bool {
        return elseBranch_ != nullptr;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitIfStatement(*this);
    }

private:
    ExpressionPtr condition_;
    StatementPtr thenBranch_;
    StatementPtr elseBranch_;
};

class ExpressionStatement : public Statement {
public:
    ExpressionStatement(SourceRange location, ExpressionPtr& expr)
        : Statement(location), expression_(std::move(expr)) {} ;

    inline auto expression() const -> const ExpressionPtr& {
        return expression_;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitExpressionStatement(*this);
    }

private:
    ExpressionPtr expression_;
};

class ContinueStatement : public Statement {
public:
    ContinueStatement(SourceRange location)
        : Statement(location) {};

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitContinueStatement(*this);
    }
};

class BreakStatement : public Statement {
public:
    BreakStatement(SourceRange location)
        : Statement(location) {} ;

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitBreakStatement(*this);
    }
};

class ReturnStatement : public Statement {
public:
    ReturnStatement(SourceRange location, ExpressionPtr& expr)
        : Statement(location), expression_(std::move(expr)) {} ;

    inline auto expression() const -> const ExpressionPtr& {
        return expression_;
    }

    inline auto haveExpression() const -> bool {
        return expression_ != nullptr;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitReturnStatement(*this);
    }

private:
    ExpressionPtr expression_;
};

class PrintStatement : public Statement {
public:
    PrintStatement(SourceRange location, ExpressionPtr& expr)
        : Statement(location), expression_(std::move(expr)) {} ;

    inline auto expression() const -> const ExpressionPtr& {
        return expression_;
    }

    inline auto haveExpression() const -> bool {
        return expression_ != nullptr;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitPrintStatement(*this);
    }

private:
    ExpressionPtr expression_;
};

// Expressions

class AssignmentExpression : public Expression {
public:
    AssignmentExpression(SourceRange location, 
                         Token name,
                         ExpressionPtr& value)
        : Expression(location),
          name_(std::move(name)),
          value_(std::move(value)) {}


    inline auto name() const -> const Token& {
        return name_;
    }

    inline auto value() const -> const ExpressionPtr& {
        return value_;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitAssignmentExpression(*this);
    }

private:
    Token name_;
    ExpressionPtr value_;
};

class BinaryExpression : public Expression {
public:
    BinaryExpression(SourceRange location,
                     Token op, 
                     ExpressionPtr& left, 
                     ExpressionPtr& right)
        : Expression(location),
          operator_(std::move(op)),
          left_(std::move(left)),
          right_(std::move(right)) {}

    inline auto op() const -> const Token& {
        return operator_;
    }

    inline auto right() const -> const ExpressionPtr& {
        return right_;
    }

    inline auto left() const -> const ExpressionPtr& {
        return left_;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitBinaryExpression(*this);
    }

private:
    Token operator_;
    ExpressionPtr left_;
    ExpressionPtr right_;
};

class UnaryExpression : public Expression {
public:
    UnaryExpression(SourceRange location, Token op, ExpressionPtr& expression)
        : Expression(location),
          operator_(std::move(op)),
          right_(std::move(expression)) {}
          
    inline auto op() const -> const Token& {
        return operator_;
    }

    inline auto right() const -> const ExpressionPtr& {
        return right_;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitUnaryExpression(*this);
    }

private:
    Token operator_;
    ExpressionPtr right_;
};

class CallExpression : public Expression {
public:
    CallExpression(SourceRange location, 
                   ExpressionPtr& callee, 
                   std::vector<ExpressionPtr>& args)
        : Expression(location),
          callee_(std::move(callee)),
          arguments_(std::move(args)) {}


    inline auto callee() const -> const ExpressionPtr& {
        return callee_;
    }

    inline auto arguments() const -> const std::vector<ExpressionPtr>& {
        return arguments_;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitCallExpression(*this);
    }

private:
    ExpressionPtr callee_;
    std::vector<ExpressionPtr> arguments_;
};


class GroupingExpression : public Expression {
public:
    GroupingExpression(SourceRange location, ExpressionPtr& expression)
        : Expression(location),
          expression_(std::move(expression)) {}
    
    inline auto expression() const -> const ExpressionPtr& {
        return expression_;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitGroupingExpression(*this);
    }

private:
    ExpressionPtr expression_;
};

class VariableExpression : public Expression {
public:
    VariableExpression(SourceRange location, Token name)
        : Expression(location), name_(name) {}

    auto inline name() const -> Token {
        return name_;
    }

    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitVariableExpression(*this);
    }
    
private:
    Token name_;
};

class LiteralExpression : public Expression {
    enum class LiteralType : std::uint8_t {
        String,
        Number,
        Boolean,
        Nil
    };

public:
    constexpr LiteralExpression(SourceRange location)
        : Expression(location) {}

    constexpr LiteralExpression(SourceRange location, double number)
        : Expression(location),
          type_(LiteralType::Number),
          data_(number) { }
          
    LiteralExpression(SourceRange location, std::string_view sv)
        : Expression(location),
          type_(LiteralType::String),
          data_{std::string(sv)} {} 

    constexpr LiteralExpression(SourceRange location, bool boolean)
        : Expression(location),
          type_(LiteralType::Boolean),
          data_(boolean) {}

    constexpr auto isBoolean() const -> bool {
        return type_ == LiteralType::Boolean;
    }

    constexpr auto isString() const -> bool {
        return type_ == LiteralType::String;
    }

    constexpr auto isNumber() const -> bool {
        return type_ == LiteralType::Number;
    }

    constexpr auto isNil() const -> bool {
        return type_ == LiteralType::Nil;
    }

    constexpr auto asBoolean() const -> bool {
        return std::get<bool>(data_);
    }

    constexpr auto asString() const -> const std::string& {
        return std::get<std::string>(data_);
    }

    constexpr auto asNumber() const -> double {
        return std::get<double>(data_);
    }


    inline auto accept(AstVisitor& visitor) -> void {
        visitor.visitLiteralExpression(*this);
    }

private:
    LiteralType type_ = LiteralType::Nil;
    std::variant<std::monostate, double, bool, std::string> data_;
};

namespace printer {

class AstPrettyPrinter : private AstVisitor {
public:
    explicit AstPrettyPrinter(std::ostream& stream, int indentSize = 4)
        : stream_(stream), indentSize_(indentSize) {}

    auto print(const std::vector<StatementPtr>& program) -> void;

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

    inline auto indent() -> void {
        level_++;
    }

    inline auto dedent() -> void {
        if(level_ > 0) level_--;
    }

    inline auto tab() const -> std::string {
       return std::string(indentSize_ * level_, ' ');
    }

private:
    std::ostream& stream_;
    int indentSize_;
    
    int level_ = 0;
};

}

}


#endif
