#include "../include/lexer.h"

#include <cctype>
#include <iterator>

namespace scriptlang::lexer {

Lexer::Lexer(std::string_view source) 
    : source_(source), 
      curr_(source_.begin()),
      start_(source_.begin()) {

    keywords_ = {
        {"let", TokenType::LetKeyword},
        {"defun", TokenType::DefunKeyword},
        {"if", TokenType::IfKeyword},
        {"else", TokenType::ElseKeyword},
        {"while", TokenType::WhileKeyword},
        {"continue", TokenType::ContinueKeyword},
        {"break", TokenType::BreakKeyword},
        {"return", TokenType::ReturnKeyword},
        {"print", TokenType::PrintKeyword},
        {"or", TokenType::OrKeyword},
        {"and", TokenType::AndKeyword},
        {"not", TokenType::NotKeyword},
        {"true", TokenType::TrueKeyword},
        {"false", TokenType::FalseKeyword},
        {"nil", TokenType::NilKeyword},
    };

}

auto Lexer::next() -> Token {

    skipWhiteSpaces();

    start_ = curr_;
    tokenStartPosition_ = getCurrentPosition();

    char c = advance();

    switch(c){
        case '\0':
            hasNext_ = false;
            return makeToken(TokenType::Eof);            
        case '+':
            return makeToken(TokenType::Plus);
        case '-':
            return makeToken(TokenType::Minus);
        case '*':
            return makeToken(match('*') 
                             ? TokenType::Exponent 
                             : TokenType::Star);
        case '/':
            return makeToken(TokenType::Slash);
        case '>':
            return makeToken(match('=') 
                             ? TokenType::GreaterEqual 
                             : TokenType::Greater);
        case '<':
            return makeToken(match('=') 
                             ? TokenType::LessEqual 
                             : TokenType::Less);
        case '=':
            return makeToken(match('=') 
                             ? TokenType::Equal 
                             : TokenType::Assign);
        case '!':
            return makeToken(match('=') 
                             ? TokenType::NotEqual 
                             : TokenType::Unknown);
        case '.':
            return makeToken(TokenType::Dot);
        case ',':
            return makeToken(TokenType::Comma);
        case ';':
            return makeToken(TokenType::Semicolon);
        case '(':
            return makeToken(TokenType::LeftParen);
        case ')':
            return makeToken(TokenType::RightParen);
        case '{':
            return makeToken(TokenType::LeftBrace);
        case '}':
            return makeToken(TokenType::RightBrace);
        case '"':
            return stringLiteral();
        default:

            if(std::isdigit(c)) return numberLiteral();

            if(std::isalpha(c) || c == '_') {
                return identifier();
            }

            break;
    }
    
    return makeToken(TokenType::Unknown);
}

auto Lexer::hasNext() const -> bool {
    return hasNext_;
}

auto Lexer::stringLiteral() -> Token {

    while(!isAtEnd() && peek(0) != '"') advance();
    
    if(isAtEnd()) {
        // TODO: write error message for unterminated string literal.
        return makeToken(TokenType::Unknown);
    }

    advance();
    return makeToken(TokenType::StringLiteral);
}

auto Lexer::numberLiteral() -> Token {
    while(std::isdigit(peek(0)) && !isAtEnd()) advance();

    if(peek(0) == '.' && std::isdigit(peek(1))){
        advance();
        while(std::isdigit(peek(0)) && !isAtEnd()) advance();        
    }

    if(match('e') || match('E')){
        if(peek(0) == '-' || peek(0) == '+') advance();
        while(std::isdigit(peek(0)) && !isAtEnd()) advance();
    }

    return makeToken(TokenType::NumberLiteral);
}

auto Lexer::getIdentiferType() -> TokenType {

    std::string_view lexeme = 
        source_.substr(std::distance(source_.begin(), start_),
                       std::distance(start_, curr_));

    try {
        return keywords_.at(lexeme);
    }catch(...){
    }

    return TokenType::Identifier;
}

auto Lexer::isValidIdentiferCharacter(char c) const -> bool {
    return std::isalnum(c) || c == '-' || c == '_';
}

auto Lexer::identifier() -> Token {

    while(isValidIdentiferCharacter(peek(0)) && !isAtEnd()){
        advance();
    }

    return makeToken(getIdentiferType());
}

auto Lexer::advance() -> char {
    return !isAtEnd()
        ? (column_++, *curr_++)
        : '\0';
}

auto Lexer::peek(std::uint32_t pos) const -> char {
    return (curr_ + pos < source_.end())
        ? *(curr_ + pos)
        : '\0';
}

auto Lexer::match(char c) -> bool {
    return peek(0) == c
        ? (advance(), true)
        : false;
}

auto Lexer::isAtEnd() const -> bool {
    return curr_ >= source_.end();
}

auto Lexer::getCurrentPosition() const -> SourcePosition {
    const std::uint32_t offset = std::distance(source_.begin(), curr_);
    return SourcePosition { source_, offset, line_, column_ };
}

auto Lexer::skipWhiteSpaces() -> void {

    while(!isAtEnd()){
        switch(peek(0)){
            case '#':
                while(!isAtEnd() && peek(0) != '\n') advance();
                if(isAtEnd()) break;
                [[fallthrough]];
            case '\n':
                line_++;
                column_ = 1;
                [[fallthrough]];
            case '\t':
                [[fallthrough]];
            case ' ':
                [[fallthrough]];
            case '\r':
                break;
            default:
                return;
        }

        advance();
    }
}

auto Lexer::makeToken(TokenType type) const -> Token {

    SourceRange range {tokenStartPosition_, getCurrentPosition()};

    std::string_view lexeme = 
        source_.substr(std::distance(source_.begin(), start_),
                       std::distance(start_, curr_));

    return Token { type, lexeme, range };
}

}
