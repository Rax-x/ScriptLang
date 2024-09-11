#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <cstdio>
#include <type_traits>

namespace scriptlang::utils {

template<typename Base, typename Derived,
         typename = std::enable_if_t<std::is_base_of_v<Base, Derived>>>
constexpr auto instanceof(Base* ptr) -> bool {
    return dynamic_cast<Derived*>(ptr) != nullptr;
}

template<typename... Args>
auto format(const char* fmt, Args&&... args) -> std::string {

    const std::size_t bufferSize = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...) + 1;
    std::string buffer(bufferSize - 1, '\0');

    std::snprintf(buffer.data(), bufferSize, fmt, std::forward<Args>(args)...);

    return buffer;
}

}


#endif
