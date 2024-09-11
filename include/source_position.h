#ifndef _SOURCE_POSITION_H_
#define _SOURCE_POSITION_H_

#include <cstdint>
#include <ostream>
#include <string_view>

namespace scriptlang::lexer {

struct SourcePosition final {
    
    std::string_view source;

    std::uint32_t offset;
    std::uint32_t line;
    std::uint32_t column;
};

struct SourceRange final {

    SourceRange() = default;
    constexpr explicit SourceRange(SourcePosition start, SourcePosition end)
        : start(start), end(end) {}

    SourcePosition start;
    SourcePosition end;
};

auto operator<<(std::ostream& stream, SourcePosition position) -> std::ostream&;
auto operator<<(std::ostream& stream, SourceRange range) -> std::ostream&;

}

#endif
