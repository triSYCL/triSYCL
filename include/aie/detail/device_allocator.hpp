#ifndef AIE_DETAIL_DEVICE_ALLOCATOR_HPP
#define AIE_DETAIL_DEVICE_ALLOCATOR_HPP

/// This file contains an implementation of a heap allocator similar to malloc
/// but this heap allocator is intended to run on device. This file also
/// implement so utility to setup and allocate blocks of memory on device from
/// the host

#ifndef __SYCL_XILINX_AIE__
# error "This file should only be used for hardware execution"
#endif

#include "hardware.hpp"
#include "sync.hpp"
#include "utils.hpp"
#include "xaie_wrapper.hpp"
#include <cstddef>
#include <cstdint>

namespace aie::detail {

/// This allocator is designed to minimize the memory overhead to be fast.
namespace heap {

/// metadata associated with each dynamic allocation.
/// 
struct block_header {
  /// Point to the previous block
  dev_ptr<block_header> prev;

  /// Size is used to find the next block.
  uint32_t size : 29;
  /// Whether the allocation is currently in use.
  uint32_t in_use : 1;
  /// Whether this allocation is the last allocation of the list
  uint32_t is_last : 1;

  uint32_t is_host_allocated : 1;

  /// Check if the block is large enough to fit a block header plus some data.
  /// If not there is nothing to be gained by splitting the block.
  bool is_splitable(uint32_t new_size) {
    return size >= new_size + sizeof(block_header) + min_alloc_size;
  }

#if !defined(__SYCL_DEVICE_ONLY__)
  /// Resize the current block to new_size and create a block with the rest of the size.
  void split(xaie::handle handle, uint32_t bh_addr, uint32_t new_size) {
    assert(size >= new_size + sizeof(block_header));
    assert((new_size % alloc_align) == 0 && "not properly aligned");

    /// Create new block
    uint32_t new_next_addr = bh_addr + sizeof(block_header) + new_size;
    block_header new_next;
    new_next.size = size - new_size - sizeof(block_header);
    new_next.in_use = 0;
    new_next.is_last = is_last;
    new_next.prev.set_offset(bh_addr);
    new_next.is_host_allocated = 1;
    assert((new_next_addr % alloc_align) == 0 && "not properly aligned");
    assert((new_next.size % alloc_align) == 0 && "not properly aligned");
    handle.store(new_next_addr, new_next);

    if (!new_next.is_last) {
      uint32_t old_next_addr = bh_addr + sizeof(block_header) + this->size;
      block_header old_next = handle.load<block_header>(old_next_addr);
      old_next.prev.set_offset(new_next_addr);
      handle.store(old_next_addr, old_next);
    }

    /// Update old block
    this->size = new_size;
    this->is_last = 0;
  }
#else

  /// Get the \c block_header from an allocation. The inverse of \c get_alloc.
  static block_header* get_header(void* ptr) {
    /// The block header is always just before the allocation in memory.
    return static_cast<block_header*>(ptr) - 1;
  }

  /// Return a pointer to the section of memory this block header tracks.
  /// This region is just after the \c block_header. The inverse of \c get_header.
  void* get_alloc() {
    return static_cast<void*>(this + 1);
  }

  /// Return the end of the allocation.
  void* get_end() {
    return static_cast<void*>(reinterpret_cast<char*>(this + 1) + size);
  }

  /// Get the next block in the list
  block_header* get_next() {
    if (is_last)
      return nullptr;
    /// Blocks are one after the other in memory: header | alloc | header | alloc ...
    /// so the end of one block is the beginign of the next.
    return reinterpret_cast<block_header*>(get_end());
  }

  /// Get the previous block in the list
  block_header* get_prev() {
    return prev.get();
  }

  /// Correct the previous field of the next block if it does exist.
  /// This is needed when inserting or removing blocks from the list.
  void correct_next() {
    if (auto* next = get_next())
      next->prev = this;
  }

  /// Resize the current block to new_size and create a block with the rest of the size.
  void split(uint32_t new_size) {
    assert(size >= new_size + sizeof(block_header));
    assert((new_size % alloc_align) == 0 && "not properly aligned");

    /// Create new block
    block_header* new_next = reinterpret_cast<block_header*>(static_cast<char*>(get_alloc()) + new_size);
    new_next->size = size - new_size - sizeof(block_header);
    new_next->in_use = 0;
    new_next->is_last = is_last;
    new_next->prev = this;
    new_next->correct_next();
    assert((reinterpret_cast<int32_t>(new_next) % alloc_align) == 0 && "not properly aligned");
    assert((new_next->size % alloc_align) == 0 && "not properly aligned");

    /// Update old block
    this->size = new_size;
    this->is_last = 0;
  }

  /// This will try to merge this block with the following block.
  /// Every reference to the next block_header maybe invalid after this call.
  void try_merge_next() {
    block_header* next = get_next();
    /// Cannot be merged with next block
    if (!next || next->in_use)
      return;
    this->size = this->size + sizeof(block_header) + next->size;
    this->is_last = next->is_last;
    this->correct_next();
  }

  /// This will try to merge this block with the previous block.
  /// The block_header may not be in usable state after this call because it
  /// could be skipped.
  void try_merge_prev() {
    auto* prev = get_prev();
    if (prev && !prev->in_use)
      prev->try_merge_next();
  }
#endif
};

/// This is to make sure we are made aware when the block_header size changes.
/// It is safe to change it.
static_assert(sizeof(block_header) == 8, "");

struct allocator_global {
#if defined(__SYCL_DEVICE_ONLY__)
  static block_header* get_start() {
    return get_object<block_header>(
        offset_table::get_heap_begin_offset());
  }
#endif
};

#if defined(__SYCL_DEVICE_ONLY__)

/// This malloc will return nullptr on failure.
void* try_malloc(uint32_t size) {
  /// extend size to the next multiple of alloc_align;
  size = align_up(size, alloc_align);
  block_header* bh = allocator_global::get_start();
  /// Go throught the whole block list.
  while (bh) {
    /// Find a suitable block.
    if (bh->size >= size && !bh->in_use) {
      /// Split the block if possible
      if (bh->is_splitable(size))
        bh->split(size);
      /// Mark the block used and return the address
      bh->in_use = 1;
      return bh->get_alloc();
    }
    bh = bh->get_next();
  }
  /// There was no suitable block, so we cannot perform this allocation.
  /// Allocation faillure can be caused by high fragmentation and do not mean
  /// that no other allocation can be performed with this allocator.
  return nullptr;
}

/// This malloc will assert on allocation failure.
void* malloc(uint32_t size) {
  void* ret = try_malloc(size);
#ifdef TRISYCL_DEVICE_ALLOCATOR_DEBUG
  log("malloc(", size, ") = ", ret, "\n");
#endif
  assert(ret && "unhandled dynamic allocation failure");
  return ret;
}

#else
/// Initialize the allocator
void init_allocator(xaie::handle handle, uint32_t heap_start,
                    uint32_t heap_size) {
  assert(sizeof(allocator_global) + min_alloc_size < heap_size &&
         "the allocator was not provided enough space to work properly");
  assert(heap_start % alloc_align == 0);
  block_header block;
  block.prev = nullptr;
  block.size = heap_size - sizeof(block_header);
  block.in_use = 0;
  block.is_last = 1;
  block.is_host_allocated = 1;
  handle.store(heap_start, block);
}

/// This malloc will return nullptr on failure.
uint32_t try_malloc(xaie::handle handle, uint32_t heap_start, uint32_t size) {
  /// extend size to the next multiple of alloc_align;
  size = align_up(size, alloc_align);
  uint32_t bh_addr = heap_start;
  block_header bh;
  /// Go throught the whole block list.
  do {
    /// load the block
    bh = handle.load<block_header>(bh_addr);
    /// Find a suitable block.
    if (bh.size >= size && !bh.in_use) {
      /// Split the block if possible
      if (bh.is_splitable(size))
        bh.split(handle, bh_addr, size);
      /// Mark the block used and return the address
      bh.in_use = 1;
      bh.is_host_allocated = 1;
      handle.store(bh_addr, bh);
      return bh_addr + sizeof(block_header);
    }
    /// Calculate the address of the next block.
    bh_addr = bh_addr + sizeof(block_header) + bh.size;
  } while (!bh.is_last);

  /// There was no suitable block, so we cannot perform this allocation.
  /// Allocation faillure can be caused by high fragmentation and do not mean
  /// that no other allocation can be performed with this allocator.
  return 0;
}

/// This malloc will assert on allocation failure.
uint32_t malloc(xaie::handle handle, uint32_t heap_start, uint32_t size) {
  uint32_t ret = try_malloc(handle, heap_start, size);
#ifdef TRISYCL_DEVICE_ALLOCATOR_DEBUG
  log("malloc(", size, ") = ", ret, "\n");
#endif
  assert(ret && "unhandled dynamic allocation failure");
  return ret;
}

// void dump_allocator_state(xaie::handle handle, uint32_t heap_start) {
//   log(
//       "dumping blocks in heap ",
//       heap_start, "-",
//       "?????", "\n");
//   int idx = 0;
//   uint32_t bh_addr = heap_start;
//   block_header bh;
//   do {
//     bh = handle.load<block_header>(bh_addr);
//     log("block ", idx++, " self=", bh_addr,
//               " alloc=", bh_addr + sizeof(block_header), " in_use=", bh.in_use,
//               " size=", bh.size,
//               " next=", bh_addr + sizeof(block_header) + bh.size,
//               " prev=", bh.prev.get_offset(),
//               " is_host_allocated=", bh.is_host_allocated, "\n");
//     /// Calculate the address of the next block.
//     bh_addr = bh_addr + sizeof(block_header) + bh.size;
//     /// load the next block
//   } while (!bh.is_last);
// }

#endif

#if defined(__SYCL_DEVICE_ONLY__)
void* try_realloc(void* ptr, uint32_t new_size) {
  /// Extend size to the next multiple of alloc_align;
  new_size = align_up(new_size, alloc_align);
  block_header* bh = block_header::get_header(ptr);
  /// Since we automatically merge blocks on free, there can only be one
  /// consecutive free block, so we only need to look at the next block. If we
  /// can use the next block
  if (bh->get_next() && !bh->get_next()->in_use)
    /// If merging with the next block would allow us to reach the requested size.
    if (bh->size + bh->get_next()->size + sizeof(block_header) >= new_size)
      bh->try_merge_next();
  /// If we can use the current (maybe after merging) block to reach the required size.
  if (bh->size > new_size) {
    if (bh->is_splitable(new_size))
      bh->split(new_size);
    return ptr;
  }

  /// Otherwise fallback to allocating a new block
  void* new_ptr = try_malloc(new_size);
  /// If we failed to allocate a new block propagate the error.
  if (!new_ptr)
    return nullptr;
  std::memcpy(new_ptr, ptr, bh->size);
  free(ptr);
  return new_ptr;
}

void* realloc(void* ptr, uint32_t new_size) {
  void* ret = try_realloc(ptr, new_size);
#ifdef TRISYCL_DEVICE_ALLOCATOR_DEBUG
  log("realloc(", ptr ,", ", new_size, ") = ", ret, "\n");
#endif
  assert(ret && "unhandled dynamic allocation failure");
  return ret;
}

/// Release an allocation and try to merge it with nearby allocations
void free(void* p) {
#ifdef TRISYCL_DEVICE_ALLOCATOR_DEBUG
  log("free(", p, ")\n");
#endif
  block_header* bh = block_header::get_header(p);
  assert(bh->in_use && "double free or free on invalid address");
  bh->in_use = 0;
  /// Try to merge with nearby blocks to reduce fragmentation.
  bh->try_merge_next();
  /// We try to merge into the previous block after merging into the next block
  /// because merging into the previous block makes the current block unusable if
  /// successful.
  bh->try_merge_prev();
}

// /// This function will log the state of the heap.
// void dump_allocator_state() {
//   log(
//       "dumping blocks in heap ",
//       get_object<void>(offset_table::get_heap_begin_offset()), "-",
//       get_object<void>(offset_table::get_heap_end_offset()), "\n");
//   int idx = 0;
//   block_header* bh = allocator_global::get_start();
//   while (bh) {
//     log("block ", idx++, " self=", bh, " alloc=", bh->get_alloc(),
//               " in_use=", bh->in_use, " size=", bh->size,
//               " next=", bh->get_next(), " prev=", bh->get_prev(),
//               " is_host_allocated=", bh->is_host_allocated, "\n");
//     bh = bh->get_next();
//   }
// }

void assert_no_leak() {
  block_header* bh = allocator_global::get_start();
  bool has_leak = false;
  while (bh) {
    if (bh->in_use && !bh->is_host_allocated) {
      has_leak = true;
      // log("block ", " addr=", bh->get_alloc(), " size=", bh->size,
      //           " still in use\n");
    }
    bh = bh->get_next();
  }
  assert(!has_leak && "leak detected");
}

#endif

}

}

#endif
