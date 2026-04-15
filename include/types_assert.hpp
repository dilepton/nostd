#ifndef NO_STD_TYPES_ASSERT_HPP
#define NO_STD_TYPES_ASSERT_HPP

#include "types.hpp"

namespace nostd
{
  /* === PRIMITIVES === */
  static_assert(sizeof(byte) == 1, "byte must be 1 byte");
  static_assert(sizeof(error_t) >= 4, "error_t must be at least 4 bytes");

  static_assert(sizeof(i8)  == 1, "i8 must be 1 byte");
  static_assert(sizeof(u8)  == 1, "u8 must be 1 byte");
  static_assert(i8(-1) < 0,       "i8 must be signed");
  static_assert(u8(-1) > 0,       "u8 must be unsigned");

  static_assert(sizeof(i16) == 2, "i16 must be 2 bytes");
  static_assert(sizeof(u16) == 2, "u16 must be 2 bytes");
  static_assert(i16(-1) < 0,      "i16 must be signed");
  static_assert(u16(-1) > 0,      "u16 must be unsigned");

  static_assert(sizeof(i32) == 4, "i32 must be 4 bytes");
  static_assert(sizeof(u32) == 4, "u32 must be 4 bytes");
  static_assert(i32(-1) < 0,      "i32 must be signed");
  static_assert(u32(-1) > 0,      "u32 must be unsigned");

  static_assert(sizeof(i64) == 8, "i64 must be 8 bytes");
  static_assert(sizeof(u64) == 8, "u64 must be 8 bytes");
  static_assert(i64(-1) < 0,      "i64 must be signed");
  static_assert(u64(-1) > 0,      "u64 must be unsigned");

  static_assert(sizeof(f32) == 4, "f32 must be 4 bytes");
  static_assert(sizeof(f64) == 8, "f64 must be 8 bytes");

  /* === EXTENDED === */
#ifdef __SIZEOF_INT128__
  static_assert(sizeof(i128) == 16, "i128 must be 16 bytes");
  static_assert(sizeof(u128) == 16, "u128 must be 16 bytes");
  static_assert(U128_MAX == ~u128{0}, "u128_max is invalid");
  static_assert(npos128  == U128_MAX, "npos128 is invalid");
#endif // __SIZEOF_INT128__

#ifdef __FLOAT128__
  static_assert(sizeof(f128) == 16, "f128 must be 16 bytes");
#endif

  /* === CHARACTER === */
  static_assert(sizeof(c8)  == 1, "c8 must be 1 byte");
  static_assert(sizeof(c16) == 2, "c16 must be 2 bytes");
  static_assert(sizeof(c32) == 4, "c32 must be 4 bytes");

  /* === SIZE === */
  static_assert(sizeof(usize) == sizeof(void*), "usize must match pointer size");
  static_assert(sizeof(isize) == sizeof(void*), "isize must match pointer size");

  /* === LIMITS === */
  static_assert(U8_MAX    == static_cast<u8>(0xFF), "u8_max is invalid");
  static_assert(U16_MAX   == static_cast<u16>(0xFFFF), "u16_max is invalid");
  static_assert(U32_MAX   == static_cast<u32>(0xFFFFFFFFu), "u32_max is invalid");
  static_assert(U64_MAX   == static_cast<u64>(0xFFFFFFFFFFFFFFFFull), "u64_max is invalid");
  static_assert(USIZE_MAX == ~usize{0}, "usize_max is invalid");

  /* === FLOATING POINT === */
  static_assert(F32_MAX > 0, "f32_max is invalid");
  static_assert(F64_MAX > 0, "f64_max is invalid");
  static_assert(F32_MIN > 0, "f32_min is invalid");
  static_assert(F64_MIN > 0, "f64_min is invalid");
  static_assert(F64_MAX > F32_MAX, "f64_max must be greater than f32_max");

  /* === SENTINEL === */
  static_assert(npos8  == U8_MAX,    "npos8 is invalid");
  static_assert(npos16 == U16_MAX,   "npos16 is invalid");
  static_assert(npos32 == U32_MAX,   "npos32 is invalid");
  static_assert(npos64 == U64_MAX,   "npos64 is invalid");
  static_assert(npos   == USIZE_MAX, "npos is invalid");

} // namespace nostd

#endif // NO_STD_TYPES_ASSERT_HPP