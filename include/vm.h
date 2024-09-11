#ifndef _VM_H_
#define _VM_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "objects.h"
#include "value.h"
#include "chunk.h"
#include "types.h"

namespace scriptlang::runtime {

using namespace types;

enum class InterpreterResult {
    Success,
    RuntimeError,
};

class VM {

    struct CallFrame {
        ObjectFunction* function;

        std::uint32_t ip;
        Value* slots;
    };

public:

    static constexpr int CALL_FRAMES = 64;
    static constexpr int STACK_SIZE = CALL_FRAMES * BYTE_MAX;

    VM() {
        resetStack();
    }

    auto execute(ObjectFunction* function) -> InterpreterResult;

private:

    auto run() -> InterpreterResult;
    
    auto call(ObjectFunction* function, int argc) -> bool;
    auto callValue(Value& value, int argc) -> bool;

    auto resetStack() -> void;

    template<typename... Args>
    auto runtimeError(const char* message, Args&&... args) -> void;
    
    inline auto currentFrame() -> CallFrame* {
        return &frames_[frameCount_ - 1];
    }

    inline auto readByte() -> Byte {
        return currentFrame()->function->chunk[currentFrame()->ip++];
    }

    inline auto push(Value value) -> void {
        *stackTop_ = std::move(value);
        stackTop_++;
    }

    constexpr auto peek(int distance = 0) -> Value& {
        return stackTop_[-1 - distance];
    }

    inline auto pop() -> Value {
        return *(--stackTop_);
    }

private:

    CallFrame frames_[CALL_FRAMES];
    int frameCount_;

    Value stack_[STACK_SIZE] = {};
    Value* stackTop_ = stack_;

    std::unordered_map<std::string, Value> globals_;
};

}


#endif
