#include "io_uring.hpp"
#include "syscall.hpp"
#include "sysno.hpp"
#include "error.hpp"
#include <linux/io_uring.h>

using namespace nostd;

res_t io_uring::do_register(u32 opcode, const void* arg, u32 nr_args) noexcept {
  u32 op = opcode;
  i32 fd = _ringfd;

  /* Like liburing do_register: only after IORING_FEAT_REG_REG_RING + register_ring_fd. */
  if (_int_flags & INT_FLAG_REG_REG_RING) {
    op |= IORING_REGISTER_USE_REGISTERED_RING;
    fd = _enterfd;
  }

  return io_uring_register(fd, op, arg, nr_args);
}

res_t io_uring::register_ring_fd() noexcept {
  if (_int_flags & INT_FLAG_REG_RING) return E::EXIST;

  io_uring_rsrc_update up {
    .offset = static_cast<u32>(-1),
    .resv   = 0,
    .data   = static_cast<u64>(_ringfd)
  };


  res_t const ret = do_register(IORING_REGISTER_RING_FDS, &up, 1);
  if (ret == 1) {
    _enterfd = up.offset;
    _int_flags |= INT_FLAG_REG_RING;

    if (_features & IORING_FEAT_REG_REG_RING) {
      _int_flags |= INT_FLAG_REG_REG_RING;
    }

  }

  return ret;
}

res_t io_uring::unregister_ring_fd() noexcept {
  if (!(_int_flags & INT_FLAG_REG_RING)) return E::INVAL;

  io_uring_rsrc_update up {
    .offset = static_cast<u32>(_enterfd),
    .resv   = 0,
    .data   = 0
  };

  res_t const ret = do_register(IORING_UNREGISTER_RING_FDS, &up, 1);
  if (ret == 1) {
    _enterfd = _ringfd;
    _int_flags = _int_flags & ~(INT_FLAG_REG_RING | INT_FLAG_REG_REG_RING); 
  }

  return ret;
}

res_t io_uring::close_ring_fd() noexcept {
  if (!(_features & IORING_FEAT_REG_REG_RING)) return E::OPNOTSUPP;
  if (!(_int_flags & INT_FLAG_REG_RING))       return E::INVAL;
  if (_ringfd < 0)                              return E::BADF;

  (void) __nostd_syscall1(sysno::CLOSE, _ringfd);
  _ringfd = -1;
  return 1;
}

res_t io_uring::register_probe(::io_uring_probe* p, u32 nr_ops) noexcept {
  return do_register(IORING_REGISTER_PROBE, p, nr_ops);
}

res_t io_uring::register_restrictions(::io_uring_restriction* res, u32 nr_res) noexcept {
  return do_register(IORING_REGISTER_RESTRICTIONS, res, nr_res);
}

res_t io_uring::enable_rings() noexcept {
  return do_register(IORING_REGISTER_ENABLE_RINGS, nullptr, 0);
}

#ifdef _GNU_SOURCE 
// TODO
res_t io_uring::register_iowq_aff() noexcept {
  return E::NOSYS;
}
#endif // _GNU_SOURCE

res_t io_uring::register_iowq_max_workers(u32* values) noexcept {
  return do_register(IORING_REGISTER_IOWQ_MAX_WORKERS, values, 2);
}

res_t io_uring::unregister_iowq_aff() noexcept {
  return do_register(IORING_UNREGISTER_IOWQ_AFF, nullptr, 0);
}

res_t io_uring::register_region(io_uring_mem_region_reg* reg) noexcept {
  return do_register(IORING_REGISTER_MEM_REGION, reg, 1);
}

res_t io_uring::register_sync_cancel(const io_uring_sync_cancel_reg* reg) noexcept {
  return do_register(IORING_REGISTER_SYNC_CANCEL, reg, 1);
}

// TODO
res_t io_uring::register_wait_reg(io_uring_reg_wait* /*reg*/, i32 /*nr*/) noexcept {
  return E::NOSYS;
}

res_t io_uring::register_sync_msg(io_uring_sqe* sqe) noexcept {
  return io_uring_register(-1, IORING_REGISTER_SEND_MSG_RING, sqe, 1);
}

res_t io_uring::register_eventfd(i32 efd) noexcept {
  return do_register(IORING_REGISTER_EVENTFD, &efd, 1);
}

res_t io_uring::register_eventfd_async(i32 efd) noexcept {
  return do_register(IORING_REGISTER_EVENTFD_ASYNC, &efd, 1);
}

res_t io_uring::unregister_eventfd() noexcept {
  return do_register(IORING_UNREGISTER_EVENTFD, nullptr, 0);
}

res_t io_uring::register_clock(io_uring_clock_register* arg) noexcept {
  return do_register(IORING_REGISTER_CLOCK, arg, 0);
}

res_t io_uring::register_file_alloc_range(u32 off, u32 len) noexcept {
  io_uring_file_index_range range {
    .off  = off,
    .len  = len,
    .resv = 0
  };

  return do_register(IORING_REGISTER_FILE_ALLOC_RANGE, &range, 0);
}

res_t io_uring::register_files(const i32* files, u32 nr_files) noexcept {
  return do_register(IORING_REGISTER_FILES, files, nr_files);
}

res_t io_uring::register_files_tags(const i32* files, const u64* tags, u32 nr) noexcept {
  io_uring_rsrc_register reg {
    .nr    = nr,
    .flags = 0,
    .resv2 = 0,
    .data  = reinterpret_cast<u64>(files),
    .tags  = reinterpret_cast<u64>(tags)
  };

  return do_register(IORING_REGISTER_FILES2, &reg, sizeof(reg));
}

res_t io_uring::register_files_sparse(u32 nr) noexcept {
  io_uring_rsrc_register reg { 
    .nr    = nr,
    .flags = IORING_RSRC_REGISTER_SPARSE,
    .resv2 = 0,
    .data  = 0,
    .tags  = 0
  };

  return do_register(IORING_REGISTER_FILES2, &reg, sizeof(reg));
}

res_t io_uring::register_files_update(u32 off, const i32* files, u32 nr_files) noexcept {
  io_uring_rsrc_update up {
    .offset = off,
    .resv   = 0,
    .data   = reinterpret_cast<u64>(files),
  };

  return do_register(IORING_REGISTER_FILES_UPDATE, &up, nr_files);
}

res_t io_uring::register_files_update_tag(u32 off, const i32* files, const u64* tags, u32 nr) noexcept {
  io_uring_rsrc_update2 up {
    .offset = off,
    .resv   = 0,
    .data   = reinterpret_cast<u64>(files),
    .tags   = reinterpret_cast<u64>(tags),
    .nr     = nr,
    .resv2  = 0
  };

  return do_register(IORING_REGISTER_FILES_UPDATE2, &up, sizeof(up));
}

res_t io_uring::unregister_files() noexcept {
  return do_register(IORING_UNREGISTER_FILES, nullptr, 0);
}

res_t io_uring::register_buffers(const iovec* iovecs, u32 nr_iovecs) noexcept {
  return do_register(IORING_REGISTER_BUFFERS, iovecs, nr_iovecs);
}

res_t io_uring::unregister_buffers() noexcept {
  return do_register(IORING_UNREGISTER_BUFFERS, nullptr, 0);
}

res_t io_uring::register_buf_ring(io_uring_buf_reg* reg, u32 flags) noexcept {
  reg->flags |= flags;
  return do_register(IORING_REGISTER_PBUF_RING, reg, 1);
}

res_t io_uring::unregister_buf_ring(i32 bgid) noexcept {
  io_uring_buf_reg buf_reg {
    .ring_addr    = 0,
    .ring_entries = 0,
    .bgid         = static_cast<u16>(bgid),
    .flags        = 0,
    .resv         = {0,0,0}
  };

  return do_register(IORING_UNREGISTER_PBUF_RING, &buf_reg, 1);
}

res_t io_uring::register_buffers_tags(const iovec* iovecs, const u64* tags, u32 nr) noexcept {
  io_uring_rsrc_register reg { 
    .nr = nr, 
    .flags = 0, 
    .resv2 = 0,
    .data = reinterpret_cast<u64>(iovecs),
    .tags = reinterpret_cast<u64>(tags)
  };

  return do_register(IORING_REGISTER_BUFFERS2, &reg, sizeof(reg));
}

res_t io_uring::register_buffers_update_tag(u32 off, const iovec* iovecs, const u64* tags, u32 nr) noexcept {
  io_uring_rsrc_update2 up { 
    .offset = off,
    .resv = 0,
    .data = reinterpret_cast<__u64>(iovecs),
    .tags = reinterpret_cast<__u64>(tags),
    .nr = nr,
    .resv2 = 0 
  };

  return do_register(IORING_REGISTER_BUFFERS_UPDATE, &up, sizeof(up));
}

res_t io_uring::register_buffers_sparse(u32 nr) noexcept {
  io_uring_rsrc_register reg { 
    .nr = nr, 
    .flags = IORING_RSRC_REGISTER_SPARSE, 
    .resv2 = 0, 
    .data = 0, 
    .tags = 0 
  };

  return do_register(IORING_REGISTER_BUFFERS2, &reg, sizeof(reg));
}

res_t io_uring::register_napi(io_uring_napi* napi) noexcept {
  return do_register(IORING_REGISTER_NAPI, napi, 1);
}

res_t io_uring::unregister_napi(io_uring_napi* napi) noexcept {
  return do_register(IORING_UNREGISTER_NAPI, napi, 1);
}

res_t io_uring::register_personality() noexcept {
  return do_register(IORING_REGISTER_PERSONALITY, nullptr, 0);
}

res_t io_uring::unregister_personality(i32 id) noexcept {
  return do_register(IORING_UNREGISTER_PERSONALITY, nullptr, static_cast<u32>(id));
}

res_t io_uring::register_ifq(io_uring_zcrx_ifq_reg* reg) noexcept {
  return do_register(IORING_REGISTER_ZCRX_IFQ, reg, 1);
}

res_t io_uring::register_buf_status(u32 buf_group, u32* head_out) noexcept {
  if (!head_out) {
    return E::INVAL;
  }
  
  ::io_uring_buf_status st{};

  st.buf_group = buf_group;
  res_t const r = do_register(IORING_REGISTER_PBUF_STATUS, &st, 1);
  if (r < 0) {
    return r;
  }
  *head_out = st.head;

  return 0;
}

res_t io_uring::register_clone_buffers(const io_uring& source, u32 dst_off, u32 src_off, u32 nr, u32 flags) noexcept {
  ::io_uring_clone_buffers c{};

  c.src_off  = src_off;
  c.dst_off  = dst_off;
  c.nr       = nr;
  c.flags    = static_cast<__u32>(flags);

  if ((flags & IORING_REGISTER_SRC_REGISTERED) && (source._int_flags & INT_FLAG_REG_REG_RING)) {
    c.src_fd   = static_cast<__u32>(source._enterfd);
  } else {
    c.src_fd = static_cast<__u32>(source._ringfd);
    c.flags &= static_cast<__u32>(~static_cast<unsigned>(IORING_REGISTER_SRC_REGISTERED));
  }

  return do_register(IORING_REGISTER_CLONE_BUFFERS, &c, 1);
}