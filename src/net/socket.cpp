#include "socket.hpp"

#include "syscall.hpp"
#include "sysno.hpp"

using namespace nostd;

/* === LIFECYCLE === */

Socket::~Socket() noexcept {
  close();
}

Socket::Socket(Socket&& other) noexcept : _fd{other._fd} {
  other._fd = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept {
  if (this != &other) {
    close();
    _fd = other._fd;
    other._fd = -1;
  }
  return *this;
}

/* === CONTROL === */

void Socket::open(long domain, long type, long proto) noexcept {
  close();
  _fd = __nostd_syscall3(sysno::SOCKET, domain, type, proto);
}

error_t Socket::shutdown(long how) noexcept {
  return __nostd_syscall2(sysno::SHUTDOWN, _fd, how);
}

void Socket::close() noexcept {
  if (_fd >= 0) {
    __nostd_syscall1(sysno::CLOSE, _fd);
    _fd = -1;
  }
}

/* === OPTIONS === */

error_t Socket::setopt(long level, long opt, const void* val, u32 len) noexcept {
  return __nostd_syscall5(sysno::SETSOCKOPT, _fd, level, opt, val, len);
}

error_t Socket::getopt(long level, long opt, void* val, u32* len) noexcept {
  return __nostd_syscall5(sysno::GETSOCKOPT, _fd, level, opt, val, len);
}

/* === CLIENT === */

error_t Socket::connect(const sockaddr_in& address) noexcept {
  return __nostd_syscall3(sysno::CONNECT, _fd, &address, sizeof(sockaddr_in));
}

error_t Socket::connect(const sockaddr_in6& address) noexcept {
  return __nostd_syscall3(sysno::CONNECT, _fd, &address, sizeof(sockaddr_in6));
}

/* === SERVER === */

error_t Socket::bind(const sockaddr_in& address) noexcept {
  return __nostd_syscall3(sysno::BIND, _fd, &address, sizeof(sockaddr_in));
}

error_t Socket::bind(const sockaddr_in6& address) noexcept {
  return __nostd_syscall3(sysno::BIND, _fd, &address, sizeof(sockaddr_in6));
}

error_t Socket::listen(long backlog) noexcept {
  return __nostd_syscall2(sysno::LISTEN, _fd, backlog);
}

Socket Socket::accept(long flags) noexcept {
  Socket s;
  s._fd = __nostd_syscall4(sysno::ACCEPT4, _fd, 0, 0, flags);
  return s;
}

/* === DATA - TCP === */

long Socket::send(const void* buffer, usize len, long flags) noexcept {
  return __nostd_syscall6(sysno::SENDTO, _fd, buffer, len, flags, 0, 0);
}

long Socket::recv(void* buffer, usize len, long flags) noexcept {
  return __nostd_syscall6(sysno::RECVFROM, _fd, buffer, len, flags, 0, 0);
}

/* === DATA - UDP === */

long Socket::sendto(const void* buffer, usize len, long flags, const sockaddr_in& address) noexcept {
  return __nostd_syscall6(sysno::SENDTO, _fd, buffer, len, flags, &address, sizeof(sockaddr_in));
}

long Socket::sendto(const void* buffer, usize len, long flags, const sockaddr_in6& address) noexcept {
  return __nostd_syscall6(sysno::SENDTO, _fd, buffer, len, flags, &address, sizeof(sockaddr_in6));
}

long Socket::recvfrom(void* buf, usize len, long flags, sockaddr_in& addr) noexcept {
  u32 addrlen = sizeof(sockaddr_in);
  return __nostd_syscall6(sysno::RECVFROM, _fd, buf, len, flags, &addr, &addrlen);
}

long Socket::recvfrom(void* buf, usize len, long flags, sockaddr_in6& addr) noexcept {
  u32 addrlen = sizeof(sockaddr_in6);
  return __nostd_syscall6(sysno::RECVFROM, _fd, buf, len, flags, &addr, &addrlen);
}

/* === DATA - MSG === */

long Socket::sendmsg(const msghdr& mhdr, long flags) noexcept {
  return __nostd_syscall3(sysno::SENDMSG, _fd, &mhdr, flags);
}

long Socket::recvmsg(msghdr& mhdr, long flags) noexcept {
  return __nostd_syscall3(sysno::RECVMSG, _fd, &mhdr, flags);
}
