#ifndef NO_STD_SYSTEM_X86_64_SYSCALL_HPP
#define NO_STD_SYSTEM_X86_64_SYSCALL_HPP

#ifdef __x86_64__
 /*
  * x86_64 syscall ABI:
  *
  *   instruction:  syscall
  *
  *   input:
  *     rax  - syscall number
  *     rdi  - arg0
  *     rsi  - arg1
  *     rdx  - arg2
  *     r10  - arg3
  *     r8   - arg4
  *     r9   - arg5
  *
  *   output:
  *     rax  - return value (or -errno on error)
  *
  *   clobbered:
  *     rcx  - destroyed (saved rip)
  *     r11  - destroyed (saved rflags)
  */

#define __nostd_syscall0(sysno) ({ \
  long rax;                        \
                                   \
  __asm__ volatile (               \
    "syscall"                      \
    : "=a"(rax)                    \
    : "a"(sysno)                   \
    : "rcx", "r11", "memory"       \
  );                               \
                                   \
  rax;                             \
})

#define __nostd_syscall1(sysno, arg1) ({             \
  long rax;                                          \
  register long __rdi __asm__("rdi") = (long)(arg1); \
                                                     \
  __asm__ volatile (                                 \
    "syscall"                                        \
    : "=a"(rax)                                      \
    : "a"(sysno), "r"(__rdi)                         \
    : "rcx", "r11", "memory"                         \
  );                                                 \
                                                     \
  rax;                                               \
})

#define __nostd_syscall2(sysno, arg1, arg2) ({       \
  long rax;                                          \
  register long __rdi __asm__("rdi") = (long)(arg1); \
  register long __rsi __asm__("rsi") = (long)(arg2); \
                                                     \
  __asm__ volatile (                                 \
    "syscall"                                        \
    : "=a"(rax)                                      \
    : "a"(sysno), "r"(__rdi), "r"(__rsi)             \
    : "rcx", "r11", "memory"                         \
  );                                                 \
                                                     \
  rax;                                               \
})

#define __nostd_syscall3(sysno, arg1, arg2, arg3) ({ \
  long rax;                                          \
  register long __rdi __asm__("rdi") = (long)(arg1); \
  register long __rsi __asm__("rsi") = (long)(arg2); \
  register long __rdx __asm__("rdx") = (long)(arg3); \
                                                     \
  __asm__ volatile (                                 \
    "syscall"                                        \
    : "=a"(rax)                                      \
    : "a"(sysno), "r"(__rdi), "r"(__rsi), "r"(__rdx) \
    : "rcx", "r11", "memory"                         \
  );                                                 \
                                                     \
  rax;                                               \
})

#define __nostd_syscall4(sysno, arg1, arg2, arg3, arg4) ({       \
  long rax;                                                      \
  register long __rdi __asm__("rdi") = (long)(arg1);             \
  register long __rsi __asm__("rsi") = (long)(arg2);             \
  register long __rdx __asm__("rdx") = (long)(arg3);             \
  register long __r10 __asm__("r10") = (long)(arg4);             \
                                                                 \
  __asm__ volatile (                                             \
    "syscall"                                                    \
    : "=a"(rax)                                                  \
    : "a"(sysno), "r"(__rdi), "r"(__rsi), "r"(__rdx), "r"(__r10) \
    : "rcx", "r11", "memory"                                     \
  );                                                             \
                                                                 \
  rax;                                                           \
})

#define __nostd_syscall5(sysno, arg1, arg2, arg3, arg4, arg5) ({ \
  long rax;                                                      \
  register long __rdi __asm__("rdi") = (long)(arg1);             \
  register long __rsi __asm__("rsi") = (long)(arg2);             \
  register long __rdx __asm__("rdx") = (long)(arg3);             \
  register long __r10 __asm__("r10") = (long)(arg4);             \
  register long __r8  __asm__("r8")  = (long)(arg5);             \
                                                                 \
  __asm__ volatile (                                             \
    "syscall"                                                    \
    : "=a"(rax)                                                  \
    : "a"(sysno), "r"(__rdi), "r"(__rsi), "r"(__rdx),            \
      "r"(__r10), "r"(__r8)                                      \
    : "rcx", "r11", "memory"                                     \
  );                                                             \
                                                                 \
  rax;                                                           \
})

#define __nostd_syscall6(sysno, arg1, arg2, arg3, arg4, arg5, arg6) ({ \
  long rax;                                                            \
  register long __rdi __asm__("rdi") = (long)(arg1);                   \
  register long __rsi __asm__("rsi") = (long)(arg2);                   \
  register long __rdx __asm__("rdx") = (long)(arg3);                   \
  register long __r10 __asm__("r10") = (long)(arg4);                   \
  register long __r8  __asm__("r8")  = (long)(arg5);                   \
  register long __r9  __asm__("r9")  = (long)(arg6);                   \
                                                                       \
  __asm__ volatile (                                                   \
    "syscall"                                                          \
    : "=a"(rax)                                                        \
    : "a"(sysno), "r"(__rdi), "r"(__rsi), "r"(__rdx),                  \
      "r"(__r10), "r"(__r8),  "r"(__r9)                                \
    : "rcx", "r11", "memory"                                           \
  );                                                                   \
                                                                       \
  rax;                                                                 \
})

#endif // __x86_64__

#if defined(__i386__) || (defined(__x86_64__) && defined(__ILP32__))

#endif // __i386__

#endif // NO_STD_SYSTEM_X86_64_SYSCALL_HPP
