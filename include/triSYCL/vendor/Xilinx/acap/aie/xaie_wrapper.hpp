#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_AIE_UTILS_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_AIE_UTILS_HPP

#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)
// for host side on device execution

/** \file

    Model of an AI Engine program, that weaves the program of each tile
    with the memory of each tile for a given device

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/detail/debug.hpp"
#include "triSYCL/detail/program_manager.hpp"

#include <iostream>
#include <sstream>

#include <boost/log/trivial.hpp>
#include <boost/type_index.hpp>

#include "hardware.hpp"

namespace trisycl::vendor::xilinx::xaie {
extern "C" {
#include <xaiengine.h>
}

/// RAII move-only object for libxaiengine transactions.
class XAie_Transaction {
  XAie_DevInst *DevInst = nullptr;

public:
  XAie_Transaction() = default;
  XAie_Transaction(XAie_DevInst *dev,
                   uint32_t flags = XAIE_TRANSACTION_ENABLE_AUTO_FLUSH)
      : DevInst(dev) {
    XAie_StartTransaction(DevInst, flags);
  }
  XAie_Transaction(const XAie_Transaction &) = delete;
  XAie_Transaction(XAie_Transaction &&) = default;
  ~XAie_Transaction() {
    if (DevInst)
      XAie_SubmitTransaction(DevInst, nullptr);
  }
};

} // namespace trisycl::vendor::xilinx::xaie

#if defined(TRISYCL_DEBUG) || defined(TRISYCL_XAIE_DEBUG)
#define TRISYCL_XAIE(XAIE_CALL)                                                \
  do {                                                                         \
    using namespace ::trisycl::vendor::xilinx::xaie;                           \
    TRISYCL_DUMP2(#XAIE_CALL, "xaie");                                         \
    boost::log::core::get()->flush();                                          \
    AieRC RC = XAIE_CALL;                                                      \
    if (RC != XAIE_OK) {                                                       \
      TRISYCL_DUMP_ALWAYS("XAIE call failed:" << RC << ":" << #XAIE_CALL);     \
      boost::log::core::get()->flush();                                        \
      assert(false);                                                           \
    }                                                                          \
    TRISYCL_DUMP2("", "done");                                                 \
    boost::log::core::get()->flush();                                          \
  } while (0)
#else
#define TRISYCL_XAIE(XAIE_CALL)                                                \
  do {                                                                         \
    using namespace ::trisycl::vendor::xilinx::xaie;                           \
    AieRC RC = XAIE_CALL;                                                      \
    if (RC != XAIE_OK) {                                                       \
      TRISYCL_DUMP_ALWAYS("XAIE call failed:" << RC << ":" << #XAIE_CALL);     \
      boost::log::core::get()->flush();                                        \
      assert(false);                                                           \
    }                                                                          \
  } while (0)
#endif

namespace trisycl::vendor::xilinx::acap {

/// This namespace contains the configuration to use for libxaiengine when using
/// v1 hardware.
namespace aiev1 {

constexpr auto dev_gen = XAIE_DEV_GEN_AIE;
constexpr auto num_hw_row = 9;
constexpr auto num_hw_col = 50;
constexpr auto addr_array_off = 0x800;
constexpr auto base_addr = 0x20000000000;
constexpr auto col_shift = 23;
constexpr auto row_shift = 18;
constexpr auto num_shim_row = 0;
constexpr auto mem_tile_row_start = 0;
constexpr auto mem_tile_row_num = 0;
constexpr auto aie_tile_row_start = 1;
constexpr auto aie_tile_row_num = 8;

}; // namespace aiev1
} // namespace trisycl::vendor::xilinx::acap

namespace trisycl::vendor::xilinx::xaie {

constexpr xaie::XAie_LocType acap_pos_to_xaie_pos(acap::pos p) {
  return xaie::XAie_TileLoc(p.x, p.y + 1);
}

constexpr acap::pos xaie_pos_to_acap_pos(xaie::XAie_LocType loc) {
  return { loc.Col, loc.Row - 1 };
}

struct handle {
  xaie::XAie_LocType tile;
  xaie::XAie_DevInst *inst;

  std::string get_pos_str() {
    std::string str;
    std::stringstream ss(str);
    acap::pos pos = xaie_pos_to_acap_pos(tile);
    ss << pos.x << ", " << pos.y;
    return ss.str();
  }

  xaie::XAie_Transaction
  get_transaction(uint32_t flags = XAIE_TRANSACTION_ENABLE_AUTO_FLUSH) {
    return {inst, flags};
  }

  /// The memory read accessors
  std::uint32_t mem_read(std::uint32_t offset) {
    std::uint32_t Data;
    TRISYCL_XAIE(
        xaie::XAie_DataMemRdWord(inst, tile, offset, &Data));
    TRISYCL_DUMP2("Reading: (" << get_pos_str() << ") + 0x" << std::hex
                               << offset << " = " << std::dec << Data,
                  "memory");
    return Data;
  }

  /// The memory write accessors
  void mem_write(std::uint32_t offset, std::uint32_t data) {
    TRISYCL_XAIE(
        xaie::XAie_DataMemWrWord(inst, tile, offset, data));
    TRISYCL_DUMP2("Writing: (" << get_pos_str() << ") + 0x" << std::hex
                               << offset << " = " << std::dec << data,
                  "memory");
  }

  /// memcpy from device to host
  void memcpy_d2h(void *data, std::uint32_t offset, std::uint32_t size) {
    TRISYCL_XAIE(xaie::XAie_DataMemBlockRead(inst, tile,
                                             offset, data, size));
    TRISYCL_DUMP2("memcpy_d2h: (" << get_pos_str() << ") + 0x" << std::hex
                                  << offset << "-0x" << offset + size
                                  << " -> 0x" << (uintptr_t)data << "-0x"
                                  << ((uintptr_t)data) + size,
                  "memory");
  }

  /// memcpy from host to device
  void memcpy_h2d(std::uint32_t offset, void *data, std::uint32_t size) {
    TRISYCL_XAIE(xaie::XAie_DataMemBlockWrite(inst, tile,
                                              offset, data, size));
    TRISYCL_DUMP2("memcpy_h2d: (" << get_pos_str() << ") + 0x" << std::hex
                                  << offset << "-0x" << offset + size
                                  << " <- 0x" << (uintptr_t)data << "-0x"
                                  << ((uintptr_t)data) + size,
                  "memory");
  }

  /// Configure device for dma.
  void mem_dma(uint32_t offset, uint32_t size) {
    xaie::XAie_DmaDesc DmaDesc;
    TRISYCL_XAIE(
        xaie::XAie_DmaDescInit(inst, &DmaDesc, tile));
    TRISYCL_XAIE(xaie::XAie_DmaSetAddrLen(&DmaDesc, offset, size));
    TRISYCL_DUMP2("Setup Dma: (" << get_pos_str() << ") 0x" << std::hex
                                 << offset << "-0x" << (offset + size),
                  "memory");
  }

  /// FIXME: is this right location for these functions?
  /// Load the elf via path to a elf binary file, handy for debugging if you
  /// wish to dump a stored binary or load something AoT compiled seperately
  void load_elf_file(const char *path) {
    // global load of elf
    TRISYCL_XAIE(xaie::XAie_LoadElf(inst, tile, path, 0));
  }

  /// Load the elf via path to a block of memory which contains an elf image
  void load_elf_image(std::string_view image) {
    assert(::trisycl::detail::program_manager::isELFMagic(image.data()) &&
           "invalid ELF magic");
    TRISYCL_XAIE(xaie::XAie_LoadElfMem(inst, tile,
                                       (u8 *)(image.data())));
  }

  /// Reset the core
  void core_reset() {
    TRISYCL_XAIE(xaie::XAie_CoreUnreset(inst, tile));
  }

  /// Start the core
  void core_run() {
    TRISYCL_XAIE(xaie::XAie_CoreEnable(inst, tile));
  }

  /// Stop the core
  void core_stop() {
    TRISYCL_XAIE(xaie::XAie_CoreDisable(inst, tile));
  }

  /// Wait for the core to complete
  void core_wait() {
    TRISYCL_DUMP2(
        std::dec << "(" << get_pos_str() << ") Waiting for kernel...", "exec");
    xaie::AieRC RC = xaie::XAIE_OK;
    do {
      RC = xaie::XAie_CoreWaitForDone(inst, tile, 0);
    } while (RC != xaie::XAIE_OK);
    TRISYCL_DUMP2(std::dec << "(" << get_pos_str() << ") done", "exec");
  }
};

} // namespace trisycl::vendor::xilinx::xaie

#endif

#endif