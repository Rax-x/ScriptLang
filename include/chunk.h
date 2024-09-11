#ifndef _CHUNK_H_
#define _CHUNK_H_

#include "opcode.h"

#include <cstdint>
#include <vector>

namespace scriptlang::runtime {

struct LineInfo {
    std::uint32_t line;
    std::uint32_t offset;
};

// Forward
class Value;

class Chunk {
public:
    Chunk() = default;

    inline auto write(OpCode code, std::uint32_t line) -> void {
        write(static_cast<Byte>(code), line);
    }

    inline auto write(Byte byte, std::uint32_t line) -> void {
        code_.push_back(byte);

        if(lines_.size() > 0 && lines_.back().line == line) return;
        lines_.push_back({line, (std::uint32_t) code_.size() - 1});
    }

    inline auto size() const -> std::size_t {
        return code_.size();
    }

    inline auto operator[](std::uint32_t index) -> Byte& {
        return code_[index];
    }

    template<typename... Args>
    inline auto addConstant(Args&&... args) -> std::uint8_t {
        constants_.emplace_back(std::forward<Args>(args)...);
        return constants_.size() - 1;
    }

    inline auto getConstant(std::uint32_t index) -> const Value& {
        return constants_[index];
    }

    auto getLine(std::uint32_t instructionOffset) -> std::uint32_t {

        std::uint32_t start = 0;
        std::uint32_t end = lines_.size() - 1;
        std::uint32_t mid;

        while(true){
            mid = (start + end) / 2;

            if(instructionOffset < lines_[mid].offset){
                end = mid - 1;
            } else if(mid == lines_.size() - 1 || instructionOffset < lines_[mid+1].offset){
                return lines_[mid].line;
            } else {
                start = mid + 1;
            }
        }
    }

private:
    std::vector<Value> constants_;
    std::vector<Byte> code_;
    std::vector<LineInfo> lines_;
};

}


#endif
