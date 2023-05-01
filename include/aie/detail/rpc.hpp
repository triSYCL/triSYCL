#ifndef AIE_DETAIL_RPC_HPP
#define AIE_DETAIL_RPC_HPP

#include "hardware.hpp"
#include "sync.hpp"
#include "xaie_wrapper.hpp"

namespace aie::detail {

struct send_log_rpc {
  /// This is the data that will be transmitted to the host when the device
  /// is logging. This struct needs to have the same layout on the host and the
  /// device.
  struct data_type {
    /// Pointer to the first character of a buffer to print
    hw::dev_ptr<const char> data;
    /// Number of characters to print
    uint32_t size;
  };
  static uint32_t act_on_data(int x, int y, xaie::handle h,
                              data_type dev_data) {
#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
    std::string str;
    str.resize(dev_data.size);
    /// Copy the indicated device data into a string.
    h.moved(hw::position{x, y}.moved(dev_data.data.get_dir()))
        .memcpy_d2h(str.data(), dev_data.data.get_offset(), str.size());
    std::cout << str << std::flush;
#endif
    return 0;
  }
};

struct done_rpc {
  enum exit_code : int32_t {
    ec_normal = 0,
    ec_assert = 1,
  };
  /// it needs no data, since it is just a signal.
  struct data_type {
    exit_code ec;
  };
  static uint32_t act_on_data(int x, int y, xaie::handle h, data_type d) {
    assert(d.ec != ec_assert && "propagating device assert to the host");
    return 0;
  }
};

}

#endif
