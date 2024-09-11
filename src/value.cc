#include "../include/value.h"

namespace scriptlang::runtime {

struct OutputVisitor final {
    OutputVisitor(std::ostream& stream)
        : stream_(stream) {}
    
    auto operator()([[maybe_unused]] std::monostate value) -> void {
        stream_ << "nil";
    }

    auto operator()(bool value) -> void {
        stream_ << std::boolalpha << value << std::noboolalpha;
    }

    auto operator()(double value) -> void {
        stream_ << value;
    }

    auto operator()(const std::string& value) -> void {
        stream_ << value;
    }

    auto operator()(const ObjectFunction& function) -> void {
        const char* name = !function.name.empty()
            ? function.name.c_str()
            : "<script>";

        stream_ << "<function '" << name << "' (param count: " << function.arity << ") >";
    }

private:
    std::ostream& stream_;
};

auto operator<<(std::ostream& out, const Value& value) -> std::ostream& {
    std::visit(OutputVisitor(out), value.data_);
    return out;
}


struct FalsinessVisitor final {
    constexpr auto operator()(bool b) const -> bool { 
        return !b; 
    }

    constexpr auto operator()([[maybe_unused]] const std::monostate n) const -> bool{ 
        return true; 
    }

    constexpr auto operator()(const double number) const -> bool{ 
        return number == 0; 
    }
    
    template <typename T>
    constexpr auto operator()([[maybe_unused]] const T& value) const -> bool { 
        return false; 
    }
};

auto Value::isFalsey() const -> bool {
    return std::visit(FalsinessVisitor(), data_);
}

}
