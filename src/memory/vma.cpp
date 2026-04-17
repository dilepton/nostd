#include "vma.hpp"

#include "syscall.hpp"
#include "sysno.hpp"

using namespace nostd;

byte* VMA::mmap(void* addr, usize len, long prot, long flags, long fd, long off) noexcept {
  return (byte*)__nostd_syscall6(sysno::MMAP, addr, len, prot, flags, fd, off);
}

long VMA::munmap(byte* p, usize len) noexcept {
  return __nostd_syscall2(sysno::MUNMAP, p, len);
}

byte* VMA::mremap(byte* p, usize old_sz, usize new_sz, long flags) noexcept {
  return (byte*)__nostd_syscall4(sysno::MREMAP, p, old_sz, new_sz, flags);
}

long VMA::mprotect(byte* p, usize len, long prot) noexcept {
  return __nostd_syscall3(sysno::MPROTECT, p, len, prot);
}

long VMA::mlock(byte* p, usize len) noexcept {
  return __nostd_syscall2(sysno::MLOCK, p, len);
}

long VMA::munlock(byte* p, usize len) noexcept {
  return __nostd_syscall2(sysno::MUNLOCK, p, len);
}

long VMA::madvise(byte* p, usize len, long advice) noexcept {
  return __nostd_syscall3(sysno::MADVISE, p, len, advice);
}