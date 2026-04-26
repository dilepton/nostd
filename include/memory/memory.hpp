#ifndef NO_STD_MEMORY_HPP
#define NO_STD_MEMORY_HPP

#include "types.hpp"

namespace nostd
{
  /* ==================== ALIGN OPERATIONS ==================== */
  constexpr bool is_power_of_two(usize v) noexcept {
    return v != 0 && (v & (v - 1)) == 0;
  }

  constexpr usize align_up(usize val, usize align) noexcept {
    if (is_power_of_two(align))
      return (val + align - 1) & ~(align - 1);
    
    if (align == 0) return val;
    return ((val + align - 1) / align) * align;
  }

  inline byte* align_ptr(byte* p, usize align) noexcept {
    return reinterpret_cast<byte*>(align_up(reinterpret_cast<usize>(p), align));
  }

  /* ==================== BASIC OPERATIONS ==================== */
  inline void* memcpy(void* dst, const void* src, usize n) noexcept {
    return __builtin_memcpy(dst, src, n);
  }

  inline void* memmove(void* dst, const void* src, usize n) noexcept {
    return __builtin_memmove(dst, src, n);
  }

  inline void* memset(void* dst, int c, usize n) noexcept {
    return __builtin_memset(dst, c, n);
  }

  inline int memcmp(const void* a, const void* b, usize n) noexcept {
    return __builtin_memcmp(a, b, n);
  }

  inline void* memchr(const void* p, int c, usize n) noexcept {
    return __builtin_memchr(p, c, n);
  }

  /* ================== STRING OPERATIONS ================== */
  inline usize strlen(const char* s) noexcept {
    return __builtin_strlen(s);
  }

  inline int strcmp(const char* a, const char* b) noexcept {
    return __builtin_strcmp(a, b);
  }

  inline const char* strchr(const char* s, int c) noexcept {
    return __builtin_strchr(s, c);
  }

  /* ================== CACHE FEATURES ================== */
  inline void cache_load_l1(const byte* ptr) noexcept { __builtin_prefetch(ptr, 0, 3); }
  inline void cache_load_l2(const byte* ptr) noexcept { __builtin_prefetch(ptr, 0, 2); }
  inline void cache_load_l3(const byte* ptr) noexcept { __builtin_prefetch(ptr, 0, 1); }

  inline void cache_store_l1(const byte* ptr) noexcept { __builtin_prefetch(ptr, 1, 3); }
  inline void cache_store_l2(const byte* ptr) noexcept { __builtin_prefetch(ptr, 1, 2); }
  inline void cache_store_l3(const byte* ptr) noexcept { __builtin_prefetch(ptr, 1, 1); }

} // namespace nostd

#endif // NO_STD_MEMORY_HPP