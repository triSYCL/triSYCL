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

#define HW_GEN XAIE_DEV_GEN_AIE
#define XAIE_NUM_ROWS            9
#define XAIE_NUM_COLS            50
#define XAIE_ADDR_ARRAY_OFF      0x800

#define XAIE_BASE_ADDR 0x20000000000
#define XAIE_COL_SHIFT 23
#define XAIE_ROW_SHIFT 18
#define XAIE_SHIM_ROW 0
#define XAIE_MEM_TILE_ROW_START 0
#define XAIE_MEM_TILE_NUM_ROWS 0
#define XAIE_AIE_TILE_ROW_START 1
#define XAIE_AIE_TILE_NUM_ROWS 8

#endif

#endif
