#include "../include/vm.h"
#include "../include/utils.h"
#include "../include/disassembler.h"

#include <cmath>
#include <iostream>

namespace scriptlang::runtime {

using scriptlang::disassembler::Disassembler;
using scriptlang::utils::format;

template<typename... Args>
auto VM::runtimeError(const char* message, Args&&... args) -> void {

    const CallFrame* frame = currentFrame();

    const std::uint32_t line = frame->function->chunk.getLine(frame->ip - 1);
    std::cout << "Runtime error [Ln: " << line << "] " 
              << format(message, std::forward<Args>(args)...)
              << '\n';

    for(int i = frameCount_ - 1; i >= 0; i--){
        const auto function = frames_[i].function;
        std::cout << "    in "  << *function << "\n";
    }

    resetStack();
}

auto VM::call(ObjectFunction* function, int argc) -> bool {
    
    if(frameCount_ == CALL_FRAMES){
        runtimeError("Stack overflow.");
        return false;
    }

    if(argc != function->arity) {
        runtimeError("Expect %d arguments, got %d.", function->arity, argc);
        return false;
    }

    CallFrame& frame = frames_[frameCount_++];

    frame.function = function;
    frame.ip = 0;
    frame.slots = stackTop_ - argc - 1;

    return true;
}

auto VM::callValue(Value& value, int argc) -> bool {
    if(!value.isCallable()){
        runtimeError("Can only call functions.");
        return false;
    }
     
    return call(&value.asFunction(), argc);
}

auto VM::execute(ObjectFunction* function) -> InterpreterResult {
   
    push(*function);
    call(function, 0);

    return run();
}

auto VM::run() -> InterpreterResult {

#ifdef DEBUG
    Disassembler disassembler(std::cout);
#endif

    CallFrame* frame = currentFrame();

    #define READ_CONSTANT() (frame->function->chunk.getConstant(readByte()))
    #define READ_SHORT() (static_cast<std::uint16_t>(((readByte() << 8) | readByte())))
    #define RUNTIME_ERROR(...) \
        runtimeError(__VA_ARGS__); \
        return InterpreterResult::RuntimeError

    #define BINARY_OPERATION(op) do {                   \
            auto b = pop();                             \
            auto a = pop();                             \
                                                        \
            if(!a.isNumber() || !b.isNumber()) {        \
                RUNTIME_ERROR("Expect two numbers.");   \
            }                                           \
                                                        \
            push(a.asNumber() op b.asNumber());         \
        } while(0)

    Byte instruction;
    while(frame->ip < frame->function->chunk.size()){

#ifdef DEBUG
        disassembler.disassembleInstruction(frame->function->chunk, frame->ip);
        std::cout << "    ";
        for(Value* it = stack_; it < stackTop_; it++){
            std::cout << '[' << *it << "] ";
        }

        std::cout << '\n';
#endif

        instruction = readByte();
        switch(instruction){
            case OpCode::PushConstant:
                push(READ_CONSTANT());
                break;
            case OpCode::Pop:
                pop();
                break;
            case OpCode::Add: {
                auto b = pop();
                auto a = pop();

                if(a.isNumber() && b.isNumber()){
                    push(a.asNumber() + b.asNumber());
                } else if(a.isString() && b.isString()){
                    push(a.asString() + b.asString());
                } else {
                    RUNTIME_ERROR("Expect two numbers or two strings.");   
                }

                break;
            }
            case OpCode::Sub:
                BINARY_OPERATION(-);
                break;
            case OpCode::Div:
                BINARY_OPERATION(/);
                break;
            case OpCode::Mult:
                BINARY_OPERATION(*);
                break;
            case OpCode::Less:
                BINARY_OPERATION(<);
                break;
            case OpCode::Greater:
                BINARY_OPERATION(>);
                break;
            case OpCode::Equal:
                push(pop() == pop());
                break;
            case OpCode::Pow: {
                auto exponent = pop();
                auto base = pop();

                if(!exponent.isNumber() || !base.isNumber()){
                    RUNTIME_ERROR("Expect two numbers.");
                }

                push(std::pow(base.asNumber(), exponent.asNumber()));
                break;
            }
            case OpCode::Not:
                push(pop().isFalsey());
                break;
            case OpCode::Negate:
                if(peek().isNumber()){
                    push(-pop().asNumber());
                } else {
                    RUNTIME_ERROR("Expect a number.");
                }
                
                break;
            case OpCode::Print:
                std::cout << pop() << '\n';
                break;
            case OpCode::JumpIfFalse: {
                std::uint16_t offset = READ_SHORT();

                if(peek().isFalsey()) {
                   frame->ip += offset;
                }

                break;
            }
            case OpCode::Jump: {
                std::uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OpCode::Loop: {
                std::uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            case OpCode::DefineGlobal: {
                auto& name = READ_CONSTANT().asString();

                if(globals_.find(name) != globals_.end()){
                    RUNTIME_ERROR("Global variable '%s' already defined.", name.c_str());
                }

                globals_[name] = pop();
                break;
            }
            case OpCode::GetGlobal: {
                auto& name = READ_CONSTANT().asString();

                if(globals_.find(name) == globals_.end()){
                    RUNTIME_ERROR("Undefined global variable '%s'.", name.c_str());
                }

                push(globals_[name]);
                break;
            }
            case OpCode::SetGlobal: {
                auto& name = READ_CONSTANT().asString();

                if(globals_.find(name) == globals_.end()){
                    RUNTIME_ERROR("Undefined global variable '%s'.", name.c_str());
                }

                globals_[name] = peek();
                break;
            }
            case OpCode::GetLocal: {
                const Byte slot = readByte();
                push(frame->slots[slot]);    
                break;
            }
            case OpCode::SetLocal: {
                const Byte slot = readByte();
                frame->slots[slot] = peek();
                break;
            }
            case OpCode::Call: {
                
                const Byte argc = readByte();

                if(!callValue(peek(argc), argc)){
                    return InterpreterResult::RuntimeError;
                }
                
                frame = &frames_[frameCount_ - 1];
                break;
            }
            case OpCode::Return: {

                auto returnValue = pop();
                frameCount_--;

                if(frameCount_ == 0){
                    pop();
                    return InterpreterResult::Success;
                }

                stackTop_ = frame->slots;
                push(std::move(returnValue));

                frame = &frames_[frameCount_ - 1];
                break;
            }
            case OpCode::True:
                push(true);
                break;
            case OpCode::False:
                push(false);
                break;
            case OpCode::Nil:
                push({});
                break;
            default:
                RUNTIME_ERROR("Unknow operation.");
        }
    }

    #undef READ_CONSTANT
    #undef READ_SHORT

    resetStack();

    return InterpreterResult::Success;
}

auto VM::resetStack() -> void {
    frameCount_ = 0;
    stackTop_ = stack_;
}

}
