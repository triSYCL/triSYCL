#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_RPC_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_RPC_HPP

/** \file

    This file implement the RPC system used for device -> host communication.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

#ifdef __SYCL_XILINX_AIE__
#include "hardware.hpp"
#include "lock.hpp"
#ifndef __SYCL_DEVICE_ONLY__
#include "xaie_wrapper.hpp"
#endif
#endif

#include <cstdint>
#include <functional>
#include <map>
#include <typeindex>
#include <variant>
#include <string>
#include <iostream>

#include "triSYCL/detail/overloaded.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// This is a convenience wrapper that allows creating functor based RPC easily.
template<typename DataTy>
struct functor_rpc {
  using data_type = DataTy;
#ifndef __SYCL_DEVICE_ONLY__
  std::function<uint32_t(int, int, xaie::handle, data_type)> impl{};

  /// This is only ever one instance per data type.
  static functor_rpc& get() {
    static functor_rpc val;
    return val;
  }

  /// Process data that is received.
  /// This is executed each time new data arrives.
  static uint32_t act_on_data(int x, int y, xaie::handle h, data_type d) {
    return get().impl(x, y, h, d);
  }
#endif
};

/// This is the data that will be transmitted from the device to the host to
/// update the image. This struct needs to have the same layout on the host and
/// the device.
struct image_update_data {
  hw::stable_pointer<void> data;
  uint64_t min_value;
  uint64_t max_value;
  uint32_t counter;
};

using image_update_rpc = functor_rpc<image_update_data>;

/// This is the data that will be transmitted when to the host when the device is logging
/// This struct needs to have the same layout on the host and the device.
struct send_log_data {
  hw::stable_pointer<const char> data;
  uint64_t size;
};

using send_log_rpc = functor_rpc<send_log_data>;

struct done_rpc {
  struct data_type {};
#ifndef __SYCL_DEVICE_ONLY__
  static uint32_t act_on_data(int x, int y, xaie::handle h, data_type d) {
    return 0;
  }
#endif
};

#ifndef __SYCL_DEVICE_ONLY__
void initialize_log() {
  send_log_rpc::get().impl = [](int x, int y, xaie::handle h,
                                 send_log_data dev_data) -> uint32_t {
    hw::dev_ptr data_ptr = hw::get_dev_ptr({x, y}, dev_data.data);
    std::string str;
    str.resize(dev_data.size);
    h.moved(data_ptr.p).memcpy_d2h(str.data(), data_ptr.offset, str.size());
    std::cout << str << std::flush;
    return 0;
  };
}
#endif

template <typename... Tys> struct rpc_impl {
  using Var = std::variant<typename Tys::data_type...>;
#ifndef __SYCL_DEVICE_ONLY__
  struct host_side {
    int x_size, y_size;
    xaie::handle h;
    uint32_t addr;
    soft_barrier::host_side get_barrier(int x, int y) {
      return {h.moved(x, y), (uint32_t)(addr + offsetof(device_side, barrier))};
    }
    template<typename T>
    static void for_all_type(T&& Call) {
      (Call(Tys{}), ...);
    }
    uint32_t visit(int x, int y, xaie::handle h, Var v) {
      auto visitor = detail::overloaded{[&](typename Tys::data_type data) {
        return Tys::act_on_data(x, y, h, data);
      }...};
      return std::visit(visitor, v);
    }
    void wait_all() {
      ::trisycl::detail::no_log_in_this_scope nls;
      int done_counter = 0;
      do {
        for (int x = 0; x < x_size; x++)
          for (int y = 0; y < y_size; y++) {
            if (!get_barrier(x, y).try_arrive())
              continue;
            Var data;
            h.moved(x, y).memcpy_d2h(&data, addr + offsetof(device_side, data),
                                     sizeof(Var));
            /// This deals with the special case of 
            if (data.index() == 0) {
              done_counter++;
            } else {
              auto ret = visit(x, y, h.moved(x, y), data);
              h.moved(x, y).mem_write(addr + offsetof(device_side, ret_val),
                                      ret);
            }
            get_barrier(x, y).wait();
          }
      } while (done_counter < x_size * y_size);
    }
  };
#endif

  struct device_side {
#ifdef __SYCL_DEVICE_ONLY__
    static device_side *get() {
      return (device_side *)(acap::hw::self_tile_addr(
                                     acap::hw::get_parity_dev()) +
                                 acap::hw::rpc_record_begin_offset);
    }
#endif
    soft_barrier::device_side barrier;
    Var data;
    uint32_t ret_val;
    template <typename Ty> uint32_t perform(Ty &&d) {
      data = std::forward<Ty>(d);
      barrier.wait();
      /// Wait for the host to process the data.
      barrier.wait();
      return ret_val;
    }
  };
};

using rpc = rpc_impl<done_rpc, image_update_rpc, send_log_rpc>;

// The advantage of this over static_assert is that V1 and V2 will be printed when it fails.
template<auto V1, auto V2>
struct assert_equal {
  static_assert(V1 == V2, "");
};

/// This variable is just to check the rpc size
constexpr assert_equal<sizeof(rpc::device_side), hw::rpc_record_size> v;

} // namespace trisycl::vendor::xilinx::acap::aie

#endif
