#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_DEVICE_ALLOCATOR_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_DEVICE_ALLOCATOR_HPP

/** \file

    contains hardware specific informations and linker scripts details of
    how the memory is used an partitioned
    TODO: One important optimization that could be done is adding a freelist.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <cstdint>

#include "hardware.hpp"
#include "log.hpp"

namespace trisycl::vendor::xilinx::acap {

/// This allocator is designed to minimize the memory overhead, to to be fast.
namespace heap {

constexpr unsigned min_alloc_size = 8;
/// Must be a power of 2;
constexpr unsigned alloc_align = 4;

/// metadata associated with each dynamic allocation.
/// 
struct block_header {
  /// This pointes to the previous block.
  hw::stable_pointer<block_header> prev;

  /// Size is used to find the next block.
  uint32_t size : 30;
  uint32_t in_use : 1;
  uint32_t is_last : 1;
#if defined(__SYCL_DEVICE_ONLY__)
  static block_header* get_header(void* ptr) {
    /// The block header is always just before the allocation in memory.
    return ((block_header*)ptr) - 1;
  }
  /// return a pointer to the section of memory this block header tracks.
  /// This region is just after the block_header.
  void* get_alloc() {
    return (void*)(this + 1);
  }
  void* get_end() {
    return (void*)((char*)(this + 1) + size);
  }
  block_header* get_next() {
    if (is_last)
      return nullptr;
    /// Blocks are one after the other in memory: header | alloc | header | alloc ...
    /// so the end of one block is the beginign of the next.
    return (block_header*)get_end();
  }
  block_header* get_prev() {
    return prev;
  }
  /// Check if the block is large enough to fit a block header plus some data.
  /// If not there is nothing to be gained by splitting the block.
  bool is_splitable(uint32_t new_size) {
    return size >= new_size + sizeof(block_header) + min_alloc_size;
  }
  void correct_next() {
    if (auto* next = get_next())
      next->prev = this;
  }
  /// resize the current block to new_size and create a block with the rest of the size.
  void split(uint32_t new_size) {
    assert(!in_use && "cannot change blocks that are in use");
    assert(size >= new_size + sizeof(block_header));
    assert((new_size % alloc_align) == 0 && "not properly aligned");

    /// Create new block
    block_header* new_next = (block_header*)((char*)get_alloc() + new_size);
    new_next->size = size - new_size - sizeof(block_header);
    new_next->in_use = 0;
    new_next->is_last = is_last;
    new_next->prev = this;
    new_next->correct_next();
    assert(((uint32_t)new_next % alloc_align) == 0 && "not properly aligned");
    assert((new_next->size % alloc_align) == 0 && "not properly aligned");

    /// Update old block
    this->size = new_size;
    this->is_last = 0;
    this->in_use = 0;
  }
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
  hw::stable_pointer<block_header> total_list;
#if defined(__SYCL_DEVICE_ONLY__)
  static allocator_global* get() {
    return (allocator_global *)(hw::self_tile_addr(hw::get_parity_dev()) +
                                hw::heap_begin_offset);
  }
  static block_header *create_block(void *p, uint32_t s) {
    block_header *block = (block_header *)p;
    block->prev = nullptr;
    block->size = s - sizeof(block_header);
    block->in_use = 0;
    block->is_last = 1;
    return block;
  }
#endif
};

#if defined(__SYCL_DEVICE_ONLY__)

void init_allocator() {
  allocator_global *ag = allocator_global::get();
  ag->total_list = allocator_global::create_block(
      (void *)(hw::self_tile_addr(hw::get_parity_dev()) +
               hw::heap_begin_offset + sizeof(allocator_global)),
      hw::heap_size - sizeof(allocator_global));
}

/// This malloc will return nullptr on failure.
void *try_malloc(uint32_t size) {
  /// extend size to the next multiple of alloc_align;
  size = (size + (alloc_align - 1)) & ~(alloc_align - 1);
  allocator_global *ag = allocator_global::get();
  block_header *bh = ag->total_list;
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
  /// There was no suitable block. so we cannot perform this allocation.
  /// Allocation faillure can be caused by high fragmentation and do not mean
  /// that no other allocation can be performed with this allocator.
  return nullptr;
}

/// This malloc will assert on allocation failure.
void* malloc(uint32_t size) {
  void* ret = try_malloc(size);
#ifdef TISYCL_DEVICE_ALLOCATOR_DEBUG
  log("malloc(");
  log(size, /*with_coord*/false);
  log(") = ", /*with_coord*/false);
  log(ret, /*with_coord*/false);
  log("\n", /*with_coord*/false);
#endif
  assert(ret != 0 && "unhandled dynamic allocation failure");
  return ret;
}

void free(void* p) {
#ifdef TISYCL_DEVICE_ALLOCATOR_DEBUG
  log("free(");
  log(p, /*with_coord*/false);
  log(")\n", /*with_coord*/false);
#endif
  block_header *bh = block_header::get_header(p);
  assert(bh->in_use && "double free or free on invalid address");
  bh->in_use = 0;
  /// try to merge with nearby blocks to reduce fragmentation.
  bh->try_merge_next();
  /// We try to merge into the previous block after mergin into the next block
  /// because mergin into the previous block makes the current block unusable if
  /// successful.
  bh->try_merge_prev();
}

/// This function will log the state of the heap.
void dump_allocator_state() {
  allocator_global *ag = allocator_global::get();
  log("dumping blocks in heap ");
  log(hw::heap_begin_offset, /*with_coord*/false);
  log("-", /*with_coord*/false);
  log(hw::heap_end_offset, /*with_coord*/false);
  log("\n", /*with_coord*/false);
  int idx = 0;
  block_header *bh = ag->total_list;
  while (bh) {
    log("block ");
    log(idx++, /*with_coord*/false);
    log(" self=", /*with_coord*/false);
    log(bh, /*with_coord*/false);
    log(" alloc=", /*with_coord*/false);
    log(bh->get_alloc(), /*with_coord*/false);
    log(" in_use=", /*with_coord*/false);
    log(bh->in_use, /*with_coord*/false);
    log(" size=", /*with_coord*/false);
    log(bh->size, /*with_coord*/false);
    log(" next=", /*with_coord*/false);
    log(bh->get_next(), /*with_coord*/false);
    log(" prev=", /*with_coord*/false);
    log(bh->get_prev(), /*with_coord*/false);
    log("\n", /*with_coord*/false);
    bh = bh->get_next();
  }
}

void assert_no_leak() {
  allocator_global *ag = allocator_global::get();
  block_header *bh = ag->total_list;
  bool has_leak = false;
  while (bh) {
    if (bh->in_use) {
      has_leak = true;
      log("block ");
      log(" addr=", /*with_coord*/ false);
      log(bh->get_alloc(), /*with_coord*/ false);
      log(" size=", /*with_coord*/ false);
      log(bh->size, /*with_coord*/ false);
      log(" still in use\n", /*with_coord*/ false);
    }
    bh = bh->get_next();
  }
  assert(!has_leak && "leak detected");
}

#endif

}

}

#endif
