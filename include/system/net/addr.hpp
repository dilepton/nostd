#ifndef NO_STD_NET_ADDR_HPP
#define NO_STD_NET_ADDR_HPP

#include "types.hpp"

namespace nostd
{
  struct alignas(2) sockaddr {
    u16  family;
    byte data[14];
  }; // struct sockaddr

  struct alignas(4) sockaddr_in {
    u16 family;
    u16 port;       // network byte order (htons)
    u32 addr;       // network byte order (htonl / ipv4())
    u8  pad[8];
  }; // struct sockaddr_in

  struct alignas(4) sockaddr_in6 {
    u16 family;
    u16 port;       // network byte order
    u32 flowinfo;   // traffic class + flow label
    u8  addr[16];   // 128-bit IPv6 address
    u32 scope_id;   // link-local scope
  }; // struct sockaddr_in6

  struct alignas(2) sockaddr_un {
    u16  family;
    char path[108];  // null-terminated socket path
  }; // struct sockaddr_un

  struct alignas(8) sockaddr_storage {
    u16 family;
    u8  pad[126];
  }; // struct sockaddr_storage

  struct alignas(8) iovec {
    void* base;
    usize len;
  }; // truct iovec

  struct alignas(8) cmsghdr {
    usize len;    // total length (header + data)
    i32   level;  // originating protocol (Level::SOCKET, Level::TCP, ...)
    i32   type;   // protocol-specific type (SO_TIMESTAMP, SCM_RIGHTS, ...)
    // variable-length data follows
  }; // struct cmsghdr

  struct alignas(8) msghdr {
    void*    name;        // remote address (sockaddr*)
    u32      namelen;     // sizeof(sockaddr_in) etc.
    iovec*   iov;
    usize    iovlen;
    cmsghdr* control;     // ancillary data buffer (cmsghdr*) in failure case CHANGE TO VOID*
    usize    controllen;
    u32      flags;
  }; // struct msghdr

  constexpr u16 htons(u16 v) noexcept {
    return __builtin_bswap16(v);
  }
  
  constexpr u16 ntohs(u16 v) noexcept {
    return __builtin_bswap16(v);
  }

  constexpr u32 htonl(u32 v) noexcept {
    return __builtin_bswap32(v);
  }

  constexpr u32 ntohl(u32 v) noexcept {
    return __builtin_bswap32(v);
  }

  constexpr u64 htonll(u64 v) noexcept {
    return __builtin_bswap64(v);
  }

  constexpr u64 ntohll(u64 v) noexcept {
    return __builtin_bswap64(v);
  }

} // namespace nostd

#endif // NO_STD_NET_ADDR_HPP