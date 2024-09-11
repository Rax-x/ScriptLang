#ifndef _OBJECTS_H_
#define _OBJECTS_H_

#include "chunk.h"

#include <string>

namespace scriptlang::runtime {

struct ObjectFunction {

    ObjectFunction() = default;

    std::string name;
    int arity;

    Chunk chunk;

    auto operator==([[maybe_unused]] const ObjectFunction& rhs) const -> bool {
        return false;
    }
};



}

#endif
