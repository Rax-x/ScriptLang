#include "../include/ast.h"

#include <ios>

namespace scriptlang::ast::printer {

auto AstPrettyPrinter::print(const std::vector<StatementPtr>& program) -> void {
    for(const auto& stmt : program) {
        stmt->accept(*this);
        stream_ << tab() << "\n\n";
    }
}

auto AstPrettyPrinter::visitVariableDeclaration(const VariableDeclaration& decl) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className << ":\n";

    indent();
    stream_ << tab();
    decl.initializer()->accept(*this);
    dedent();

    stream_ << ">";
}

auto AstPrettyPrinter::visitFunctionDeclaration(const FunctionDeclaration& decl) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className << ":\n";
    
    indent();
    stream_ << tab();
    
    decl.body()->accept(*this);
    
    dedent();
    stream_ << ">";
}

auto AstPrettyPrinter::visitBlock(const Block& block) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className << ":";
    indent();

    auto it = block.statements().begin();
    const auto end = block.statements().end();

    indent();
    do{
        stream_ << '\n' << tab();
        (*it)->accept(*this);
        it++;
    } while(it != end);

    dedent();
    stream_ << '>';
}

auto AstPrettyPrinter::visitWhileStatement(const WhileStatement& stmt) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className << ":\n";
    indent();
    
    stream_ << tab();
    stmt.condition()->accept(*this);
    stream_ << '\n';

    stream_ << tab();
    stmt.body()->accept(*this);

    dedent();
    stream_ << '>';
}

auto AstPrettyPrinter::visitIfStatement(const IfStatement& stmt) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className << ":\n";
    indent();
    
    stream_ << tab();
    stmt.condition()->accept(*this);
    stream_ << '\n';

    stream_ << tab();
    stmt.thenBranch()->accept(*this);

    if(stmt.haveElseBranch()){
        stream_ << '\n' << tab();
        stmt.elseBranch()->accept(*this);
    }

    dedent();
    stream_ << '>';
}

auto AstPrettyPrinter::visitExpressionStatement(const ExpressionStatement& stmt) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className << ":\n";
    
    indent();
    stream_ << tab();
    stmt.expression()->accept(*this);

    dedent();
    stream_ << '>';
}

auto AstPrettyPrinter::visitContinueStatement([[maybe_unused]] const ContinueStatement& stmt) -> void {
    const char* className = __func__ + 5;
    stream_ << '<' << className << '>';
}

auto AstPrettyPrinter::visitBreakStatement([[maybe_unused]] const BreakStatement& stmt) -> void {
    const char* className = __func__ + 5;
    stream_ << '<' << className << '>';
}

auto AstPrettyPrinter::visitReturnStatement(const ReturnStatement& stmt) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className;

    if(stmt.haveExpression()){
        indent();
        stream_ << ":\n" << tab();
        stmt.expression()->accept(*this);

        dedent();
    }

    stream_ << '>';
}

auto AstPrettyPrinter::visitPrintStatement(const PrintStatement& stmt) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className;

    indent();
    stream_ << ":\n" << tab();
    stmt.expression()->accept(*this);

    dedent();

    stream_ << '>';
}

auto AstPrettyPrinter::visitAssignmentExpression(const AssignmentExpression& expr) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className << ":\n";

    indent();
    stream_ << tab();
    expr.value()->accept(*this);

    dedent();
    stream_ << '>';
}

auto AstPrettyPrinter::visitBinaryExpression(const BinaryExpression& expr) -> void { 
    const char* className = __func__ + 5;

    stream_ << '<' << className << ": " << expr.op().lexeme << '\n';
    indent();

    stream_ << tab();
    expr.left()->accept(*this);

    stream_ << '\n' << tab();
    expr.right()->accept(*this);

    dedent();
    stream_ << '>';
}

auto AstPrettyPrinter::visitUnaryExpression(const UnaryExpression& expr) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className << ": " << expr.op().lexeme << '\n';
    indent();

    stream_ << tab();
    expr.right()->accept(*this);

    dedent();
    stream_ << '>';
}

auto AstPrettyPrinter::visitCallExpression(const CallExpression& expr) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className << ":\n";
    
    indent();
    stream_ << tab();
    expr.callee()->accept(*this);
    
    indent();
    auto it = expr.arguments().begin();
    const auto end = expr.arguments().end();

    do{
        stream_ << '\n' << tab();
        (*it)->accept(*this);
        it++;
    } while(it != end);

    dedent();
    dedent();
    stream_ << '>';
}

auto AstPrettyPrinter::visitGroupingExpression(const GroupingExpression& expr) -> void {
    const char* className = __func__ + 5;
    stream_ << '<' << className << ":\n";

    indent();
    stream_ << tab();

    expr.expression()->accept(*this);

    dedent();
    stream_ << ">";
}

auto AstPrettyPrinter::visitVariableExpression(const VariableExpression& expr) -> void {
    const char* className = __func__ + 5;
    stream_ << '<' << className << ": " << expr.name().lexeme << ">";
}

auto AstPrettyPrinter::visitLiteralExpression(const LiteralExpression& expr) -> void { 
    const char* className = __func__ + 5;
    stream_ << '<' << className << ": ";

    if(expr.isBoolean()){
        stream_ << std::boolalpha << expr.asBoolean() << std::noboolalpha;
    } else if(expr.isNumber()){
        stream_ << expr.asNumber();
    } else if(expr.isString()){
        stream_ << expr.asString();
    } else if(expr.isNil()){
        stream_ << "nil";
    }

    stream_ << ">";
}
    
}

