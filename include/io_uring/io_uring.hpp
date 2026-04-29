#ifndef NO_STD_IO_URING_HPP
#define NO_STD_IO_URING_HPP

#include "types.hpp"
#include "atomic.hpp"
#include "net/addr.hpp"

#include <linux/io_uring.h>
#include <linux/openat2.h>

namespace nostd
{
  inline constexpr i32 AT_FDCWD = -100; // Mirrored kernel constant

  enum Flags : u8 {
    INT_FLAG_REG_RING  = IORING_ENTER_REGISTERED_RING,
    INT_FLAG_NO_IOWAIT = IORING_ENTER_NO_IOWAIT,
    INT_FLAG_REG_REG_RING	= 1,
	  INT_FLAG_APP_MEM	= 2,
	  INT_FLAG_CQ_ENTER	= 4,
  }; // enum Flags

  struct alignas(8) io_uring_sq {
  public: // Fields
    u32* khead{nullptr};
    u32* ktail{nullptr};
    u32  ring_mask{0};
    u32  ring_entries{0};
    u32* kflags{nullptr};
    u32* kdropped{nullptr};

    u32* array{nullptr};
    io_uring_sqe* sqes{nullptr};
    u32 sqe_head{0};
    u32 sqe_tail{0};

    usize ring_sz{0};
    byte* ring_ptr{nullptr};
    u32 sqes_sz{0};

  public: // Methods
    error_t map_ring(i32 fd, const io_uring_params& p, usize ring_bytes = 0) noexcept;
    error_t map_sqes(i32 fd, const io_uring_params& p) noexcept;
    void setup_ptrs(const io_uring_params& p) noexcept;

    void unmap_ring() noexcept;
    void unmap_sqes() noexcept;
    void reset()      noexcept;

    // Accessors
    Atomic<u32&> head()  noexcept { return Atomic<u32&>(khead);  }
    Atomic<u32&> tail()  noexcept { return Atomic<u32&>(ktail);  }
    Atomic<u32&> flags() noexcept { return Atomic<u32&>(kflags); }

    u32 ready() const noexcept { return sqe_tail - sqe_head; }
    u32 space_left() const noexcept { return ring_entries - ready(); }
    u32 flush() noexcept;
  }; // struct io_uring_sq

  struct alignas(8) io_uring_cq {
  public: // Fields
    u32* khead{nullptr};
    u32* ktail{nullptr};
    u32  ring_mask{0};
    u32  ring_entries{0};
    u32* kflags{nullptr};
    u32* koverflow{nullptr};

    struct io_uring_cqe* cqes{nullptr};

    usize ring_sz{0};
    byte* ring_ptr{nullptr};

  public: // Methods
    error_t map_ring(i32 fd, const io_uring_params& p, usize ring_bytes = 0) noexcept;
    void setup_ptrs(const io_uring_params& p) noexcept;

    void unmap_ring() noexcept;
    void reset() noexcept;

    // Accessors
    Atomic<u32&> head()  noexcept { return Atomic<u32&>(khead);  }
    Atomic<u32&> tail()  noexcept { return Atomic<u32&>(ktail);  }
    Atomic<u32&> flags() noexcept { return Atomic<u32&>(kflags); }

    u32 ready() const noexcept;
  }; // struct io_uring_cq

  class alignas(8) io_uring {
  private: // Fields
    io_uring_sq _sq;
    io_uring_cq _cq;

    u32 _flags{0};
    i32 _ringfd{-1};
    i32 _enterfd{-1};
    u32 _features{0};
    u8  _int_flags{0};

  private: // Methods

    constexpr u32 roundup_pow2(u32 d) noexcept {
      if (d <= 1) return 1;
      if (d > (1u << 31)) return 1u << 31;
      return 1u << (32 - __builtin_clz(d - 1));
    }

    // SYSCALLs
    res_t   io_uring_setup(u32 entries, io_uring_params* p) noexcept;
    error_t io_uring_register(i32 fd, u32 opcode, const void* arg, u32 nr_args) noexcept;
    res_t   io_uring_enter(u32 to_submit, u32 min_complete, u32 flags, void* arg, usize argsz) noexcept;

    // Setup
    error_t init(u32 entries, io_uring_params& p, byte* buf = nullptr, usize buf_sz = 0) noexcept;
    error_t map_rings(i32 fd, io_uring_params& p) noexcept;
    error_t no_mmap(u32 entries, io_uring_params& p, byte* buf, usize buf_sz) noexcept;
    void    unmap_rings() noexcept;
    error_t dontfork() noexcept;

    // Submission
    bool  __sq_needs_enter(u32 submitted, u32& enter_flags) noexcept;
    res_t __submit(u32 submitted, u32 wait_nr) noexcept;

    // Completion
    error_t __get_cqe(io_uring_cqe** cqe_out, u32 submit, u32 wait_nr) noexcept;
    res_t do_register(u32 opcode, const void* arg, u32 nr_args) noexcept;

  public: // Constructors & Destructots & Operators
    explicit io_uring(u32 entries, u32 flags = 0) noexcept;
    io_uring(u32 entries, io_uring_params& p) noexcept;
    io_uring(u32 entries, io_uring_params& p, byte* buf = nullptr, usize buf_sz = 0) noexcept;
    ~io_uring() noexcept;
    
    io_uring(const io_uring&) noexcept = delete;
    io_uring& operator=(const io_uring&) noexcept = delete;

    explicit operator bool() const noexcept { return _ringfd >= 0; }

  public: // Methods

    // Probe
    ::io_uring_probe* get_probe_ring() noexcept;
    static ::io_uring_probe* get_probe() noexcept;

    static inline bool opcode_supported(const ::io_uring_probe* p, i32 op) noexcept {
      if (!p || op < 0 || op > p->last_op) return false;
      return (p->ops[op].flags & IO_URING_OP_SUPPORTED) != 0;
    }

    static void free_probe(::io_uring_probe* probe) noexcept;
    io_uring_sqe* get_sqe() noexcept;
    u32 sq_ready()      const noexcept { return _sq.ready(); }
    u32 sq_space_left() const noexcept { return _sq.space_left(); }

    res_t submit() noexcept;
    res_t submit_and_wait(u32 wait_nr) noexcept;

    // Completion
    u32 cq_ready() const noexcept { return _cq.ready(); }

    io_uring_cqe* peek_cqe() noexcept;
    error_t wait_cqe(io_uring_cqe** cqe) noexcept;
    error_t wait_cqe_nr(io_uring_cqe** cqe, u32 wait_nr) noexcept;

    void cq_advance(u32 n) noexcept;
    static inline u32 cqe_nr(const io_uring_cqe* cqe) noexcept {
      return (cqe->flags & IORING_CQE_F_32) ? 2u : 1u;
    }
    void cqe_seen(io_uring_cqe* cqe) noexcept { if (cqe) cq_advance(cqe_nr(cqe)); }

    template<typename F>
    u32 for_each_cqe(F&& fn) noexcept {
      u32 head = *_cq.khead;
      const u32 tail  = __atomic_load_n(_cq.ktail, __ATOMIC_ACQUIRE);
      const u32 mask  = _cq.ring_mask;
      const int shift = (_flags & IORING_SETUP_CQE32) ? 1 : 0;
      const u32 start = head;

      while (head != tail) {
        io_uring_cqe* cqe = &_cq.cqes[(head & mask) << shift];
        head += cqe_nr(cqe);
        
        if constexpr (__is_same(decltype(fn(cqe)), bool)) {
          if (!fn(cqe)) break; 
        } else {
          fn(cqe);
        }
        
      } // while (head != tail)

      const u32 n = head - start;
      if (n) {
        _cq.head().store(head, MemoryOrder::RELEASE);
      }
      
      return n;
    } // u32 for_each_cqe()

  /* ================================================================
   *                       IO Helpers API
   * ================================================================ */

   /* ------ Buffer Ring ------ */
    ::io_uring_buf_ring* setup_buf_ring(u32 nentries, u16 bgid, u16 flags, error_t* err) noexcept;
    res_t free_buf_ring(::io_uring_buf_ring* br, u32 nentries, u16 bgid)  noexcept;
    void buf_ring_cq_advance(::io_uring_buf_ring* br, u32 count) noexcept;
    res_t buf_ring_available(::io_uring_buf_ring* br, u16 bgid) noexcept;

    static inline u32 buf_ring_mask(u32 ring_entries) noexcept {
      return ring_entries - 1;
    }

    static inline void buf_ring_init(::io_uring_buf_ring* br) noexcept {
      br->tail = 0;
    }

    static inline void buf_ring_add(::io_uring_buf_ring* br, void* addr, u32 len, u16 bid, i32 mask, i32 buf_offset) noexcept {
      ::io_uring_buf* buf = &br->bufs[(br->tail + buf_offset) & mask];
      buf->addr = reinterpret_cast<u64>(addr);
      buf->len  = len;
      buf->bid  = bid;
    }

    static inline void buf_ring_advance(::io_uring_buf_ring* br, i32 count) noexcept {
      const u16 new_tail = static_cast<u16>(br->tail + count);
      __atomic_store_n(&br->tail, new_tail, __ATOMIC_RELEASE);
    }

   /* ------ Msghdr Helpers ------ */
    static inline ::io_uring_recvmsg_out* recvmsg_validate(void* buf, i32 buf_len, msghdr* msgh) noexcept {
      const usize header = msgh->controllen + msgh->namelen + sizeof(::io_uring_recvmsg_out);
      if (buf_len < 0 || static_cast<usize>(buf_len) < header) return nullptr;
      return static_cast<::io_uring_recvmsg_out*>(buf);
    }

    static inline void* recvmsg_name(::io_uring_recvmsg_out* o) noexcept {
      return static_cast<void*>(o + 1);
    }

    static inline cmsghdr* recvmsg_cmsg_firsthdr(::io_uring_recvmsg_out* o, msghdr* msgh) noexcept {
      if (o->controllen < sizeof(cmsghdr)) return nullptr;
      return reinterpret_cast<cmsghdr*>(
        static_cast<byte*>(recvmsg_name(o)) + msgh->namelen);
    }

    static inline cmsghdr* recvmsg_cmsg_nexthdr(::io_uring_recvmsg_out* o, msghdr* msgh, cmsghdr* cmsg) noexcept {
      constexpr usize CMSG_ALIGN_MASK = sizeof(long) - 1;
      if (cmsg->len < sizeof(cmsghdr)) return nullptr;

      byte* end = reinterpret_cast<byte*>(recvmsg_cmsg_firsthdr(o, msgh)) + o->controllen;
      cmsg = reinterpret_cast<cmsghdr*>(
        reinterpret_cast<byte*>(cmsg) + ((cmsg->len + CMSG_ALIGN_MASK) & ~CMSG_ALIGN_MASK));

      if (reinterpret_cast<byte*>(cmsg + 1) > end) return nullptr;
      if (reinterpret_cast<byte*>(cmsg) + ((cmsg->len + CMSG_ALIGN_MASK) & ~CMSG_ALIGN_MASK) > end) return nullptr;

      return cmsg;
    }

    static inline void* recvmsg_payload(::io_uring_recvmsg_out* o, msghdr* msgh) noexcept {
      return static_cast<void*>(
        static_cast<byte*>(recvmsg_name(o)) + msgh->namelen + msgh->controllen);
    }

    static inline u32 recvmsg_payload_length(::io_uring_recvmsg_out* o, usize buf_len, msghdr* msgh) noexcept {
      const usize payload_start = reinterpret_cast<usize>(recvmsg_payload(o, msgh));
      const usize payload_end   = reinterpret_cast<usize>(o) + buf_len;

      return static_cast<u32>(payload_end - payload_start);
    }

  /* ================================================================
   *                       REGISTER API
   * ================================================================ */
   
   /* ------ Input Output User Rings ------ */
    res_t register_ring_fd() noexcept;
    res_t unregister_ring_fd() noexcept;
    res_t close_ring_fd() noexcept;

    res_t register_probe(::io_uring_probe* p, u32 nr_ops) noexcept;
    res_t register_restrictions(::io_uring_restriction* res, u32 nr_res) noexcept;
    res_t enable_rings() noexcept; 

#ifdef _GNU_SOURCE
    res_t register_iowq_aff() noexcept;
#endif // _GNU_SOURCE
    res_t register_iowq_max_workers(u32* values) noexcept;
    res_t unregister_iowq_aff() noexcept;

    res_t register_region(::io_uring_mem_region_reg* reg) noexcept;
      
    res_t register_sync_cancel(const ::io_uring_sync_cancel_reg* reg) noexcept;
    res_t register_wait_reg(::io_uring_reg_wait* reg, i32 nr) noexcept;

    res_t register_sync_msg(io_uring_sqe* sqe) noexcept;

    res_t register_eventfd(i32 efd) noexcept;
    res_t register_eventfd_async(i32 efd) noexcept;
    res_t unregister_eventfd() noexcept;

    res_t register_clock(::io_uring_clock_register* arg) noexcept;

    /* ------ Files  ------ */
    res_t register_file_alloc_range(u32 off, u32 len) noexcept;

    res_t register_files(const i32* files, u32 nr_files) noexcept;
    res_t register_files_tags(const i32* files, const u64* tags, u32 nr) noexcept; 
    res_t register_files_sparse(u32 nr) noexcept;
    res_t register_files_update_tag(u32 off, const i32* files, const u64* tags, u32 nr_files) noexcept; 
    res_t register_files_update(u32 off, const i32* files, u32 nr_files) noexcept;
    res_t unregister_files() noexcept; 

    /* ------ IO Buffers  ------ */
    res_t register_buffers(const iovec* iovecs, u32 nr_iovecs) noexcept;
    res_t unregister_buffers() noexcept;

    res_t register_buf_ring(::io_uring_buf_reg* reg, u32 flags) noexcept;
    res_t unregister_buf_ring(i32 bgid) noexcept;

    res_t register_buffers_tags(const iovec* iovecs, const u64* tags, u32 nr) noexcept;
    res_t register_buffers_update_tag(u32 off, const iovec* iovecs, const u64* tags, u32 nr) noexcept;

    res_t register_buffers_sparse(u32 nr) noexcept;

    res_t register_buf_status(u32 buf_group, u32* head_out) noexcept;
    res_t register_clone_buffers(const io_uring& source, u32 dst_off, u32 src_off, u32 nr, u32 flags) noexcept;

    /* ------ NAPI / ZC/IFQ / BPF  ------ */
    res_t register_napi(::io_uring_napi* napi) noexcept;
    res_t unregister_napi(::io_uring_napi* napi) noexcept;

    res_t register_personality() noexcept;
    res_t unregister_personality(i32 id) noexcept;

    res_t register_ifq(::io_uring_zcrx_ifq_reg* reg) noexcept;

    // res_t register_bpf_filter(::io_uring_bpf* bpf) noexcept;
    // res_t register_bpf_filter_task(::io_uring_bpf* bpf) noexcept;

  /* ================================================================
   *                          PREP
   * ================================================================ */
    static inline void prep_rw(io_uring_sqe* sqe, int op, int fd, const void* addr, u32 len, u64 off) noexcept {
      sqe->opcode = static_cast<u8>(op);
      sqe->fd  = fd;
      sqe->off = off;
      sqe->addr = reinterpret_cast<u64>(addr);
      sqe->len = len;
      return;
    }

    static inline void set_target_fixed_file(io_uring_sqe* sqe, u32 file_index) noexcept {
      if (file_index == IORING_FILE_INDEX_ALLOC) --file_index;
      sqe->file_index = file_index + 1;
    }

    // NOP
    static inline void prep_nop(io_uring_sqe* sqe) noexcept {
      prep_rw(sqe, IORING_OP_NOP, -1, nullptr, 0, 0);
    }

    static inline void prep_nop128(io_uring_sqe* sqe) noexcept {
      prep_rw(sqe, IORING_OP_NOP128, -1, nullptr, 0, 0);
    }

  /* ================================================================
   *                          FILE IO 
   * ================================================================ */
    /* ------ Open / Close ------ */
    static inline void prep_openat(io_uring_sqe* sqe, i32 dfd, const char* path, i32 flags, u32 mode) noexcept {
      prep_rw(sqe, IORING_OP_OPENAT, dfd, path, mode, 0);
      sqe->open_flags = static_cast<u32>(flags);
    }

    static inline void prep_openat_direct(io_uring_sqe* sqe, i32 dfd, const char* path, i32 flags, u32 mode, u32 file_index) noexcept {
      prep_openat(sqe, dfd, path, flags, mode);
      set_target_fixed_file(sqe, file_index);
    }
    
    static inline void prep_open(io_uring_sqe* sqe, const char* path, i32 flags, u32 mode) noexcept {
      prep_openat(sqe, AT_FDCWD, path, flags, mode);
    }

    static inline void prep_open_direct(io_uring_sqe* sqe, const char* path, i32 flags, u32 mode, u32 file_index) noexcept {
      prep_openat_direct(sqe, AT_FDCWD, path, flags, mode, file_index);
    }

    static inline void prep_openat2(io_uring_sqe* sqe, i32 dfd, const char* path, const ::open_how* how) noexcept {
      prep_rw(sqe, IORING_OP_OPENAT2, dfd, path, sizeof(::open_how), reinterpret_cast<u64>(how));
    }

    static inline void prep_openat2_direct(io_uring_sqe* sqe, i32 dfd, const char* path, const ::open_how* how, u32 file_index) noexcept {
      prep_openat2(sqe, dfd, path, how);
      set_target_fixed_file(sqe, file_index);
    }

    static inline void prep_close(io_uring_sqe* sqe, i32 fd) noexcept {
      prep_rw(sqe, IORING_OP_CLOSE, fd, nullptr, 0, 0);
    }

    static inline void prep_close_direct(io_uring_sqe* sqe, u32 file_index) noexcept {
      prep_close(sqe, 0);

      set_target_fixed_file(sqe, file_index);
    }

    /* ------ Read / Write ------ */
    static inline void prep_read(io_uring_sqe* sqe, i32 fd, void* buf, u32 nbytes, u64 offset) noexcept {
      prep_rw(sqe, IORING_OP_READ, fd, buf, nbytes, offset);
    }

    static inline void prep_read_multishot(io_uring_sqe* sqe, i32 fd, u32 nbytes, u64 offset, i32 buf_group) noexcept {
      prep_rw(sqe, IORING_OP_READ_MULTISHOT, fd, nullptr, nbytes, offset);

      sqe->buf_group = static_cast<u16>(buf_group);
      sqe->flags = IOSQE_BUFFER_SELECT;
    }

    static inline void prep_readv(io_uring_sqe* sqe, i32 fd, const iovec* iovecs, u32 nr_vecs, u64 offset) noexcept {
      prep_rw(sqe, IORING_OP_READV, fd, iovecs, nr_vecs, offset);
    }

    static inline void prep_readv2(io_uring_sqe* sqe, i32 fd, const iovec* iovecs, u32 nr_vecs, u64 offset, i32 flags) noexcept {
      prep_readv(sqe, fd, iovecs, nr_vecs, offset);

      sqe->rw_flags = flags;
    }

    static inline void prep_read_fixed(io_uring_sqe* sqe, i32 fd, void* buf, u32 nbytes, u64 offset, i32 buf_index) noexcept {
      prep_rw(sqe, IORING_OP_READ_FIXED, fd, buf, nbytes, offset);

      sqe->buf_index = static_cast<u16>(buf_index);
    }

    static inline void prep_readv_fixed(io_uring_sqe* sqe, i32 fd, const iovec* iovecs, u32 nr_vecs, u64 offset, i32 flags, i32 buf_index) noexcept {
      prep_readv2(sqe, fd, iovecs, nr_vecs, offset, flags);

      sqe->opcode = IORING_OP_READV_FIXED;
      sqe->buf_index = static_cast<u16>(buf_index);
    }

    static inline void prep_write(io_uring_sqe* sqe, i32 fd, const void* buf, u32 nbytes, u64 offset) noexcept {
      prep_rw(sqe, IORING_OP_WRITE, fd, buf, nbytes, offset);
    }

    static inline void prep_writev(io_uring_sqe* sqe, i32 fd, const iovec* iovecs, u32 nr_vecs, u64 offset) noexcept {
      prep_rw(sqe, IORING_OP_WRITEV, fd, iovecs, nr_vecs, offset);
    }

    static inline void prep_writev2(io_uring_sqe* sqe, i32 fd, const iovec* iovecs, u32 nr_vecs, u64 offset, i32 flags) noexcept {
      prep_writev(sqe, fd, iovecs, nr_vecs, offset);

      sqe->rw_flags = flags;
    }

    static inline void prep_write_fixed(io_uring_sqe* sqe, i32 fd, const void* buf, u32 nbytes, u64 offset, i32 buf_index) noexcept {
      prep_rw(sqe, IORING_OP_WRITE_FIXED, fd, buf, nbytes, offset);

      sqe->buf_index = static_cast<u16>(buf_index);
    }

    static inline void prep_writev_fixed(io_uring_sqe* sqe, i32 fd, const iovec* iovecs, u32 nr_vecs, u64 offset, i32 flags, i32 buf_index) noexcept {
      prep_writev2(sqe, fd, iovecs, nr_vecs, offset, flags);

      sqe->opcode = IORING_OP_WRITEV_FIXED;
      sqe->buf_index = static_cast<u16>(buf_index);
    }

    /* ------ Sync & Metadata ------ */
    static inline void prep_fsync(io_uring_sqe* sqe, i32 fd, u32 fsync_flags) noexcept {
      prep_rw(sqe, IORING_OP_FSYNC, fd, nullptr, 0, 0);

      sqe->fsync_flags = fsync_flags;
    }

    static inline void prep_sync_file_range(io_uring_sqe* sqe, i32 fd, u32 len, u64 offset, i32 flags) noexcept {
      prep_rw(sqe, IORING_OP_SYNC_FILE_RANGE, fd, nullptr, len, offset);

      sqe->sync_range_flags = static_cast<u32>(flags);
    }

    static inline void prep_fallocate(io_uring_sqe* sqe, i32 fd, i32 mode, u64 offset, u64 len) noexcept {
      prep_rw(sqe, IORING_OP_FALLOCATE, fd, nullptr, static_cast<u32>(mode), offset);

      sqe->addr = len;
    }

    static inline void prep_ftruncate(io_uring_sqe* sqe, i32 fd, u64 len) noexcept {
      prep_rw(sqe, IORING_OP_FTRUNCATE, fd, nullptr, 0, len);
    }

    struct statx; // Forward declaration
    static inline void prep_statx(io_uring_sqe* sqe, i32 dfd, const char* path, i32 flags, u32 mask, statx* statxbuf) noexcept {
      prep_rw(sqe, IORING_OP_STATX, dfd, path, mask, reinterpret_cast<u64>(statxbuf));

      sqe->statx_flags = static_cast<u32>(flags);
    }

    /* ------ Advise ------ */
    static inline void prep_fadvise(io_uring_sqe* sqe, i32 fd, u64 offset, u32 len, i32 advice) noexcept {
      prep_rw(sqe, IORING_OP_FADVISE, fd, nullptr, len, offset);

      sqe->fadvise_advice = static_cast<u32>(advice);
    }

    static inline void prep_fadvise64(io_uring_sqe* sqe, i32 fd, u64 offset, u64 len, i32 advice) noexcept {
      prep_rw(sqe, IORING_OP_FADVISE, fd, nullptr, 0, offset);

      sqe->addr = len;
      sqe->fadvise_advice = static_cast<u32>(advice);
    }

    static inline void prep_madvise(io_uring_sqe* sqe, void* addr, u32 length, i32 advice) noexcept {
      prep_rw(sqe, IORING_OP_MADVISE, -1, addr, length, 0);

      sqe->fadvise_advice = static_cast<u32>(advice);
    }

    static inline void prep_madvise64(io_uring_sqe* sqe, void* addr, u64 length, i32 advice) noexcept {
      prep_rw(sqe, IORING_OP_MADVISE, -1, addr, 0, length);
      
      sqe->fadvise_advice = static_cast<u32>(advice);
    }

    /* ------ Namespace ops ------ */
    static inline void prep_linkat(io_uring_sqe* sqe, i32 olddfd, const char* oldpath, i32 newdfd, const char* newpath, i32 flags) noexcept {
      prep_rw(sqe, IORING_OP_LINKAT, olddfd, oldpath, static_cast<u32>(newdfd), reinterpret_cast<u64>(newpath));

      sqe->hardlink_flags = static_cast<u32>(flags);
    }

    static inline void prep_link(io_uring_sqe* sqe, const char* oldpath, const char* newpath, i32 flags) noexcept {
      prep_linkat(sqe, AT_FDCWD, oldpath, AT_FDCWD, newpath, flags);
    }

    static inline void prep_symlinkat(io_uring_sqe* sqe, const char* target, i32 newdirfd, const char* linkpath) noexcept {
      prep_rw(sqe, IORING_OP_SYMLINKAT, newdirfd, target, 0, reinterpret_cast<u64>(linkpath));
    }

    static inline void prep_symlink(io_uring_sqe* sqe, const char* target, const char* linkpath) noexcept {
      prep_symlinkat(sqe, target, AT_FDCWD, linkpath);
    }

    static inline void prep_unlinkat(io_uring_sqe* sqe, i32 dfd, const char* path, i32 flags) noexcept {
      prep_rw(sqe, IORING_OP_UNLINKAT, dfd, path, 0, 0);

      sqe->unlink_flags = static_cast<u32>(flags);
    }

    static inline void prep_unlink(io_uring_sqe* sqe, const char* path, i32 flags) noexcept {
      prep_unlinkat(sqe, AT_FDCWD, path, flags);
    }

    static inline void prep_mkdirat(io_uring_sqe* sqe, i32 dfd, const char* path, u32 mode) noexcept {
      prep_rw(sqe, IORING_OP_MKDIRAT, dfd, path, mode, 0);
    }

    static inline void prep_mkdir(io_uring_sqe* sqe, const char* path, u32 mode) noexcept {
      prep_mkdirat(sqe, AT_FDCWD, path, mode);
    }

    static inline void prep_renameat(io_uring_sqe* sqe, i32 olddfd, const char* oldpath, i32 newdfd, const char* newpath, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_RENAMEAT, olddfd, oldpath, static_cast<u32>(newdfd), reinterpret_cast<u64>(newpath));

      sqe->rename_flags = flags;
    }

    static inline void prep_rename(io_uring_sqe* sqe, const char* oldpath, const char* newpath) noexcept {
      prep_renameat(sqe, AT_FDCWD, oldpath, AT_FDCWD, newpath, 0);
    }

    /* ------ Extended attributes ------ */
    static inline void prep_getxattr(io_uring_sqe* sqe, const char* name, char* value, const char* path, u32 len) noexcept {
      prep_rw(sqe, IORING_OP_GETXATTR, 0, name, len, reinterpret_cast<u64>(value));

      sqe->addr3 = reinterpret_cast<u64>(path);
      sqe->xattr_flags = 0;
    } 

    static inline void prep_setxattr(io_uring_sqe* sqe, const char* name, const char* value, const char* path, i32 flags, u32 len) noexcept {
      prep_rw(sqe, IORING_OP_SETXATTR, 0, name, len, reinterpret_cast<u64>(value));

      sqe->addr3 = reinterpret_cast<u64>(path);
      sqe->xattr_flags = flags;
    }

    static inline void prep_fgetxattr(io_uring_sqe* sqe, i32 fd, const char* name, char* value, u32 len) noexcept {
      prep_rw(sqe, IORING_OP_FGETXATTR, fd, name, len, reinterpret_cast<u64>(value));

      sqe->xattr_flags = 0;
    }

    static inline void prep_fsetxattr(io_uring_sqe* sqe, i32 fd, const char* name, const char* value, i32 flags, u32 len) noexcept {
      prep_rw(sqe, IORING_OP_FSETXATTR, fd, name, len, reinterpret_cast<u64>(value));

      sqe->xattr_flags = flags;
    }

  /* ================================================================
   *                          Network 
   * ================================================================ */

    /* ------ Socket lifecycle ------ */
    static inline void prep_socket(io_uring_sqe* sqe, i32 domain, i32 type, i32 protocol, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_SOCKET, domain, nullptr, static_cast<u32>(protocol), static_cast<u64>(type));

      sqe->rw_flags = flags;
    }

    static inline void prep_socket_direct(io_uring_sqe* sqe, i32 domain, i32 type, i32 protocol, u32 file_index, u32 flags) noexcept {
      prep_socket(sqe, domain, type, protocol, flags);
      set_target_fixed_file(sqe, file_index);
    }

    static inline void prep_socket_direct_alloc(io_uring_sqe* sqe, i32 domain, i32 type, i32 protocol, u32 flags) noexcept {
      prep_socket(sqe, domain, type, protocol, flags);

      set_target_fixed_file(sqe, IORING_FILE_INDEX_ALLOC);
    }

    static inline void prep_bind(io_uring_sqe* sqe, i32 fd, const sockaddr* addr, u32 addrlen) noexcept {
      prep_rw(sqe, IORING_OP_BIND, fd, addr, 0, addrlen);
    }

    static inline void prep_listen(io_uring_sqe* sqe, i32 fd, i32 backlog) noexcept {
      prep_rw(sqe, IORING_OP_LISTEN, fd, nullptr, static_cast<u32>(backlog), 0);
    }

    static inline void prep_connect(io_uring_sqe* sqe, i32 fd, const sockaddr* addr, u32 addrlen) noexcept {
      prep_rw(sqe, IORING_OP_CONNECT, fd, addr, 0, addrlen);
    }

    static inline void prep_shutdown(io_uring_sqe* sqe, i32 fd, i32 how) noexcept {
      prep_rw(sqe, IORING_OP_SHUTDOWN, fd, nullptr, static_cast<u32>(how), 0);
    }

    /* ------ Accept ------ */
    static inline void prep_accept(io_uring_sqe* sqe, i32 fd, sockaddr* addr, u32* addrlen, i32 flags) noexcept {
      prep_rw(sqe, IORING_OP_ACCEPT, fd, addr, 0, reinterpret_cast<u64>(addrlen));

      sqe->accept_flags = static_cast<u32>(flags);
    }

    static inline void prep_accept_direct(io_uring_sqe* sqe, i32 fd, sockaddr* addr, u32* addrlen, i32 flags, u32 file_index) noexcept {
      prep_accept(sqe, fd, addr, addrlen, flags);
      set_target_fixed_file(sqe, file_index);
    }

    static inline void prep_accept_multishot(io_uring_sqe* sqe, i32 fd, sockaddr* addr, u32* addrlen, i32 flags) noexcept {
      prep_accept(sqe, fd, addr, addrlen, flags);

      sqe->ioprio |= IORING_ACCEPT_MULTISHOT;
    }

    static inline void prep_multishot_accept_direct(io_uring_sqe* sqe, i32 fd, sockaddr* addr, u32* addrlen, i32 flags) noexcept {
      prep_accept_multishot(sqe, fd, addr, addrlen, flags);

      set_target_fixed_file(sqe, IORING_FILE_INDEX_ALLOC);
    }

    /* ------ Send ------ */
    static inline void prep_send(io_uring_sqe* sqe, i32 sockfd, const void* buf, u32 len, i32 flags) noexcept {
      prep_rw(sqe, IORING_OP_SEND, sockfd, buf, len, 0);

      sqe->msg_flags = static_cast<u32>(flags);
    }

    static inline void prep_send_bundle(io_uring_sqe* sqe, i32 sockfd, u32 len, i32 flags) noexcept {
      prep_send(sqe, sockfd, nullptr, len, flags);

      sqe->ioprio |= IORING_RECVSEND_BUNDLE;
    }

    static inline void prep_send_set_addr(io_uring_sqe* sqe, const sockaddr* dest_addr, u16 addr_len) noexcept {
      sqe->addr2 = reinterpret_cast<u64>(dest_addr);
      sqe->addr_len = addr_len;
    }

    static inline void prep_sendto(io_uring_sqe* sqe, i32 sockfd, const void* buf, u32 len, i32 flags, const sockaddr* addr, u32 addrlen) noexcept {
      prep_send(sqe, sockfd, buf, len, flags);
      prep_send_set_addr(sqe, addr, static_cast<u16>(addrlen));
    }

    static inline void prep_send_zc(io_uring_sqe* sqe, i32 sockfd, const void* buf, u32 len, i32 flags, u32 zc_flags) noexcept {
      prep_rw(sqe, IORING_OP_SEND_ZC, sockfd, buf, len, 0);

      sqe->msg_flags = static_cast<u32>(flags);
      sqe->ioprio = static_cast<u16>(zc_flags);
    }

    static inline void prep_send_zc_fixed(io_uring_sqe* sqe, i32 sockfd, const void* buf, u32 len, i32 flags, u32 zc_flags, u32 buf_index) noexcept {
      prep_send_zc(sqe, sockfd, buf, len, flags, zc_flags);

      sqe->ioprio |= IORING_RECVSEND_FIXED_BUF;
      sqe->buf_index = static_cast<u16>(buf_index);
    }

    static inline void prep_sendmsg(io_uring_sqe* sqe, i32 fd, const msghdr* msg, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_SENDMSG, fd, msg, 1, 0);

      sqe->msg_flags = flags;
    }

    static inline void prep_sendmsg_zc(io_uring_sqe* sqe, i32 fd, const msghdr* msg, u32 flags) noexcept {
      prep_sendmsg(sqe, fd, msg, flags);

      sqe->opcode = IORING_OP_SENDMSG_ZC;
    }

    static inline void prep_sendmsg_zc_fixed(io_uring_sqe* sqe, i32 fd, const msghdr* msg, u32 flags, u32 buf_index) noexcept {
      prep_sendmsg_zc(sqe, fd, msg, flags);

      sqe->ioprio |= IORING_RECVSEND_FIXED_BUF;
      sqe->buf_index = static_cast<u16>(buf_index);
    }

    /* ------ Recv ------ */
    static inline void prep_recv(io_uring_sqe* sqe, i32 sockfd, void* buf, u32 len, i32 flags) noexcept {
      prep_rw(sqe, IORING_OP_RECV, sockfd, buf, len, 0);

      sqe->msg_flags = static_cast<u32>(flags);
    }

    static inline void prep_recv_multishot(io_uring_sqe* sqe, i32 sockfd, void* buf, u32 len, i32 flags) noexcept {
      prep_recv(sqe, sockfd, buf, len, flags);

      sqe->ioprio |= IORING_RECV_MULTISHOT;
    }

    static inline void prep_recv_bundle(io_uring_sqe* sqe, i32 sockfd, u32 len, i32 flags) noexcept {
      prep_recv(sqe, sockfd, nullptr, len, flags);

      sqe->ioprio |= IORING_RECVSEND_BUNDLE;
    }

    static inline void prep_recvmsg(io_uring_sqe* sqe, i32 fd, msghdr* msg, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_RECVMSG, fd, msg, 1, 0);

      sqe->msg_flags = flags;
    }

    static inline void prep_recvmsg_multishot(io_uring_sqe* sqe, i32 fd, msghdr* msg, u32 flags) noexcept {
      prep_recvmsg(sqe, fd, msg, flags);

      sqe->ioprio |= IORING_RECV_MULTISHOT;
    }

  /* ================================================================
   *                         Polling & Events 
   * ================================================================ */

    /* ------ Poll ------ */
    static inline void prep_poll_add(io_uring_sqe* sqe, i32 fd, u32 poll_mask) noexcept {
      prep_rw(sqe, IORING_OP_POLL_ADD, fd, nullptr, 0, 0);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      poll_mask = __builtin_bswap32(poll_mask) >> 16;
#endif
      sqe->poll32_events = poll_mask;
    }

    static inline void prep_poll_multishot(io_uring_sqe* sqe, i32 fd, u32 poll_mask) noexcept {
      prep_poll_add(sqe, fd, poll_mask);
      sqe->len = IORING_POLL_ADD_MULTI;
    }

    static inline void prep_poll_update(io_uring_sqe* sqe, u64 old_user_data, u64 new_user_data, u32 poll_mask, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_POLL_REMOVE, -1, nullptr, flags, new_user_data);

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      poll_mask = __builtin_bswap32(poll_mask) >> 16;
#endif
      sqe->addr = old_user_data;
      sqe->poll32_events = poll_mask;
    }

    static inline void prep_poll_remove(io_uring_sqe* sqe, u64 user_data) noexcept {
      prep_rw(sqe, IORING_OP_POLL_REMOVE, -1, nullptr, 0, 0);
      sqe->addr = user_data;
    }

    /* ------ Epoll ------ */
    struct epoll_event; // Forward Declaration
    static inline void prep_epoll_ctl(io_uring_sqe* sqe, i32 epfd, i32 fd, i32 op, epoll_event* ev) noexcept {
      prep_rw(sqe, IORING_OP_EPOLL_CTL, epfd, ev, static_cast<u32>(op), static_cast<u32>(fd));
    }

    static inline void prep_epoll_wait(io_uring_sqe* sqe, i32 fd, epoll_event* events, i32 maxevents, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_EPOLL_WAIT, fd, events, static_cast<u32>(maxevents), 0);
      sqe->rw_flags = flags;
    }

    /* ------ Timer ------ */
    static inline void prep_timeout(io_uring_sqe* sqe, const __kernel_timespec* ts, u32 count, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_TIMEOUT, -1, ts, 1, count);
      sqe->timeout_flags = flags;
    }

    static inline void prep_timeout_update(io_uring_sqe* sqe, const __kernel_timespec* ts, u64 user_data, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_TIMEOUT_REMOVE, -1, nullptr, 0, reinterpret_cast<u64>(ts));

      sqe->addr = user_data;
      sqe->timeout_flags = flags | IORING_TIMEOUT_UPDATE;
    }

    static inline void prep_link_timeout(io_uring_sqe* sqe, const __kernel_timespec* ts, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_LINK_TIMEOUT, -1, ts, 1, 0);

      sqe->timeout_flags = flags;
    }

    static inline void prep_timeout_remove(io_uring_sqe* sqe, u64 user_data, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_TIMEOUT_REMOVE, -1, nullptr, 0, 0);

      sqe->addr = user_data;
      sqe->timeout_flags = flags;
    }

    /* ------ Cancellation ------ */
    static inline void prep_cancel64(io_uring_sqe* sqe, u64 user_data, i32 flags) noexcept {
      prep_rw(sqe, IORING_OP_ASYNC_CANCEL, -1, nullptr, 0, 0);
      sqe->addr = user_data;
      sqe->cancel_flags = static_cast<u32>(flags);
    }

    static inline void prep_cancel(io_uring_sqe* sqe, const void* user_data, i32 flags) noexcept {
      prep_cancel64(sqe, reinterpret_cast<u64>(user_data), flags);
    }

    static inline void prep_cancel_fd(io_uring_sqe* sqe, i32 fd, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_ASYNC_CANCEL, fd, nullptr, 0, 0);

      sqe->cancel_flags = flags | IORING_ASYNC_CANCEL_FD;
    }

    /* ------ Buffers ------ */
    static inline void prep_provide_buffers(io_uring_sqe* sqe, void* addr, i32 len, i32 nr, i32 bgid, i32 bid) noexcept {
      prep_rw(sqe, IORING_OP_PROVIDE_BUFFERS, nr, addr, static_cast<u32>(len), static_cast<u64>(bid));

      sqe->buf_group = static_cast<u16>(bgid);
    }

    static inline void prep_remove_buffers(io_uring_sqe* sqe, i32 nr, i32 bgid) noexcept {
      prep_rw(sqe, IORING_OP_REMOVE_BUFFERS, nr, nullptr, 0, 0);

      sqe->buf_group = static_cast<u16>(bgid);
    }

  /* ============================================================
   *   Splice & Pipe
   * ============================================================ */

    static inline void prep_splice(io_uring_sqe* sqe, i32 fd_in, i64 off_in, i32 fd_out, i64 off_out, u32 nbytes, u32 splice_flags) noexcept {
      prep_rw(sqe, IORING_OP_SPLICE, fd_out, nullptr, nbytes, static_cast<u64>(off_out));

      sqe->splice_off_in = static_cast<u64>(off_in);
      sqe->splice_fd_in = fd_in;
      sqe->splice_flags = splice_flags;
    }

    static inline void prep_tee(io_uring_sqe* sqe, i32 fd_in, i32 fd_out, u32 nbytes, u32 splice_flags) noexcept {
      prep_rw(sqe, IORING_OP_TEE, fd_out, nullptr, nbytes, 0);

      sqe->splice_off_in = 0;
      sqe->splice_fd_in = fd_in;
      sqe->splice_flags = splice_flags;
    }

    static inline void prep_pipe(io_uring_sqe* sqe, i32* fds, i32 pipe_flags) noexcept {
      prep_rw(sqe, IORING_OP_PIPE, 0, fds, 0, 0);

      sqe->pipe_flags = static_cast<u32>(pipe_flags);
    }

    static inline void prep_pipe_direct(io_uring_sqe* sqe, i32* fds, i32 pipe_flags, u32 file_index) noexcept {
      prep_pipe(sqe, fds, pipe_flags);
      set_target_fixed_file(sqe, file_index);
    }

  /* ============================================================
   *   Futex
   * ============================================================ */

    static inline void prep_futex_wake(io_uring_sqe* sqe, const u32* futex, u64 val, u64 mask, u32 futex_flags, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_FUTEX_WAKE, static_cast<i32>(futex_flags), futex, 0, val);

      sqe->futex_flags = flags;
      sqe->addr3 = mask;
    }

    static inline void prep_futex_wait(io_uring_sqe* sqe, const u32* futex, u64 val, u64 mask, u32 futex_flags, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_FUTEX_WAIT, static_cast<i32>(futex_flags), futex, 0, val);

      sqe->futex_flags = flags;
      sqe->addr3 = mask;
    }

    struct futex_waitv; // Forward Declaration
    static inline void prep_futex_waitv(io_uring_sqe* sqe, const futex_waitv* futex, u32 nr_futex, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_FUTEX_WAITV, 0, futex, nr_futex, 0);

      sqe->futex_flags = flags;
    }

  /* ============================================================
   *   Ring messaging
   * ============================================================ */

    static inline void prep_msg_ring(io_uring_sqe* sqe, i32 fd, u32 len, u64 data, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_MSG_RING, fd, nullptr, len, data);

      sqe->msg_ring_flags = flags;
    }

    static inline void prep_msg_ring_cqe_flags(io_uring_sqe* sqe, i32 fd, u32 len, u64 data, u32 flags, u32 cqe_flags) noexcept {
      prep_rw(sqe, IORING_OP_MSG_RING, fd, nullptr, len, data);

      sqe->msg_ring_flags = IORING_MSG_RING_FLAGS_PASS | flags;
      sqe->file_index = cqe_flags;
    }

    static inline void prep_msg_ring_fd(io_uring_sqe* sqe, i32 fd, i32 source_fd, i32 target_fd, u64 data, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_MSG_RING, fd, reinterpret_cast<const void*>(static_cast<unsigned long>(IORING_MSG_SEND_FD)), 0, data);
      sqe->addr3 = static_cast<u64>(source_fd);

      set_target_fixed_file(sqe, static_cast<u32>(target_fd));
      sqe->msg_ring_flags = flags;
    }

    static inline void prep_msg_ring_fd_alloc(io_uring_sqe* sqe, i32 fd, i32 source_fd, u64 data, u32 flags) noexcept {
      prep_msg_ring_fd(sqe, fd, source_fd, static_cast<i32>(IORING_FILE_INDEX_ALLOC), data, flags);
    }

  /* ============================================================
   *   URing command
   * ============================================================ */

    static inline void prep_uring_cmd(io_uring_sqe* sqe, i32 cmd_op, i32 fd) noexcept {
      sqe->opcode = IORING_OP_URING_CMD;
      sqe->fd = fd;
      sqe->cmd_op = static_cast<u32>(cmd_op);
      sqe->__pad1 = 0;
      sqe->addr = 0;
      sqe->len = 0;
    }

    static inline void prep_uring_cmd128(io_uring_sqe* sqe, i32 cmd_op, i32 fd) noexcept {
      sqe->opcode = IORING_OP_URING_CMD128;
      sqe->fd = fd;
      sqe->cmd_op = static_cast<u32>(cmd_op);
      sqe->__pad1 = 0;
      sqe->addr = 0;
      sqe->len = 0;
    }

    static inline void prep_cmd_sock(io_uring_sqe* sqe, i32 cmd_op, i32 fd, i32 level, i32 optname, void* optval, i32 optlen) noexcept {
      prep_uring_cmd(sqe, cmd_op, fd);
      sqe->optval = reinterpret_cast<u64>(optval);
      sqe->optname = static_cast<u32>(optname);
      sqe->optlen = static_cast<u32>(optlen);
      sqe->level = static_cast<u32>(level);
    }

    static inline void prep_cmd_getsockname(io_uring_sqe* sqe, i32 fd, sockaddr* addr, u32* addrlen, i32 peer) noexcept {
      prep_uring_cmd(sqe, SOCKET_URING_OP_GETSOCKNAME, fd);
      sqe->addr = reinterpret_cast<u64>(addr);
      sqe->addr3 = reinterpret_cast<u64>(addrlen);
      sqe->optlen = static_cast<u32>(peer);
    }

    static inline void prep_cmd_setsockopt(io_uring_sqe* sqe, i32 fd, i32 level, i32 optname, const void* optval, i32 optlen) noexcept {
      prep_cmd_sock(sqe, SOCKET_URING_OP_SETSOCKOPT, fd, level, optname, const_cast<void*>(optval), optlen);
    }

    static inline void prep_cmd_getsockopt(io_uring_sqe* sqe, i32 fd, i32 level, i32 optname, void* optval, i32 optlen) noexcept {
      prep_cmd_sock(sqe, SOCKET_URING_OP_GETSOCKOPT, fd, level, optname, optval, optlen);
    }

    static inline void prep_cmd_discard(io_uring_sqe* sqe, i32 fd, u64 offset, u64 nbytes) noexcept {
      constexpr u32 BLOCK_URING_CMD_DISCARD = 0; // Mirrored kernel constant
      prep_uring_cmd(sqe, BLOCK_URING_CMD_DISCARD, fd);

      sqe->addr = offset;
      sqe->addr3 = nbytes;
    }

  /* ============================================================
   *   Misc
   * ============================================================ */

    static inline void prep_waitid(io_uring_sqe* sqe, i32 idtype, u32 id, void* infop, i32 options, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_WAITID, static_cast<i32>(id), nullptr, static_cast<u32>(idtype), 0);

      sqe->waitid_flags = flags;
      sqe->file_index = static_cast<u32>(options);
      sqe->addr2 = reinterpret_cast<u64>(infop);
    }

    static inline void prep_files_update(io_uring_sqe* sqe, i32* fds, u32 nr_fds, i32 offset) noexcept {
      prep_rw(sqe, IORING_OP_FILES_UPDATE, -1, fds, nr_fds, static_cast<u64>(offset));
    }

    static inline void prep_fixed_fd_install(io_uring_sqe* sqe, i32 fd, u32 flags) noexcept {
      prep_rw(sqe, IORING_OP_FIXED_FD_INSTALL, fd, nullptr, 0, 0);
      
      sqe->flags = IOSQE_FIXED_FILE;
      sqe->install_fd_flags = flags;
    }

    // Buffers 
    inline i32  buf_ring_mask(u32 ring_entries) noexcept;
    inline void buf_ring_init(io_uring_buf_ring* br) noexcept;
    inline void buf_ring_add(io_uring_buf_ring* br, void* addr, u32 len, u16 bid, i32 mask, i32 buf_offset) noexcept;
    inline void buf_ring_advance(io_uring_buf_ring* br, i32 count) noexcept;

    // Recvmsg helpers
    inline io_uring_recvmsg_out* recvmsg_validate(void* buf, i32 buf_len, msghdr* msgh) noexcept;
    inline void*    recvmsg_name(io_uring_recvmsg_out* o) noexcept;
    inline cmsghdr* recvmsg_cmsg_firsthdr(io_uring_recvmsg_out* o, msghdr* msgh) noexcept;
    inline cmsghdr* recvmsg_cmsg_nexthdr(io_uring_recvmsg_out* o, msghdr* msgh, cmsghdr* cmsg) noexcept;
    inline void*    recvmsg_payload(io_uring_recvmsg_out* o, msghdr* msgh) noexcept;
    inline u32      recvmsg_payload_length(io_uring_recvmsg_out* o, i32 buf_len, msghdr* msgh) noexcept;

    // Register

  }; // class io_uring

} // namespace nostd

#endif // NO_STD_IO_URING_HPP