#ifndef _DISASSEMBLER_H_
#define _DISASSEMBLER_H_

#include <ostream>
#include "vm.h"

namespace scriptlang::disassembler {

using scriptlang::runtime::Chunk;

class Disassembler final {
public:
    Disassembler(std::ostream& stream)
        : stream_(stream) {}
    
    auto disassembleChunk(const char* name, Chunk& chunk) -> void;
    auto disassembleInstruction(Chunk& chunk, int offset) -> int;

private:
    auto simpleInstruction(const char* name, int offset) -> int;
    auto byteInstruction(const char* name, Chunk& chunk, int offset) -> int;
    auto jumpInstruction(const char* name, Chunk& chunk, int sign, int offset) -> int;
    auto constantInstruction(const char* name, Chunk& chunk, int offset) -> int;
    
private:
    std::ostream& stream_;
};

}

#endif
