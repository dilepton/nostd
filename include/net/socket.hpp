#ifndef NO_STD_NET_SOCKET_HPP
#define NO_STD_NET_SOCKET_HPP

#include "types.hpp"
#include "defs.hpp"
#include "addr.hpp"

namespace nostd
{
  class Socket final {
  private: // Fields
    long _fd{-1};

  public: // c/d-tors & operators
    Socket() noexcept = default;
    ~Socket() noexcept;

    // NO COPY - MOVE ONLY
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    explicit operator bool() const noexcept { return _fd >= 0; }

  public: // Methods
    // Getters
    inline long fd() const noexcept { return _fd; }

    // Control
    void    open(long domain, long type, long proto = 0) noexcept;
    error_t shutdown(long how = Shut::RDWR) noexcept;
    void    close() noexcept;

    // Options
    error_t setopt(long level, long opt, const void* val, u32 len) noexcept;
    error_t getopt(long level, long opt, void* val, u32* len) noexcept;

    // Client
    error_t connect(const sockaddr_in& address) noexcept;
    error_t connect(const sockaddr_in6& address) noexcept;

    // Server
    error_t bind(const sockaddr_in& address) noexcept;
    error_t bind(const sockaddr_in6& address) noexcept;
    error_t listen(long backlog = 128) noexcept;
    Socket  accept(long flags = 0) noexcept;

    // Data - TCP
    long send(const void* buffer, usize len, long flags = 0) noexcept;
    long recv(void* buffer, usize len, long flags = 0) noexcept;

    // Data - UDP
    long sendto(const void* buffer, usize len, long flags, const sockaddr_in& address) noexcept;
    long sendto(const void* buffer, usize len, long flags, const sockaddr_in6& address) noexcept;
    long recvfrom(void* buf, usize len, long flags, sockaddr_in& addr) noexcept;
    long recvfrom(void* buf, usize len, long flags, sockaddr_in6& addr) noexcept;

    // Data - Other
    long sendmsg(const msghdr& mhdr, long flags = 0) noexcept;
    long recvmsg(msghdr& mhdr, long flags = 0) noexcept;
  }; // class Socket
  
} // namespace nostd

#endif // NO_STD_NET_SOCKET_HPP