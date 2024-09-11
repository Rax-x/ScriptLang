#include "../include/source_position.h"

namespace scriptlang::lexer {


auto operator<<(std::ostream& stream, SourcePosition position) -> std::ostream& {
    return stream << "(Offset: " << position.offset << ")"
           << "[Ln: " << position.line 
           << ", Col: " << position.column 
           << ']';
}

auto operator<<(std::ostream& stream, SourceRange range) -> std::ostream& {
    return stream << "<" << range.start << " - " << range.end << ">";
}

}
