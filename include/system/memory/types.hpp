#ifndef NO_STD_MEMORY_DEFS_HPP
#define NO_STD_MEMORY_DEFS_HPP

#include "linux/mman.h"
#include "asm-generic/mman-common.h"

namespace nostd 
{
  struct Protection final {
    static constexpr long N   = PROT_NONE;  // NONE
    static constexpr long R   = PROT_READ;  // READ
    static constexpr long W   = PROT_WRITE; // WRITE
    static constexpr long X   = PROT_EXEC;  // EXECUTE

    static constexpr long RW  = R | W; // READ | WRITE
    static constexpr long RX  = R | X; // READ | EXECUTE
    static constexpr long RWX = R | W | X; // READ | WRITE | EXECUTE
  }; // struct Protection

  struct MapFlags final {
    static constexpr long SHARED    = MAP_SHARED;
    static constexpr long PRIVATE   = MAP_PRIVATE;

    static constexpr long FIXED     = MAP_FIXED;
    static constexpr long FIXED_NOREPLACE = MAP_FIXED_NOREPLACE;
    static constexpr long ANONYMOUS = MAP_ANONYMOUS;
    static constexpr long POPULATE  = MAP_POPULATE;
    static constexpr long NONBLOCK  = MAP_NONBLOCK;

    static constexpr long STACK     = MAP_STACK;
    static constexpr long SYNC      = MAP_SYNC;

    static constexpr long HUGETLB   = MAP_HUGETLB;
    static constexpr long HUGE_2MB  = HUGETLB_FLAG_ENCODE_2MB;
    static constexpr long HUGE_1GB  = HUGETLB_FLAG_ENCODE_1GB;
  }; // struct MapFlags

  struct RemapFlags final {
    static constexpr long MAYMOVE   = MREMAP_MAYMOVE;
    static constexpr long FIXED     = MREMAP_FIXED;
    static constexpr long DONTUNMAP = MREMAP_DONTUNMAP;
  }; // struct RemapFlags 

  struct Advice final {
    static constexpr long NORMAL = MADV_NORMAL;
    static constexpr long RANDOM = MADV_RANDOM;
    static constexpr long SEQUENTIAL = MADV_SEQUENTIAL;
    static constexpr long WILLNEED = MADV_WILLNEED;
    static constexpr long FREE     = MADV_FREE;
    static constexpr long REMOVE   = MADV_REMOVE;
    static constexpr long DONTFORK = MADV_DONTFORK;
    static constexpr long DOFORK   = MADV_DOFORK; 
    static constexpr long HUGEPG   = MADV_HUGEPAGE;
    static constexpr long NOHUGEPG = MADV_NOHUGEPAGE;
    static constexpr long DONTDUMP = MADV_DONTDUMP;
    static constexpr long DODUMP   = MADV_DODUMP;
    // PREFAULT PAGE 
    static constexpr long POPULATE_READ  = MADV_POPULATE_READ; 
    static constexpr long POPULATE_WRITE = MADV_POPULATE_WRITE;
  }; // struct Advice

} // namespace nostd

#endif // NO_STD_MEMORY_DEFS_HPP