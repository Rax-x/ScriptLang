#include "../include/disassembler.h"

namespace scriptlang::disassembler {


using scriptlang::runtime::OpCode;
using scriptlang::runtime::Byte;

auto Disassembler::disassembleChunk(const char* name, Chunk& chunk) -> void {
    std::uint32_t offset = 0;

    stream_ << "======= " << name << " =======\n";
    while(offset < chunk.size()){
        offset = disassembleInstruction(chunk, offset);
    }

    stream_ << "======= end " << name << " =======\n";
}

auto Disassembler::disassembleInstruction(Chunk& chunk, int offset) -> int {
    
    const OpCode opcode = static_cast<OpCode>(chunk[offset]);

    stream_ << offset << " |\t";

    switch(opcode){
        case OpCode::PushConstant:
            return constantInstruction("OpCode::PushConstant", chunk, offset);
        case OpCode::Pop:
            return simpleInstruction("OpCode::Pop", offset);
        case OpCode::Add:
            return simpleInstruction("OpCode::Add", offset);
        case OpCode::Sub:
            return simpleInstruction("OpCode::Sub", offset);
        case OpCode::Div:
            return simpleInstruction("OpCode::Div", offset);
        case OpCode::Mult:
            return simpleInstruction("OpCode::Mult", offset);
        case OpCode::Less:
            return simpleInstruction("OpCode::Less", offset);
        case OpCode::Greater:
            return simpleInstruction("OpCode::Greater", offset);
        case OpCode::Equal:
            return simpleInstruction("OpCode::Equal", offset);
        case OpCode::Pow:
            return simpleInstruction("OpCode::Pow", offset);
        case OpCode::Not:
            return simpleInstruction("OpCode::Not", offset);
        case OpCode::Negate:
            return simpleInstruction("OpCode::Negate", offset);
        case OpCode::Print:
            return simpleInstruction("OpCode::Print", offset);
        case OpCode::JumpIfFalse:
            return jumpInstruction("OpCode::JumpIfFalse", chunk, 1, offset);
        case OpCode::Jump:
            return jumpInstruction("OpCode::Jump", chunk, 1, offset);
        case OpCode::Loop:
            return jumpInstruction("OpCode::Loop", chunk, -1, offset);
        case OpCode::GetLocal:
            return byteInstruction("OpCode::GetLocal", chunk, offset);
        case OpCode::SetLocal:
            return byteInstruction("OpCode::SetLocal", chunk, offset);
        case OpCode::DefineGlobal:
            return constantInstruction("OpCode::DefineGlobal", chunk, offset);
        case OpCode::GetGlobal:
            return constantInstruction("OpCode::GetGlobal", chunk, offset);
        case OpCode::SetGlobal:
            return constantInstruction("OpCode::SetGlobal", chunk, offset);
        case OpCode::Call:
            return byteInstruction("OpCode::Call", chunk, offset);
        case OpCode::Return:
            return simpleInstruction("OpCode::Return", offset);
        case OpCode::True:
            return simpleInstruction("OpCode::True", offset);
        case OpCode::False:
            return simpleInstruction("OpCode::False", offset);
        case OpCode::Nil:
            return simpleInstruction("OpCode::Nil", offset);
        default:
            stream_ << "Unknown opcode '" << opcode << "'.\n";
            break;
    }

    return offset + 1;
}

auto Disassembler::simpleInstruction(const char* name, int offset) -> int {
    stream_ << name << '\n';
    return offset + 1;
}

auto Disassembler::byteInstruction(const char* name, Chunk& chunk, int offset) -> int {
    const int byte = chunk[offset + 1];
    stream_ << name << '\t' << byte << '\n';

    return offset + 2;
}

auto Disassembler::jumpInstruction(const char* name, Chunk& chunk, int sign, int offset) -> int {
    
    std::uint16_t jump = static_cast<std::uint16_t>((chunk[offset + 1] << 8) | chunk[offset + 2]);
    stream_ << name << '\t' << offset << " -> " << (offset + 3) + (sign*jump) << '\n';

    return offset + 3;
}
auto Disassembler::constantInstruction(const char* name, Chunk& chunk, int offset) -> int {

    const std::uint32_t index = chunk[offset + 1];

    stream_ << name << "\tIndex: " << index << " (" << chunk.getConstant(index) << ')'  << '\n';
    return offset + 2;
}


}
