#include "io_uring.hpp"
#include "atomic.hpp"
#include "error.hpp"

using namespace nostd;

bool io_uring::__sq_needs_enter(u32 submitted, u32& enter_flags) noexcept {
  if (!submitted)                       return false;
  if (!(_flags & IORING_SETUP_SQPOLL))  return true;

  thread_fence(MemoryOrder::SEQ_CST);
  if (_sq.flags().load(MemoryOrder::RELAXED) & IORING_SQ_NEED_WAKEUP) {
    enter_flags |= IORING_ENTER_SQ_WAKEUP;
    return true;
  }
  return false;
}

res_t io_uring::__submit(u32 submitted, u32 wait_nr) noexcept {
  u32 enter_flags = 0;

  if (!__sq_needs_enter(submitted, enter_flags) && !wait_nr)
    return static_cast<res_t>(submitted);

  if (wait_nr)                         enter_flags |= IORING_ENTER_GETEVENTS;
  if (_int_flags & INT_FLAG_REG_RING)  enter_flags |= IORING_ENTER_REGISTERED_RING;

  return io_uring_enter(submitted, wait_nr, enter_flags, nullptr, 0);
}

res_t io_uring::submit() noexcept {
  return __submit(_sq.flush(), 0);
}

res_t io_uring::submit_and_wait(u32 wait_nr) noexcept {
  return __submit(_sq.flush(), wait_nr);
}

static inline void init_sqe(io_uring_sqe* sqe) noexcept {
  sqe->flags       = 0;
  sqe->ioprio      = 0;
  sqe->rw_flags    = 0;
  sqe->buf_index   = 0;
  sqe->personality = 0;
  sqe->file_index  = 0;
  sqe->addr3       = 0;
  sqe->__pad2[0]   = 0;
}

io_uring_sqe* io_uring::get_sqe() noexcept {
  const u32 tail  = _sq.sqe_tail;
  const int shift = (_flags & IORING_SETUP_SQE128) ? 1 : 0;

  u32 head;
  if (_flags & IORING_SETUP_SQPOLL) {
    head = _sq.head().load(MemoryOrder::ACQUIRE);
  } else {
    head = *_sq.khead;
  }

  if (tail - head >= _sq.ring_entries) return nullptr;

  io_uring_sqe* sqe = &_sq.sqes[(tail & _sq.ring_mask) << shift];
  _sq.sqe_tail = tail + 1;

  init_sqe(sqe);

  return sqe;
}

io_uring_cqe* io_uring::peek_cqe() noexcept {
  constexpr u64 LIBURING_UDATA_TIMEOUT = ~u64{0}; // Mirrored kernel constant

  const u32 tail  = __atomic_load_n(_cq.ktail, __ATOMIC_ACQUIRE);
  const u32 mask  = _cq.ring_mask;
  const int shift = (_flags & IORING_SETUP_CQE32) ? 1 : 0;

  u32 head = *_cq.khead;
  while (head != tail) {
    io_uring_cqe* cqe = &_cq.cqes[(head & mask) << shift];
    if (cqe->user_data != LIBURING_UDATA_TIMEOUT) return cqe;

    head += cqe_nr(cqe);
    _cq.head().store(head, MemoryOrder::RELEASE);
  }

  return nullptr;
}

void io_uring::cq_advance(u32 n) noexcept {
  if (n == 0) return;
  _cq.head().store(*_cq.khead + n, MemoryOrder::RELEASE);
}

error_t io_uring::__get_cqe(io_uring_cqe** cqe_out, u32 submit, u32 wait_nr) noexcept {
  *cqe_out = nullptr;

  while (true) {
    io_uring_cqe* cqe = peek_cqe();
    if (cqe) {
      *cqe_out = cqe;
      return 0;
    }

    if (!wait_nr && !submit) return E::AGAIN;

    u32 flags = IORING_ENTER_GETEVENTS;
    if (_int_flags & INT_FLAG_REG_RING) flags |= IORING_ENTER_REGISTERED_RING;

    const res_t ret = io_uring_enter(submit, wait_nr, flags, nullptr, 0);
    if (ret < 0) return static_cast<error_t>(ret);

    submit  = 0;
    wait_nr = 0;
  }
}

error_t io_uring::wait_cqe_nr(io_uring_cqe** cqe, u32 wait_nr) noexcept {
  return __get_cqe(cqe, 0, wait_nr);
}

error_t io_uring::wait_cqe(io_uring_cqe** cqe) noexcept {
  return wait_cqe_nr(cqe, 1);
}