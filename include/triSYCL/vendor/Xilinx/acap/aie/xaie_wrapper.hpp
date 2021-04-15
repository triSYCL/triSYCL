#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_AIE_UTILS_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_AIE_UTILS_HPP

/** \file

    Model of an AI Engine program, that weaves the program of each tile
    with the memory of each tile for a given device

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <iostream>
#include "triSYCL/detail/debug.hpp"

#include <boost/log/trivial.hpp>
#include <boost/type_index.hpp>

#ifdef __SYCL_XILINX_AIE__
namespace trisycl::vendor::xilinx::acap::aie::xaie {
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
} // namespace trisycl::vendor::xilinx::acap::aie::xaie

#if defined(TRISYCL_DEBUG) || defined(TRISYCL_XAIE_DEBUG)
#define TRISYCL_XAIE(XAIE_CALL)                                                \
  do {                                                                         \
    using namespace ::trisycl::vendor::xilinx::acap::aie::xaie;                \
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
    using namespace ::trisycl::vendor::xilinx::acap::aie::xaie;                \
    AieRC RC = XAIE_CALL;                                                      \
    if (RC != XAIE_OK) {                                                       \
      TRISYCL_DUMP_ALWAYS("XAIE call failed:" << RC << ":" << #XAIE_CALL);     \
      boost::log::core::get()->flush();                                        \
      assert(false);                                                           \
    }                                                                          \
  } while (0)
#endif

namespace trisycl::vendor::xilinx::acap::aie::xaie {

namespace aiev1 {

static constexpr auto dev_gen = XAIE_DEV_GEN_AIE;
static constexpr auto num_hw_row = 9;
static constexpr auto num_hw_col = 50;
static constexpr auto addr_array_off = 0x800;
static constexpr auto base_addr = 0x20000000000;
static constexpr auto col_shift = 23;
static constexpr auto row_shift = 18;
static constexpr auto num_shim_row = 0;
static constexpr auto mem_tile_row_start = 0;
static constexpr auto mem_tile_row_num = 0;
static constexpr auto aie_tile_row_start = 1;
static constexpr auto aie_tile_row_num = 8;
static constexpr auto args_start = 0x1000;
static constexpr auto args_size = 0x2000;

};
}

#endif

#endif
