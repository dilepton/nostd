#ifndef NO_STD_SYSTEM_X86_SYSNO_HPP
#define NO_STD_SYSTEM_X86_SYSNO_HPP
#ifdef __x86_64__

namespace nostd::sysno
{
  /* === MEMORY === */
  constexpr long MMAP       = 9;
  constexpr long MUNMAP     = 11;
  constexpr long MREMAP     = 25;
  constexpr long MPROTECT   = 10;
  constexpr long MADVISE    = 28;
  constexpr long MLOCK      = 149;
  constexpr long MUNLOCK    = 150;
  constexpr long MLOCKALL   = 151;
  constexpr long MUNLOCKALL = 152;
  //constexpr long MSYNC    = 26;

  /* === IO === */
  constexpr long OPEN     = 2;
  constexpr long OPENAT   = 257;
  constexpr long OPENAT2  = 437;

  constexpr long READ     = 0;
  constexpr long READV    = 19;
  constexpr long PREAD64  = 17;

  constexpr long WRITE    = 1;
  constexpr long WRITEV   = 20;
  constexpr long PWRITE64 = 18;

  constexpr long CLOSE        = 3;
  constexpr long LSEEK        = 8;
  constexpr long NEWFSTATAT   = 262;
  constexpr long STATX        = 332;
  constexpr long FCNTL        = 72;
  constexpr long IOCTL        = 16;
  constexpr long DUP3         = 292;
  constexpr long FALLOCATE    = 285;
  constexpr long SENDFILE     = 40;
  constexpr long MEMFD_CREATE = 319;

  /* === SOCKET ===*/
  constexpr long SOCKET   = 41;
  constexpr long SHUTDOWN = 48;
  constexpr long CONNECT  = 42;

  constexpr long BIND    = 49;
  constexpr long LISTEN  = 50;
  constexpr long ACCEPT  = 43;
  constexpr long ACCEPT4 = 288;

  constexpr long SENDTO  = 44;
  constexpr long SENDMSG = 46;

  constexpr long RECVFROM = 45;
  constexpr long RECVMSG  = 47;

  constexpr long GETSOCKNAME = 51;
  constexpr long GETPEERNAME = 52;
  constexpr long SOCKETPAIR  = 53;
  constexpr long SETSOCKOPT  = 54;
  constexpr long GETSOCKOPT  = 55;

  /* === ASYNC IO === */
  // epoll
  constexpr long EPOLL_CREATE1 = 291;
  constexpr long EPOLL_CTL     = 233;
  constexpr long EPOLL_WAIT    = 232;
  constexpr long EPOLL_PWAIT   = 281;

  // -- event --
  constexpr long TIMERFD_CREATE = 283;
  constexpr long EVENTFD2       = 290;
  constexpr long PIPE2          = 293;

  // -- io_uring --
  constexpr long IO_URING_SETUP    = 425;
  constexpr long IO_URING_ENTER    = 426;
  constexpr long IO_URING_REGISTER = 427;

} // namespace nostd::sysno

#endif // __x86_64__
#endif // NO_STD_SYSTEM_X86_SYSNO_HPP