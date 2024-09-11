#ifndef _LEXER_H_
#define _LEXER_H_

#include "token.h"

#include <map>

namespace scriptlang::lexer {

class Lexer final {
public:
    Lexer(std::string_view source);

    auto next() -> Token;
    auto hasNext() const -> bool;

private:

    auto stringLiteral() -> Token;
    auto numberLiteral() -> Token;

    auto getIdentiferType() -> TokenType;
    auto isValidIdentiferCharacter(char c) const -> bool;
    auto identifier() -> Token;

    auto advance() -> char;
    auto peek(std::uint32_t pos = 0) const -> char;
    auto match(char c) -> bool;

    auto isAtEnd() const -> bool;

    auto getCurrentPosition() const -> SourcePosition;
    auto skipWhiteSpaces() -> void;

    auto makeToken(TokenType type) const -> Token;

private:

    std::string_view source_;
    bool hasNext_ = true;

    std::string_view::const_iterator curr_;
    std::string_view::const_iterator start_;

    std::uint32_t line_ = 1;
    std::uint32_t column_ = 0;

    std::uint32_t startLine_ = 1;
    std::uint32_t startColumn_ = 0;

    std::map<std::string_view, TokenType> keywords_;

    SourcePosition tokenStartPosition_;
};

}


#endif

