#ifndef NO_STD_TYPES_HPP
#define NO_STD_TYPES_HPP

namespace nostd 
{
  /* === PRIMITIVES === */
  using byte = unsigned char;
  using error_t = long; // 0 success, negative error
  using res_t   = long; // positive result, negative error

  using i8  = signed char;
  using u8  = unsigned char;

  using i16 = signed short;
  using u16 = unsigned short;

  using i32 = signed int;
  using u32 = unsigned int;

  using i64 = signed long long;
  using u64 = unsigned long long;

  using f32 = float;
  using f64 = double;

  /* === EXTENDED === */
#ifdef __SIZEOF_INT128__
  using i128 = __int128;
  using u128 = unsigned __int128;
#endif // __SIZEOF_INT128__

#ifdef __FLOAT128__
  using f128 = __float128;
#endif // __FLOAT128__

  /* === CHARACTER === */
  using c8  = unsigned char;   // UTF-8
  using c16 = __CHAR16_TYPE__; // UTF-16, 2 bytes
  using c32 = __CHAR32_TYPE__; // UTF-32, 4 bytes

  /* === SIZE === */
  using usize = decltype(sizeof(0));
  using isize = decltype((char*)0 - (char*)0);
  using off_t = usize;

  /* === LIMITS === */
  inline constexpr i8    I8_MIN    = -128;
  inline constexpr i8    I8_MAX    =  127;
  inline constexpr i16   I16_MIN   = -32768;
  inline constexpr i16   I16_MAX   =  32767;
  inline constexpr i32   I32_MIN   = static_cast<i32>(1u << 31);
  inline constexpr i32   I32_MAX   = 0x7FFFFFFF;
  inline constexpr i64   I64_MIN   = static_cast<i64>(1ull << 63);
  inline constexpr i64   I64_MAX   = 0x7FFFFFFFFFFFFFFF;

  inline constexpr u8    U8_MAX    = ~u8{0};
  inline constexpr u16   U16_MAX   = ~u16{0};
  inline constexpr u32   U32_MAX   = ~u32{0};
  inline constexpr u64   U64_MAX   = ~u64{0};
  inline constexpr usize USIZE_MAX = ~usize{0};

#ifdef __SIZEOF_INT128__
  inline constexpr u128  U128_MAX  = ~u128{0};
#endif // __SIZEOF_INT128__

  inline constexpr f32 F32_MAX = __FLT_MAX__;
  inline constexpr f32 F32_MIN = __FLT_MIN__;
  inline constexpr f32 F32_LOW = -__FLT_MAX__;
  inline constexpr f64 F64_MAX = __DBL_MAX__;
  inline constexpr f64 F64_MIN = __DBL_MIN__;
  inline constexpr f64 F64_LOW = -__DBL_MAX__;

  /* === SENTINEL === */
  inline constexpr u8    npos8  = static_cast<u8>(-1);
  inline constexpr u16   npos16 = static_cast<u16>(-1);
  inline constexpr u32   npos32 = static_cast<u32>(-1);
  inline constexpr u64   npos64 = static_cast<u64>(-1);
  inline constexpr usize npos   = static_cast<usize>(-1);

#ifdef __SIZEOF_INT128__
  inline constexpr u128 npos128 = static_cast<u128>(-1);
#endif // __SIZEOF_INT128__

} // namespace nostd

#endif // NO_STD_TYPES_HPP
