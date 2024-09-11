#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <cstdint>
#include <ostream>
#include <string_view>

#include "source_position.h"

namespace scriptlang::lexer {

enum class TokenType : std::uint8_t {
    Unknown,

    // Keywords
    LetKeyword,
    DefunKeyword,
    IfKeyword,
    ElseKeyword,
    WhileKeyword,
    ContinueKeyword,
    BreakKeyword,
    ReturnKeyword,
    PrintKeyword,
    OrKeyword,
    AndKeyword,
    NotKeyword,
    TrueKeyword,
    FalseKeyword,
    NilKeyword,

    // Operators
    Plus,
    Minus,
    Star,
    Slash,
    Exponent,
    Greater,
    Less,
    GreaterEqual,
    LessEqual,
    Equal,
    NotEqual,
    Assign,
    
    // Symbols
    Dot,
    Comma,
    Semicolon,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,

    // Literals
    Identifier,
    StringLiteral,
    NumberLiteral,
    
    Eof
};

struct Token final {
    TokenType type;
    std::string_view lexeme;
    
    SourceRange position;

    friend auto operator<<(std::ostream& out, const Token& token) -> std::ostream&;
};

}


#endif
