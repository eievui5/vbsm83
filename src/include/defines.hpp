#pragma once

// Global definitions
// This file is automatically included in every translation unit and aliases
// common type names to make them more terse.

#include <cstdint>
#include <memory>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

template <typename T> using uptr = std::unique_ptr<T>;
