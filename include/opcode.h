#ifndef _OPCODE_H_
#define _OPCODE_H_

#include "types.h"

namespace scriptlang::runtime {

using types::Byte;

enum OpCode : Byte {
    PushConstant,
    Pop,
    Add,
    Sub,
    Div,
    Mult,
    Pow,
    Less,
    Greater,
    Equal,
    Not,
    Negate,
    Print,
    JumpIfFalse,
    Jump,
    Loop,
    GetLocal,
    SetLocal,
    DefineGlobal,
    GetGlobal,
    SetGlobal,
    Call,
    Return,
    True,
    False,
    Nil,
};


}

#endif
