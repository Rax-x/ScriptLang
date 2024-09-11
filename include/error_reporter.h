#ifndef _ERROR_REPORTER_H_
#define _ERROR_REPORTER_H_

#include <string>
#include <vector>
#include <memory>

#include "source_position.h"
#include "utils.h"

namespace scriptlang::error {

using scriptlang::lexer::SourceRange;
using scriptlang::utils::format;

class ErrorReporter {
public:
    virtual ~ErrorReporter() = default;

    constexpr auto hadError() const -> bool {
        return hadError_;
    }

    constexpr auto reset() -> void {
        hadError_ = false;
    }
    
    template<typename... Args>
    inline auto error(SourceRange location, const char* fmt, Args&&... args) -> void{
        hadError_ = true;
        error(format(fmt, std::forward<Args>(args)...), location);
    }

    virtual auto error(const std::string& message, SourceRange location) -> void = 0;

private:
    bool hadError_ = false;
};

class BasicErrorReporter : public ErrorReporter {
public:
    BasicErrorReporter() = default;

    inline auto errors() const -> const std::vector<std::string>& {
        return errors_;
    }

    auto error(const std::string& message, SourceRange location) -> void;
  
private:
    std::vector<std::string> errors_;

};

}


#endif
