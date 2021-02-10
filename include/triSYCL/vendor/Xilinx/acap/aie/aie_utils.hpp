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
extern "C" {
  #include <xaiengine.h>
}
#include <iostream>
#include "triSYCL/detail/debug.hpp"

#include <boost/log/trivial.hpp>
#include <boost/type_index.hpp>

using namespace std::string_literals;

#if defined(TRISYCL_DEBUG) || defined(TRISYCL_XAIE_DEBUG)
#define TRISYCL_XAIE(XAIE_CALL)                                                \
  do {                                                                         \
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
    AieRC RC = XAIE_CALL;                                                      \
    if (RC != XAIE_OK) {                                                       \
      BOOST_LOG_TRIVIAL(debug)                                                 \
          << "XAIE call failed:" << RC << ":" << #XAIE_CALL;                   \
      boost::log::core::get()->flush();                                        \
      assert(false);                                                           \
    }                                                                          \
  } while (0)
#endif

#endif

#endif
