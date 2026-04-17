#ifndef NO_STD_NET_DEFS_HPP
#define NO_STD_NET_DEFS_HPP

namespace nostd
{
  struct Domain final {
    static constexpr long UNSPEC      = 0;   // unspecified
    static constexpr long LOCAL       = 1;   // local IPC (pipes, unix sockets)
    static constexpr long UNIX        = 1;   // alias for LOCAL (POSIX name)
    static constexpr long INET        = 2;   // IPv4
    static constexpr long AX25        = 3;   // amateur radio AX.25
    static constexpr long IPX         = 4;   // novell IPX
    static constexpr long APPLETALK   = 5;   // appletalk DDP
    static constexpr long NETROM      = 6;   // amateur radio NET/ROM
    static constexpr long BRIDGE      = 7;   // multiprotocol bridge
    static constexpr long ATMPVC      = 8;   // ATM permanent virtual circuits
    static constexpr long X25         = 9;   // ITU-T X.25 / ISO-8208
    static constexpr long INET6       = 10;  // IPv6
    static constexpr long ROSE        = 11;  // amateur radio X.25 PLP
    static constexpr long DECnet      = 12;  // DECnet
    static constexpr long NETBEUI     = 13;  // 802.2LLC
    static constexpr long SECURITY    = 14;  // security callback pseudo AF
    static constexpr long KEY         = 15;  // PF_KEY key management API
    static constexpr long NETLINK     = 16;  // kernel/userspace messaging
    static constexpr long PACKET      = 17;  // raw ethernet frames
    static constexpr long ASH         = 18;  // ash
    static constexpr long ECONET      = 19;  // acorn econet
    static constexpr long ATMSVC      = 20;  // ATM switched virtual circuits
    static constexpr long RDS         = 21;  // reliable datagram sockets
    static constexpr long SNA         = 22;  // linux SNA project
    static constexpr long IRDA        = 23;  // infrared (IrDA)
    static constexpr long PPPOX       = 24;  // PPP over X
    static constexpr long WANPIPE     = 25;  // wanpipe API
    static constexpr long LLC         = 26;  // linux LLC
    static constexpr long IB          = 27;  // native infiniband
    static constexpr long MPLS        = 28;  // multiprotocol label switching
    static constexpr long CAN         = 29;  // controller area network
    static constexpr long TIPC        = 30;  // transparent inter-process communication
    static constexpr long BLUETOOTH   = 31;  // bluetooth
    static constexpr long IUCV        = 32;  // inter-user communication vehicle (z/VM)
    static constexpr long RXRPC       = 33;  // rx RPC
    static constexpr long ISDN        = 34;  // ISDN
    static constexpr long PHONET      = 35;  // nokia phonet
    static constexpr long IEEE802154  = 36;  // IEEE 802.15.4 (low-rate wireless)
    static constexpr long CAIF        = 37;  // ST-Ericsson CAIF
    static constexpr long ALG         = 38;  // kernel crypto API
    static constexpr long NFC         = 39;  // near field communication
    static constexpr long VSOCK       = 40;  // VM sockets (guest ↔ host)
    static constexpr long KCM         = 41;  // kernel connection multiplexor
    static constexpr long QIPCRTR     = 42;  // qualcomm IPC router
    static constexpr long SMC         = 43;  // shared memory communications (IBM)
    static constexpr long XDP         = 44;  // express data path
    static constexpr long MCTP        = 45;  // management component transport protocol
  }; // struct Domain

  struct SocketType final {
    static constexpr long STREAM    = 1;       // reliable byte stream (TCP)
    static constexpr long DGRAM     = 2;       // connectionless datagrams (UDP)
    static constexpr long RAW       = 3;       // raw protocol access
    static constexpr long RDM       = 4;       // reliable datagrams (unordered)
    static constexpr long SEQPACKET = 5;       // reliable ordered datagrams (SCTP)
    static constexpr long DCCP      = 6;       // datagram congestion control protocol
    static constexpr long PACKET    = 10;      // device-level packet access (deprecated)
    // flags — OR with type
    static constexpr long NONBLOCK  = 2048;    // non-blocking mode on creation
    static constexpr long CLOEXEC   = 524288;  // close fd on exec
  }; // struct SocketType

  struct Protocol final {
    static constexpr long IP        = 0;    // dummy / default (kernel picks for you)
    static constexpr long ICMP      = 1;    // internet control message protocol
    static constexpr long IGMP      = 2;    // internet group management protocol
    static constexpr long IPIP      = 4;    // IP-in-IP tunneling
    static constexpr long TCP       = 6;    // transmission control protocol
    static constexpr long EGP       = 8;    // exterior gateway protocol
    static constexpr long PUP       = 12;   // PARC universal packet protocol
    static constexpr long UDP       = 17;   // user datagram protocol
    static constexpr long IDP       = 22;   // XNS IDP
    static constexpr long TP        = 29;   // SO transport protocol class 4
    static constexpr long DCCP      = 33;   // datagram congestion control protocol
    static constexpr long IPv6      = 41;   // IPv6 header
    static constexpr long RSVP      = 46;   // resource reservation protocol
    static constexpr long GRE       = 47;   // generic routing encapsulation
    static constexpr long ESP       = 50;   // encapsulating security payload (IPsec)
    static constexpr long AH        = 51;   // authentication header (IPsec)
    static constexpr long MTP       = 92;   // multicast transport protocol
    static constexpr long BEETPH    = 94;   // IP encapsulation within IP (BEET mode)
    static constexpr long ENCAP     = 98;   // encapsulation header
    static constexpr long PIM       = 103;  // protocol independent multicast
    static constexpr long COMP      = 108;  // compression header protocol
    static constexpr long L2TP      = 115;  // layer 2 tunneling protocol
    static constexpr long SCTP      = 132;  // stream control transmission protocol
    static constexpr long UDPLITE   = 136;  // UDP-Lite (partial checksum)
    static constexpr long MPLS      = 137;  // multiprotocol label switching in IP
    static constexpr long ETHERNET  = 143;  // ethernet within IP
    static constexpr long RAW       = 255;  // raw IP packets
    static constexpr long MPTCP     = 262;  // multipath TCP
  }; // struct Protocol

  struct Level final {
    static constexpr long IP     = 0;   // IPv4 options (IPPROTO_IP)
    static constexpr long SOCKET = 1;   // socket-level options (SOL_SOCKET)
    static constexpr long TCP    = 6;   // TCP options (IPPROTO_TCP)
    static constexpr long UDP    = 17;  // UDP options (IPPROTO_UDP)
    static constexpr long IPv6   = 41;  // IPv6 options (IPPROTO_IPV6)
  }; // struct Level

  struct SocketOption final {
    static constexpr long DEBUG         = 1;   // enable debug info
    static constexpr long REUSEADDR     = 2;   // allow local address reuse
    static constexpr long TYPE          = 3;   // get socket type (read-only)
    static constexpr long ERROR         = 4;   // get and clear pending error (read-only)
    static constexpr long DONTROUTE     = 5;   // bypass routing table
    static constexpr long BROADCAST     = 6;   // allow broadcast
    static constexpr long SNDBUF        = 7;   // send buffer size (bytes)
    static constexpr long RCVBUF        = 8;   // receive buffer size (bytes)
    static constexpr long KEEPALIVE     = 9;   // enable TCP keepalive probes
    static constexpr long OOBINLINE     = 10;  // receive out-of-band data inline
    static constexpr long NO_CHECK      = 11;  // disable checksum (UDP)
    static constexpr long PRIORITY      = 12;  // packet priority
    static constexpr long LINGER        = 13;  // linger on close if data present
    static constexpr long BSDCOMPAT     = 14;  // BSD bug-to-bug compatibility (deprecated)
    static constexpr long REUSEPORT     = 15;  // allow multiple sockets on same port
    static constexpr long PASSCRED      = 16;  // receive SCM_CREDENTIALS messages
    static constexpr long PEERCRED      = 17;  // get peer credentials (unix sockets)
    static constexpr long RCVLOWAT      = 18;  // minimum bytes before recv wakes up
    static constexpr long SNDLOWAT      = 19;  // minimum bytes before send wakes up
    static constexpr long RCVTIMEO      = 20;  // receive timeout
    static constexpr long SNDTIMEO      = 21;  // send timeout
    static constexpr long BINDTODEVICE  = 25;  // bind to specific network interface
    static constexpr long TIMESTAMP     = 29;  // receive packet timestamps
    static constexpr long ACCEPTCONN    = 30;  // socket is listening (read-only)
    static constexpr long SNDBUFFORCE   = 32;  // set send buffer ignoring rmem_max (CAP_NET_ADMIN)
    static constexpr long RCVBUFFORCE   = 33;  // set recv buffer ignoring rmem_max (CAP_NET_ADMIN)
    static constexpr long MARK          = 36;  // set fwmark for routing/netfilter
    static constexpr long PROTOCOL      = 38;  // get protocol (read-only)
    static constexpr long DOMAIN        = 39;  // get domain (read-only)
    static constexpr long BUSY_POLL     = 46;  // busy-poll timeout (microseconds)
    static constexpr long INCOMING_CPU  = 49;  // get CPU that last received data (read-only)
    static constexpr long ZEROCOPY      = 60;  // enable zero-copy sendmsg
    static constexpr long BINDTOIFINDEX = 62;  // bind to interface by index
  }; // struct SocketOption

  struct TCPOption final {
    static constexpr long NODELAY              = 1;   // disable Nagle's algorithm
    static constexpr long MAXSEG               = 2;   // maximum segment size
    static constexpr long CORK                 = 3;   // hold small packets until uncorked
    static constexpr long KEEPIDLE             = 4;   // idle time before keepalive probes (seconds)
    static constexpr long KEEPINTVL            = 5;   // interval between keepalive probes (seconds)
    static constexpr long KEEPCNT              = 6;   // number of keepalive probes before drop
    static constexpr long SYNCNT               = 7;   // number of SYN retransmits
    static constexpr long LINGER2              = 8;   // FIN_WAIT2 timeout (seconds)
    static constexpr long DEFER_ACCEPT         = 9;   // wake listener only when data arrives
    static constexpr long WINDOW_CLAMP         = 10;  // bound advertised receive window
    static constexpr long INFO                 = 11;  // get TCP connection info (struct tcp_info)
    static constexpr long QUICKACK             = 12;  // enable quick ACK mode (disable delayed ACK)
    static constexpr long CONGESTION           = 13;  // set congestion control algorithm name
    static constexpr long MD5SIG               = 14;  // TCP MD5 signature (RFC 2385)
    static constexpr long THIN_LINEAR_TIMEOUTS = 16;  // linear timeouts for thin streams
    static constexpr long THIN_DUPACK          = 17;  // fast retransmit on first dupack (thin streams)
    static constexpr long USER_TIMEOUT         = 18;  // max time (ms) data may remain unacked
    static constexpr long REPAIR               = 19;  // enable TCP connection repair mode
    static constexpr long REPAIR_QUEUE         = 20;  // select queue for repair
    static constexpr long QUEUE_SEQ            = 21;  // set sequence number for repair
    static constexpr long REPAIR_OPTIONS       = 22;  // set TCP options for repair
    static constexpr long FASTOPEN             = 23;  // enable TCP Fast Open (server: queue length)
    static constexpr long TIMESTAMP            = 24;  // enable TCP timestamps (RFC 1323)
    static constexpr long NOTSENT_LOWAT        = 25;  // min unsent bytes before writable notification
    static constexpr long CC_INFO              = 26;  // get congestion control info
    static constexpr long SAVE_SYN             = 27;  // save incoming SYN headers
    static constexpr long SAVED_SYN            = 28;  // get saved SYN headers
    static constexpr long REPAIR_WINDOW        = 29;  // set window parameters for repair
    static constexpr long FASTOPEN_CONNECT     = 30;  // TFO on connect() (client-side)
    static constexpr long ULP                  = 31;  // attach upper layer protocol (e.g. kTLS)
    static constexpr long MD5SIG_EXT           = 32;  // extended TCP MD5 signature
    static constexpr long FASTOPEN_KEY         = 33;  // set TFO server key
    static constexpr long FASTOPEN_NO_COOKIE   = 34;  // TFO without cookie validation
    static constexpr long ZEROCOPY_RECEIVE     = 35;  // zero-copy receive via mmap
    static constexpr long INQ                  = 36;  // get bytes in receive queue (via cmsg)
    static constexpr long TX_DELAY             = 37;  // delay transmission (nanoseconds)
  }; // struct TCPOption

  struct Shut final {
    static constexpr long RD   = 0;  // shutdown read half
    static constexpr long WR   = 1;  // shutdown write half
    static constexpr long RDWR = 2;  // shutdown both
  }; // struct Shut

  struct MsgFlag final {
    static constexpr long OOB       = 1;         // out-of-band data
    static constexpr long PEEK      = 2;         // peek at incoming data without consuming
    static constexpr long DONTROUTE = 4;         // bypass routing, use direct interface
    static constexpr long TRUNC     = 32;        // data was truncated (datagram too large)
    static constexpr long DONTWAIT  = 64;        // non-blocking for this call only
    static constexpr long EOR       = 128;       // end of record
    static constexpr long WAITALL   = 256;       // block until full request is satisfied
    static constexpr long NOSIGNAL  = 16384;     // suppress SIGPIPE on broken connection
    static constexpr long MORE      = 32768;     // more data coming (cork-like hint)
    static constexpr long ZEROCOPY  = 0x4000000; // zero-copy send
  }; // struct MsgFlag

} // namespace nostd::net

#endif // NO_STD_NET_DEFS_HPP