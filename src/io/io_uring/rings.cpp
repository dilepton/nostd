#include "io_uring.hpp"
#include "memory/defs.hpp"
#include "vma.hpp"
#include "error.hpp"

using namespace nostd;

/* ========== SQ RING ========== */

error_t io_uring_sq::map_ring(i32 fd, const io_uring_params& p, usize ring_bytes) noexcept {
  if (ring_bytes == 0) {
    ring_bytes = p.sq_off.array + p.sq_entries * sizeof(u32);
  }

  ring_sz  = ring_bytes;
  ring_ptr = VMA::mmap(nullptr, ring_sz, Protection::RW, MapFlags::SHARED | MapFlags::POPULATE, fd, IORING_OFF_SQ_RING);
  if (is_error(ring_ptr)) {
    const error_t e = ptr_err(ring_ptr);
    ring_ptr = nullptr;
    ring_sz  = 0;
    return e;
  }

  return 0;
}

error_t io_uring_sq::map_sqes(i32 fd, const io_uring_params& p) noexcept {
  sqes_sz = (static_cast<usize>(p.sq_entries) << !!(p.flags & IORING_SETUP_SQE128)) * sizeof(io_uring_sqe);
  if ((unsigned int) sqes_sz != sqes_sz) {
    sqes_sz = 0;
    return E::INVAL;
  }

  sqes = (io_uring_sqe*) VMA::mmap(nullptr, sqes_sz, Protection::RW, MapFlags::SHARED | MapFlags::POPULATE, fd, IORING_OFF_SQES);
  if (is_error(sqes)) {
    const error_t e = ptr_err(sqes);
    sqes_sz = 0;
    sqes = nullptr;
    return e;
  }

  return 0;
}

void io_uring_sq::setup_ptrs(const io_uring_params& p) noexcept {
  khead        = reinterpret_cast<u32*>(ring_ptr + p.sq_off.head);
  ktail        = reinterpret_cast<u32*>(ring_ptr + p.sq_off.tail);
  kflags       = reinterpret_cast<u32*>(ring_ptr + p.sq_off.flags);
  kdropped     = reinterpret_cast<u32*>(ring_ptr + p.sq_off.dropped);
  if (!(p.flags & IORING_SETUP_NO_SQARRAY)) {
    array = reinterpret_cast<u32*>(ring_ptr + p.sq_off.array);
  }
  ring_mask    = *reinterpret_cast<u32*>(ring_ptr + p.sq_off.ring_mask);
  ring_entries = *reinterpret_cast<u32*>(ring_ptr + p.sq_off.ring_entries);

  return;
}

void io_uring_sq::unmap_ring() noexcept {
  if (ring_ptr && ring_sz) {
    VMA::munmap(ring_ptr, ring_sz);
    ring_ptr = nullptr;
    ring_sz = 0;
  }

  return;
}

void io_uring_sq::unmap_sqes() noexcept {
  if (sqes && sqes_sz) {
    VMA::munmap(reinterpret_cast<byte*>(sqes), sqes_sz);
    sqes_sz = 0;
    sqes = nullptr;
  }

  return;
}

void io_uring_sq::reset() noexcept {
  khead = ktail = kflags = kdropped = array = nullptr;
  ring_ptr   = nullptr;
  sqes       = nullptr;
  ring_mask  = ring_entries = sqe_head = sqe_tail = ring_sz = sqes_sz = 0;
}

u32 io_uring_sq::flush() noexcept {
  if (sqe_head != sqe_tail) {
    tail().store(sqe_tail, MemoryOrder::RELEASE);
    sqe_head = sqe_tail;
  }
  
  return sqe_tail - *khead;
}

/* ========== CQ RING ========== */
u32 io_uring_cq::ready() const noexcept {
  return __atomic_load_n(ktail, __ATOMIC_ACQUIRE) - *khead;
}

error_t io_uring_cq::map_ring(i32 fd, const io_uring_params& p, usize ring_bytes) noexcept {
  if (ring_bytes == 0) {
    ring_bytes = p.cq_off.cqes + (static_cast<usize>(p.cq_entries) << !!(p.flags & IORING_SETUP_CQE32)) * sizeof(io_uring_cqe);
  }

  ring_sz  = ring_bytes;
  ring_ptr = VMA::mmap(nullptr, ring_sz, Protection::RW, MapFlags::SHARED | MapFlags::POPULATE, fd, IORING_OFF_CQ_RING);
  if (is_error(ring_ptr)) {
    const error_t e = ptr_err(ring_ptr);
    ring_ptr = nullptr;
    ring_sz  = 0;
    return e;
  }
  
  return 0;
}

void io_uring_cq::setup_ptrs(const io_uring_params& p) noexcept {
  khead     = reinterpret_cast<u32*>(ring_ptr + p.cq_off.head);
  ktail     = reinterpret_cast<u32*>(ring_ptr + p.cq_off.tail);
  koverflow = reinterpret_cast<u32*>(ring_ptr + p.cq_off.overflow);
  cqes      = reinterpret_cast<io_uring_cqe*>(ring_ptr + p.cq_off.cqes);
  if (p.cq_off.flags) {
    kflags  = reinterpret_cast<u32*>(ring_ptr + p.cq_off.flags);
  }
  ring_mask = *reinterpret_cast<u32*>(ring_ptr + p.cq_off.ring_mask);
  ring_entries = *reinterpret_cast<u32*>(ring_ptr + p.cq_off.ring_entries);

  return;
}

void io_uring_cq::unmap_ring() noexcept {
  if (ring_ptr && ring_sz) {
    VMA::munmap(ring_ptr, ring_sz);
    ring_ptr = nullptr;
    ring_sz = 0;
  }

  return;
}

void io_uring_cq::reset() noexcept {
  khead = ktail = koverflow = kflags = nullptr;
  cqes     = nullptr;
  ring_ptr = nullptr;
  ring_mask = ring_entries = ring_sz = 0;
}