#ifndef OI_DEBUG_TEST_REGISTERED_HPP
#define OI_DEBUG_TEST_REGISTERED_HPP

#include "oi_debug/fields.hpp"
#include <array>

namespace fixture {
struct Record {
    int value;
    std::array<int, 2> samples;
};
} // namespace fixture

OI_DEBUG_FIELDS(fixture::Record, value, samples);

#endif // OI_DEBUG_TEST_REGISTERED_HPP
