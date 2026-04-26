#include "io_uring.hpp"

#include "syscall.hpp"
#include "sysno.hpp"
#include "error.hpp"

using namespace nostd;

/* ========== SYSCALLS ========== */

res_t io_uring::io_uring_setup(u32 entries, io_uring_params* p) noexcept {
  return __nostd_syscall2(sysno::IO_URING_SETUP, entries, p);
}

error_t io_uring::io_uring_register() noexcept {
  return E::NOSYS;
}

res_t io_uring::io_uring_enter(u32 to_submit, u32 min_complete, u32 flags, void* arg, usize argsz) noexcept {
  return __nostd_syscall6(sysno::IO_URING_ENTER, _ringfd, to_submit, min_complete, flags, arg, argsz);
}