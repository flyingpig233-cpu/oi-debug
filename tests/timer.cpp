#include "oi_debug/timer.hpp"
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>

const std::unique_ptr<int> &return_reference(const std::unique_ptr<int> &value) {
    return value;
}

std::unique_ptr<int> return_owned() { return std::make_unique<int>(7); }

void take_owned(std::unique_ptr<int> value) { assert(*value == 5); }

int return_number() { return 3; }

void throw_callable() { throw std::runtime_error("callable"); }

struct ThrowingResult {
    std::string to_string() const { throw std::runtime_error("result"); }
};

ThrowingResult throw_serializer() { return {}; }

int main() {
    const auto value = std::make_unique<int>(42);
    get_time_us(return_reference, value);
    assert(*value == 42);
    get_time_us(return_owned);
    get_time_ms(take_owned, std::make_unique<int>(5));
    get_time_s(return_number);
    bool callable_caught = false;
    try { get_time_us(throw_callable); }
    catch (const std::runtime_error &error) {
        callable_caught = std::string(error.what()) == "callable";
    }
    assert(callable_caught);
    bool serializer_caught = false;
    try { get_time_us(throw_serializer); }
    catch (const std::runtime_error &error) {
        serializer_caught = std::string(error.what()) == "result";
    }
    assert(serializer_caught);
    std::puts("timer checks passed");
}
