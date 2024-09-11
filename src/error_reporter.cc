#include <sstream>
#include <iomanip>
#include <cmath>

#include "../include/error_reporter.h"

namespace scriptlang::error {

auto BasicErrorReporter::error(const std::string& message, SourceRange location) -> void {

    const auto& [start, end] = location;

    std::stringstream sstream;

    auto linesCount = (end.line - start.line) + 1;
    auto sourceStart = start.source.begin() + start.offset;
    auto sourceEnd = end.source.begin() + end.offset;

    sstream << "[Ln: " << end.line << ", Col: " << end.column << "] Error: " << message << '\n';

    const int spacing = (std::log10(end.line) + 1) + 4;

    for(auto line = start.line; line < start.line + linesCount; line++){
        sstream << std::setw(spacing) << line << " | ";
        while(sourceStart < sourceEnd && *sourceStart != '\n') {
            
            sstream << *sourceStart;
            sourceStart++;
        }
       
        sourceStart++;
        sstream << '\n';
    }

    sstream << '\n';
    errors_.emplace_back(sstream.str());
}


}
