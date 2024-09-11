#include "../include/token.h"
#include <type_traits>

static constexpr const char* tokenNamesTable[] = {
    "Unknown",
    "LetKeyword",
    "DefunKeyword",
    "IfKeyword",
    "ElseKeyword",
    "ReturnKeyword",
    "PrintKeyword",
    "OrKeyword",
    "AndKeyword",
    "NotKeyword",
    "TrueKeyword",
    "FalseKeyword",
    "NullKeyword",
    "Plus",
    "Minus",
    "Star",
    "Slash",
    "Exponent",
    "Grater",
    "Less",
    "GraterEqual",
    "LessEqaul",
    "Equal",
    "NotEqaul",
    "Assign",
    "Dot",
    "Comma",
    "Semicolon",
    "LeftParen",
    "RightParen",
    "LeftBrace",
    "RightBrace",
    "Identifier",
    "StringLiteral",
    "NumberLiteral",
    "Eof"
};

namespace scriptlang::lexer {

namespace detail {

    template<typename Enum,
        typename = std::enable_if_t<std::is_enum_v<Enum>>>
    inline auto to_integer(Enum e) -> typename std::underlying_type_t<Enum> {
        return static_cast<typename std::underlying_type_t<Enum>>(e);
    }

}

auto operator<<(std::ostream& out, const Token& token) -> std::ostream& {
    const auto index = detail::to_integer(token.type);

    return out  << "Token(" << tokenNamesTable[index] << "):" 
        << '\'' << token.lexeme << '\''
        << "\nPosition: " << token.position
        << '\n';
}

}
