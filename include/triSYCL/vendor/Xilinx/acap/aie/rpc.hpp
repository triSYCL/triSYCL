#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_RPC_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_RPC_HPP

/** \file

    Implement the RPC system used for device → host communication.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

#include "hardware.hpp"

#ifdef __SYCL_XILINX_AIE__
#include "lock.hpp"
#include "xaie_wrapper.hpp"
#endif

#include <cstdint>
#include <functional>
#include <variant>

#include "triSYCL/detail/layout_utils.hpp"
#include "triSYCL/detail/overloaded.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// This is a convenience wrapper that allows creating functor based RPC easily.
template<typename DataTy>
struct functor_rpc {
  using data_type = DataTy;

#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  std::function<uint32_t(int, int, xaie::handle, data_type)> impl{};

  /// This is only one instance of this per template parameter, this function
  /// will get that instance.
  static functor_rpc &get() {
    static functor_rpc val;
    return val;
  }

  template<typename Func>
  static void set_handler(Func&& func) {
    get().impl = std::forward<Func>(func);
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
  /// A pointer to the new image data
  hw::dev_ptr<void> data;

  /// In min_value and max_value, uint64_t is just to have 8-byte of storage,
  /// the graphics system will bitcast min_value to the proper type before use.

  /// 8-byte storage for the minimun value of a pixel.
  uint64_t min_value;
  /// 8-byte storage for The maximum value of a pixel.
  uint64_t max_value;

  /// Number of image updates that as been sent
  uint32_t counter;
};

/// The act_on_data has dependencies on graphics.hpp but we do not want to
/// include graphics.hpp here because:
///  - rpc.hpp is already included in graphics.hpp and we cannot have circular
///    dependencies. this might be removable with sufficient refactoring.
///  - rpc.hpp is a lower level system then graphics.hpp so
///    graphics.hpp should include rpc.hpp not the inverse.
/// so we use functor_rpc to have a type-erased RPC
/// class. and the functor is setup in graphics.hpp.
using image_update_rpc = functor_rpc<image_update_data>;

struct send_log_rpc {
  /// This is the data that will be transmitted to the host when the device
  /// is logging. This struct needs to have the same layout on the host and the
  /// device.
  struct data_type {
    /// Pointer to the first character of a buffer to print
    hw::dev_ptr<const char> data;
    /// Number of characters to print
    uint64_t size;
  };
#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  static uint32_t act_on_data(int x, int y, xaie::handle h,
                              data_type dev_data) {
    std::string str;
    str.resize(dev_data.size);
    /// Copy the indicated device data into a string.
    h.moved(hw::position{x, y}.moved(dev_data.data.get_dir()))
        .memcpy_d2h(str.data(), dev_data.data.get_offset(), str.size());
    std::cout << str << std::flush;
    return 0;
  }
#endif
};

struct host_breakpoint {
  struct data_type {
    uint32_t offset;
    uint32_t count;
  };
#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  __attribute__((noinline)) static void host_break(int x, int y, xaie::handle h, data_type d) {
    /// Put a breakpoint in this function to debug
  }
  static uint32_t act_on_data(int x, int y, xaie::handle h, data_type d) {
    host_break(x, y, h, d);
    return 0;
  }
#endif
};

/// done_rpc is handled differently because indicate if a kernel is done
/// executing.
struct done_rpc {
  /// it needs no data, since it is just a signal.
  struct data_type {};
#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  static uint32_t act_on_data(int x, int y, xaie::handle h, data_type d) {
    /// The effect of the signal are handled directly by wait_all
    assert(false && "This should be handled by wait_all");
    return 0;
  }
#endif
};

template <typename... Tys> struct rpc_impl {
  using Var = std::variant<typename Tys::data_type...>;
#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  struct host_side {
    int x_size, y_size;
    xaie::handle h;

    /// this will retrun a handle to the synchronization barrier between the device and the host.
    soft_barrier::host_side get_barrier(int x, int y) {
      return {h.moved(x, y), (uint32_t)(hw::offset_table::get_rpc_record_begin_offset() + offsetof(device_side, barrier))};
    }

    /// This will invoke the correct function to process the data in v
    uint32_t visit(int x, int y, xaie::handle h, Var v) {
      auto visitor = detail::overloaded{[&](typename Tys::data_type data) {
        return Tys::act_on_data(x, y, h, data);
      }...};
      return std::visit(visitor, v);
    }

    /// This will wait on every kernel while handling there RPC requests
    void wait_all() {
      ::trisycl::detail::no_log_in_this_scope nls;
      int addr = hw::offset_table::get_rpc_record_begin_offset();
      /// This count the number of kernel that indicated they finished
      /// executing. any kernel can signal it finished executing just once
      /// because it stop executing or get stuck in an infinite loop after that.
      /// so it is not needed to keep track of which kernel stoped executing
      /// just how many.
      int done_counter = 0;
      do {
        for (int x = 0; x < x_size; x++)
          for (int y = 0; y < y_size; y++) {
            /// We process at least one request per device.
            bool chain = true;
            /// while the device asks to chain requests.
            while (chain) {
              /// If try_arrive returns true the device has written data and is
              /// wating on the host to act on it
              if (!get_barrier(x, y).try_arrive())
                continue;
              Var data;
              /// Read the data the device has written.
              h.moved(x, y).memcpy_d2h(
                  &data, addr + offsetof(device_side, data), sizeof(Var));
              /// Deal with the special case of a kernel indicating it is done.
              /// This kernel stopped executing.
              if (data.index() == 0) {
                done_counter++;
              } else {
                /// Otherwise call the appropriate function.
                auto ret = visit(x, y, h.moved(x, y), data);
                /// And write back the response.
                h.moved(x, y).mem_write(addr + offsetof(device_side, ret_val),
                                        ret);
                /// read if the device requested to chain the is request.
                chain = h.moved(x, y).mem_read(
                    addr + offsetof(device_side, chained_request));
              }
              get_barrier(x, y).wait();
            }
          }
        /// Wait until all kernels have finished.
      } while (done_counter < x_size * y_size);
    }
  };
#endif

  struct device_side {
#if defined(__SYCL_XILINX_AIE__)
    static device_side *get() {
      return hw::get_object<device_side>(
          acap::hw::offset_table::get_rpc_record_begin_offset());
    }
    soft_barrier::device_side barrier;
    Var data;
    uint32_t ret_val;

    /// This asks the host to wait for on other request from the same device
    /// after processing this request. this exist to prevent le host from
    /// interleaving log requests.
    uint32_t chained_request;

    /// This sent data to the host to be processed.
    template <typename Ty> uint32_t perform(Ty &&d, bool chained = false) {
      /// Write the data.
      data = d;
      chained_request = chained;
      /// Notify the host of the data being available.
      barrier.wait();
      /// Wait for the host to process the data.
      barrier.wait();
      return ret_val;
    }
#endif
  };
};

using rpc = rpc_impl<done_rpc, image_update_rpc, send_log_rpc, host_breakpoint>;

#if defined(__SYCL_XILINX_AIE__)
namespace {
detail::assert_equal_layout<
    rpc::device_side, rpc::device_side,
    /*expected_size=*/hw::offset_table::get_rpc_record_size()>
    rpc_check;
}
#endif

} // namespace trisycl::vendor::xilinx::acap::aie

#endif
