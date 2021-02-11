#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_AIE_UTILS_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_AIE_UTILS_HPP

/** \file

    Model of an AI Engine program, that weaves the program of each tile
    with the memory of each tile for a given device

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifdef __SYCL_XILINX_AIE__
namespace trisycl::vendor::xilinx::acap::aie::xaie {
extern "C" {
  #include <xaiengine.h>
}
}
#include <iostream>
#include "triSYCL/detail/debug.hpp"

#include <boost/log/trivial.hpp>
#include <boost/type_index.hpp>

#if defined(TRISYCL_DEBUG) || defined(TRISYCL_XAIE_DEBUG)
#define TRISYCL_XAIE(XAIE_CALL)                                                \
  do {                                                                         \
    using namespace ::trisycl::vendor::xilinx::acap::aie::xaie;                \
    BOOST_LOG_TRIVIAL(debug) << #XAIE_CALL;                                    \
    boost::log::core::get()->flush();                                          \
    AieRC RC = XAIE_CALL;                                                      \
    if (RC != XAIE_OK) {                                                       \
      BOOST_LOG_TRIVIAL(debug)                                                 \
          << "XAIE call failed:" << RC << ":" << #XAIE_CALL;                   \
      boost::log::core::get()->flush();                                        \
      assert(false);                                                           \
    }                                                                          \
    BOOST_LOG_TRIVIAL(debug) << " done";                                       \
    boost::log::core::get()->flush();                                          \
  } while (0)
#else
#define TRISYCL_XAIE(XAIE_CALL)                                                \
  do {                                                                         \
    using namespace ::trisycl::vendor::xilinx::acap::aie::xaie;                \
    AieRC RC = XAIE_CALL;                                                      \
    if (RC != XAIE_OK) {                                                       \
      BOOST_LOG_TRIVIAL(debug)                                                 \
          << "XAIE call failed:" << RC << ":" << #XAIE_CALL;                   \
      boost::log::core::get()->flush();                                        \
      assert(false);                                                           \
    }                                                                          \
  } while (0)
#endif
namespace trisycl::vendor::xilinx::acap::aie::xaie {

struct aiev1 {

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
static constexpr auto args_size = 0x100;

};
}

#endif

#endif
