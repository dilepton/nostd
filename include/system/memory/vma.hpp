#ifndef NO_STD_VMA_HPP
#define NO_STD_VMA_HPP

#include "types.hpp"

namespace nostd
{
  class VMA {
  public:
    VMA() = delete;
    ~VMA() = delete;

    static const usize PAGE_SIZE;

    static usize page_round(usize bytes) noexcept {
      return (bytes + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    }

    static byte* mmap(void* addr, usize len, long prot, long flags, long fd, long off) noexcept;
    static long  munmap(byte* p, usize len) noexcept;
    static byte* mremap(byte* p, usize old_sz, usize new_sz, long flags) noexcept;

    static long  mprotect(byte* p, usize len, long prot) noexcept;
    static long  madvise(byte* p, usize len, long advice) noexcept;

    static long  mlock(byte* p, usize len) noexcept;
    static long  munlock(byte* p, usize len) noexcept;
  }; // class VMA

} // namespace nostd

#endif // NO_STD_VMA_HPP
