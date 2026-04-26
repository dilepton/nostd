#include "io_uring.hpp"
#include "syscall.hpp"
#include "sysno.hpp"
#include "vma.hpp"
#include "memory.hpp"
#include "memory/defs.hpp"
#include "error.hpp"

using namespace nostd;

/* ========== CONSTRUCTORS & DESTRUCTORS ========== */

io_uring::io_uring(u32 entries, u32 flags) noexcept {
  io_uring_params p{};
  p.flags = flags;

  (void) init(entries, p);

  return;
}

io_uring::io_uring(u32 entries, io_uring_params& p) noexcept {
  (void) init(entries, p);

  return;
}

io_uring::io_uring(u32 entries, io_uring_params& p, byte* buf, usize buf_sz) noexcept {
  if (buf) p.flags |= IORING_SETUP_NO_MMAP;
  (void) init(entries, p, buf, buf_sz);
  
  return;
}

io_uring::~io_uring() noexcept {
  if (_int_flags & Flags::INT_FLAG_REG_RING) {
    // unreg
  }

  unmap_rings();
  if (_ringfd >= 0) {
    (void) __nostd_syscall1(sysno::CLOSE, _ringfd);
    _ringfd = -1;
  }
}

error_t io_uring::init(u32 entries, io_uring_params& p, byte* buf, usize buf_sz) noexcept {
  if (p.flags & IORING_SETUP_REGISTERED_FD_ONLY && !(p.flags & IORING_SETUP_NO_MMAP)) return E::INVAL;

  if (p.flags & IORING_SETUP_NO_MMAP) {
    if (error_t ret = no_mmap(entries, p, buf, buf_sz); ret < 0) {
      return ret;
    }
    if (buf) {
      _int_flags |= INT_FLAG_APP_MEM;
    }
  }

  const res_t fd = io_uring_setup(entries, &p);
  if (fd < 0) {
    if ((p.flags & IORING_SETUP_NO_MMAP) && !(_int_flags & INT_FLAG_APP_MEM)) {
      unmap_rings();
    }
    return static_cast<error_t>(fd);
  }

  if (p.flags & IORING_SETUP_NO_MMAP) {
    _sq.setup_ptrs(p);
    _cq.setup_ptrs(p);
  } else {
    if (error_t ret = map_rings(static_cast<i32>(fd), p); ret < 0) {
      __nostd_syscall1(sysno::CLOSE, fd);
      return ret;
    }
  }

  if (!(p.flags & IORING_SETUP_NO_SQARRAY)) {
    for (u32 i = 0; i < _sq.ring_entries; ++i) {
      _sq.array[i] = i;
    }
  }

  _flags = p.flags;
  _features = p.features;
  _ringfd = static_cast<i32>(fd);

  if (p.flags & IORING_SETUP_REGISTERED_FD_ONLY) {
    _int_flags |= INT_FLAG_REG_RING | INT_FLAG_REG_REG_RING;
  }

  if ((_flags & (IORING_SETUP_IOPOLL | IORING_SETUP_SQPOLL)) == IORING_SETUP_IOPOLL) {
    _int_flags |= INT_FLAG_CQ_ENTER;
  }

  return 0;
}

error_t io_uring::no_mmap(u32 entries, io_uring_params& p, byte* buf, usize buf_sz) noexcept {
  static constexpr u32   KERN_MAX_SQ    = 32768;
  static constexpr u32   KERN_MAX_CQ    = KERN_MAX_SQ * 2;
  static constexpr usize KRING_SIZE     = 64;
  static constexpr usize HUGE_PAGE_SIZE = 2u * 1024u * 1024u;

  if (!entries) return E::INVAL;
  if (entries > KERN_MAX_SQ) {
    if (!(p.flags & IORING_SETUP_CLAMP)) return E::INVAL;
    entries = KERN_MAX_SQ;
  }

  const u32 sqe_count = roundup_pow2(entries);
  u32 cqe_count = 0;

  if (p.flags & IORING_SETUP_CQSIZE) {
    if (!p.cq_entries) return E::INVAL;
    cqe_count = p.cq_entries;

    if (cqe_count > KERN_MAX_CQ) {
      if (!(p.flags & IORING_SETUP_CLAMP)) return E::INVAL;
      cqe_count = KERN_MAX_CQ;
    }
    cqe_count = roundup_pow2(cqe_count);
    if (cqe_count < sqe_count) return E::INVAL;
  } else {
    cqe_count = 2u * sqe_count;
  }

  usize sqes_mem = (static_cast<usize>(sqe_count) << !!(p.flags & IORING_SETUP_SQE128)) * sizeof(io_uring_sqe);
  if (!(p.flags & IORING_SETUP_NO_SQARRAY)) {
    sqes_mem += static_cast<usize>(sqe_count) * sizeof(u32);
  }
  sqes_mem = align_up(sqes_mem, VMA::PAGE_SIZE);

  const usize ring_mem = KRING_SIZE + (static_cast<usize>(cqe_count) << !!(p.flags & IORING_SETUP_CQE32)) * sizeof(io_uring_cqe);
  const usize mem_used = align_up(sqes_mem + ring_mem, VMA::PAGE_SIZE);

  if (!buf && (sqes_mem > HUGE_PAGE_SIZE || ring_mem > HUGE_PAGE_SIZE)) return E::NOMEM;

  byte* ptr     = nullptr;
  usize sqes_sz = 0;

  if (buf) {
    if (mem_used > buf_sz) return E::NOMEM;
    ptr = buf;
  } else {
    const usize want    = (sqes_mem <= VMA::PAGE_SIZE) ? VMA::PAGE_SIZE : HUGE_PAGE_SIZE;
    const long  hugetlb = (sqes_mem <= VMA::PAGE_SIZE) ? 0L              : MapFlags::HUGETLB;

    ptr = VMA::mmap(nullptr, want, Protection::RW,
                    MapFlags::SHARED | MapFlags::ANONYMOUS | hugetlb, -1, 0);
    if (is_error(ptr)) return static_cast<error_t>(ptr_err(ptr));

    sqes_sz = want;
    buf_sz  = want;
  }

  _sq.sqes    = reinterpret_cast<io_uring_sqe*>(ptr);
  _sq.sqes_sz = static_cast<u32>(sqes_sz);

  if (mem_used <= buf_sz) {
    _sq.ring_ptr = ptr + sqes_mem;
    _sq.ring_sz  = 0;
    _cq.ring_sz  = 0;
  } else {
    const usize want    = (ring_mem <= VMA::PAGE_SIZE) ? VMA::PAGE_SIZE : HUGE_PAGE_SIZE;
    const long  hugetlb = (ring_mem <= VMA::PAGE_SIZE) ? 0L              : MapFlags::HUGETLB;

    byte* ring = VMA::mmap(nullptr, want, Protection::RW, MapFlags::SHARED | MapFlags::ANONYMOUS | hugetlb, -1, 0);
    if (is_error(ring)) {
      const error_t e = static_cast<error_t>(ptr_err(ring));
      if (sqes_sz) VMA::munmap(ptr, sqes_sz);
      _sq.sqes    = nullptr;
      _sq.sqes_sz = 0;
      return e;
    }

    _sq.ring_ptr = ring;
    _sq.ring_sz  = want;
    _cq.ring_sz  = 0;
  }

  _cq.ring_ptr = _sq.ring_ptr;

  p.sq_off.user_addr = reinterpret_cast<__u64>(_sq.sqes);
  p.cq_off.user_addr = reinterpret_cast<__u64>(_sq.ring_ptr);

  return 0;
}

error_t io_uring::map_rings(i32 fd, io_uring_params& p) noexcept {
  usize sq_len = p.sq_off.array + p.sq_entries * sizeof(u32);
  usize cq_len = p.cq_off.cqes + (static_cast<usize>(p.cq_entries) << !!(p.flags & IORING_SETUP_CQE32)) * sizeof(io_uring_cqe);

  if (p.features & IORING_FEAT_SINGLE_MMAP) {
    const usize one = (cq_len > sq_len) ? cq_len : sq_len;
    sq_len = one;
    cq_len = one;
  }

  if (error_t e = _sq.map_ring(fd, p, sq_len)) {
    return e;
  }

  if (p.features & IORING_FEAT_SINGLE_MMAP) {
    _cq.ring_ptr = _sq.ring_ptr;
    _cq.ring_sz  = _sq.ring_sz;
  } else {
    if (error_t e = _cq.map_ring(fd, p, cq_len)) {
      unmap_rings();
      return e;
    }
  }

  if (error_t e = _sq.map_sqes(fd, p)) {
    unmap_rings();
    return e;
  }

  _sq.setup_ptrs(p);
  _cq.setup_ptrs(p);

  return 0;
}

void io_uring::unmap_rings() noexcept {
  if (_int_flags & INT_FLAG_APP_MEM) {
    _sq.reset();
    _cq.reset();
    return;
  }

  _sq.unmap_sqes();

  const bool shared = _sq.ring_ptr && _cq.ring_ptr && (_sq.ring_ptr == _cq.ring_ptr);
  if (shared) {
    _cq.ring_ptr = nullptr;
    _cq.ring_sz  = 0;
  } else {
    _cq.unmap_ring();
  }

  _sq.unmap_ring();
}

error_t io_uring::dontfork() noexcept {
  if (!_sq.ring_ptr || !_sq.sqes || !_cq.ring_ptr) return E::INVAL;

  const usize sqes_sz = (static_cast<usize>(_sq.ring_entries) << !!(_flags & IORING_SETUP_SQE128)) * sizeof(io_uring_sqe);

  if (res_t r = VMA::madvise(reinterpret_cast<byte*>(_sq.sqes), sqes_sz, Advice::DONTFORK); r < 0) {
    return static_cast<error_t>(r);
  }

  if (res_t r = VMA::madvise(_sq.ring_ptr, _sq.ring_sz, Advice::DONTFORK); r < 0) {
    return static_cast<error_t>(r);
  }

  if (_cq.ring_ptr != _sq.ring_ptr) {
    if (res_t r = VMA::madvise(_cq.ring_ptr, _cq.ring_sz, Advice::DONTFORK); r < 0) {
      return static_cast<error_t>(r);
    }
  }

  return 0;
}