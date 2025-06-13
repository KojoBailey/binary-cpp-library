#ifndef KOJO_BINARY_LIB
#define KOJO_BINARY_LIB

#include <bit>
#include <cstdint>

/* DEBUGGING */
// #include <iostream>
// #include <format>

#ifdef USE_BINARY_TYPES
    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;
    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;
    using std::string;
#endif // USE_BINARY_TYPES

/** @note Not `KojoBailey` like on GitHub since that's a bit tedious. */
namespace kojo {

namespace binary::types {
    using u8  = std::uint8_t;   // 8-bit unsigned   (0 - 255)
    using u16 = std::uint16_t;  // 16-bit unsigned  (0 - 65,535)
    using u32 = std::uint32_t;  // 32-bit unsigned  (0 - 4,294,967,295)
    using u64 = std::uint64_t;  // 64-bit unsigned  (0 - 18,446,744,073,709,551,615)
    using i8  = std::int8_t;    // 8-bit signed     (-128 - 127)
    using i16 = std::int16_t;   // 16-bit signed    (-32,768 - 32,767)
    using i32 = std::int32_t;   // 32-bit signed    (-2,147,483,648 - 2,147,483,647)
    using i64 = std::int64_t;   // 64-bit signed    (-9,223,372,036,854,775,808 - 9,223,372,036,854,775,807)
}

}

#endif // KOJO_BINARY_LIB