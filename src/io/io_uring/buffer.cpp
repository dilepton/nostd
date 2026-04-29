#include "io_uring.hpp"
#include "error.hpp"
#include "memory/defs.hpp"
#include "vma.hpp"

using namespace nostd;

::io_uring_buf_ring* io_uring::setup_buf_ring(u32 nentries, u16 bgid, u16 flags, error_t* err) noexcept {
  if (!err) return nullptr;
  *err = 0;

  if (!nentries || (nentries & (nentries - 1))) {
    *err = E::INVAL;
    return nullptr;
  }

  const usize ring_sz = nentries * sizeof(::io_uring_buf);
  const usize map_sz  = VMA::page_round(ring_sz);

  ::io_uring_buf_reg reg{};
  reg.ring_entries = nentries;
  reg.bgid         = bgid;
  reg.flags        = flags;

  ::io_uring_buf_ring* br = nullptr;

  if (!(flags & IOU_PBUF_RING_MMAP)) {
    byte* p = VMA::mmap(nullptr, map_sz, Protection::RW,MapFlags::PRIVATE | MapFlags::ANONYMOUS, -1, 0);
    if (is_error(p)) { 
      *err = ptr_err(p);
      return nullptr;
    }

    br = reinterpret_cast<::io_uring_buf_ring*>(p);
    reg.ring_addr = reinterpret_cast<u64>(p);
  }

  if (const res_t ret = register_buf_ring(&reg, 0); ret) {
    if (br) {
      VMA::munmap(reinterpret_cast<byte*>(br), map_sz);
    }

    *err = ret;
    return nullptr;
  }

  if (flags & IOU_PBUF_RING_MMAP) {
    const usize off = IORING_OFF_PBUF_RING | (static_cast<u64>(bgid) << IORING_OFF_PBUF_SHIFT);
    byte* p = VMA::mmap(nullptr, map_sz, Protection::RW, MapFlags::SHARED | MapFlags::POPULATE, _ringfd, off);

    if (is_error(p)) {
      unregister_buf_ring(bgid);
      *err = ptr_err(p);
      return nullptr;
    }
    
    br = reinterpret_cast<::io_uring_buf_ring*>(p);
  }

  br->tail = 0;
  return br;
}

res_t io_uring::free_buf_ring(::io_uring_buf_ring* br, u32 nentries, u16 bgid)  noexcept {
  if (!br) return E::INVAL;

  if (const res_t ret = unregister_buf_ring(bgid); ret) {
    return ret;
  }

  const usize ring_sz = static_cast<usize>(nentries) * sizeof(::io_uring_buf);
  const usize map_sz  = VMA::page_round(ring_sz);

  return VMA::munmap(reinterpret_cast<byte*>(br), map_sz);
}

void io_uring::buf_ring_cq_advance(::io_uring_buf_ring* br, u32 count) noexcept {
  if (!br || !count) return;

  buf_ring_advance(br, static_cast<i32>(count));
  cq_advance(count);
}

res_t io_uring::buf_ring_available(::io_uring_buf_ring* br, u16 bgid) noexcept {
  if (!br) return E::INVAL;

  u32 head = 0;
  if (const res_t ret = register_buf_status(bgid, &head); ret) {
    return ret;
  }

  const u16 tail = br->tail;
  
  return static_cast<u16>(tail - static_cast<u16>(head));
}
