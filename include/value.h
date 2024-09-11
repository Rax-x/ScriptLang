#ifndef _VALUE_H_
#define _VALUE_H_

#include "objects.h"

#include <ostream>
#include <variant>
#include <string>

namespace scriptlang::runtime {

class Value final {
public:
    constexpr Value() : data_(std::monostate()) {}
    constexpr Value(bool boolean) : data_(boolean) {}
    constexpr Value(double number) : data_(number) {}

    Value(std::string string) : data_(std::move(string)) {}
    Value(ObjectFunction function) : data_(std::move(function)) {}

    auto isFalsey() const -> bool;

    constexpr auto isNil() const -> bool {
        return std::holds_alternative<std::monostate>(data_);
    }

    constexpr auto isNumber() const -> bool {
        return std::holds_alternative<double>(data_);
    }

    constexpr auto isBoolean() const -> bool {
        return std::holds_alternative<bool>(data_);
    }

    constexpr auto isString() const -> bool {
        return std::holds_alternative<std::string>(data_);
    }

    constexpr auto isFunction() const -> bool {
        return std::holds_alternative<ObjectFunction>(data_);
    }

    constexpr auto asNumber() const -> double {
        return std::get<double>(data_);
    }
    
    constexpr auto asBoolean() const -> bool {
        return std::get<bool>(data_);
    }

    constexpr auto asString() const -> const std::string& {
        return std::get<std::string>(data_);
    }

    constexpr auto asFunction() -> ObjectFunction& {
        return std::get<ObjectFunction>(data_);
    }

    constexpr auto isCallable() const -> bool {
        return isFunction();
    }

    inline auto operator==(const Value& rhs) -> bool {
        return data_ == rhs.data_;
    }

    friend auto operator<<(std::ostream& out, const Value& value) -> std::ostream&;

private:
    std::variant<bool, double, std::string, ObjectFunction, std::monostate> data_;
};

auto operator<<(std::ostream& out, const Value& value) -> std::ostream&;


}


#endif
