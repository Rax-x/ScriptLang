#ifndef _TYPES_H_
#define _TYPES_H_

#include <cstdint>
#include <limits>

namespace scriptlang::types {

    using Byte = std::uint8_t;
    constexpr Byte BYTE_MAX = std::numeric_limits<Byte>::max();

    using Short = std::uint16_t;
    constexpr Short SHORT_MAX = std::numeric_limits<Short>::max();
}

#endif
