#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ACCESSOR_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ACCESSOR_HPP

/** \file

    SYCL accessor for AIE tile device

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <utility>

#include "hardware.hpp"
#include "triSYCL/device/facade/device.hpp"
#include "triSYCL/detail/layout_utils.hpp"

/// \ingroup aie
/// @{

namespace trisycl::vendor::xilinx::acap::aie {

struct device_accessor_base {
  mutable hw::dev_ptr<void> ptr;
  int size;
};

template <typename DataType, int Dimensions, access::mode AccessMode,
          access::target Target = access::target::global_buffer>
class device_accessor : device_accessor_base {
  using base = device_accessor_base;
  static_assert(Dimensions == 1, "only 1d is supported for now");
  hw::dev_ptr<DataType> get_ptr() const {
    return (hw::dev_ptr<DataType>)base::ptr;
  }

public:
  template <typename Buffer,
            typename std::enable_if<
                !std::is_same<Buffer, device_accessor>::value, int>::type = 0>
  device_accessor(const Buffer&) {
    /// This exists only for the purpose of compilation and should never be
    /// called at runtime.
    assert(false && "should never be executed");
  }
  device_accessor(const device_accessor&) = default;

  DataType *begin() const { return get_ptr().get(); }
  DataType *end() const { return begin() + get_count(); }
  DataType &operator[](size_t index) const { return get_ptr()[index]; }
  size_t get_count() const { return base::size / sizeof(DataType); }
  size_t get_size() const { return base::size; }
};

/** Create a SYCL-like device view of an AI Engine CGRA with some layout

    \param Layout is the layout description of the machine to
    instantiate with the physical size
*/

template <typename DataType, int Dimensions, access::mode AccessMode,
          access::target Target = access::target::global_buffer>
using base_accessor_device =
    device_accessor<DataType, Dimensions,
                    AccessMode, Target>;

template <typename DataType, int Dimensions, access::mode AccessMode,
          access::target Target = access::target::host_buffer>
using base_accessor_host =
    ::trisycl::accessor<DataType, Dimensions,
                        AccessMode, Target>;

template <typename DataType, int Dimensions, access::mode AccessMode,
          access::target Target, bool is_device>
using base_accessor = typename std::conditional<
    is_device, base_accessor_device<DataType, Dimensions, AccessMode, access::target::global_buffer>,
    base_accessor_host<DataType, Dimensions, AccessMode, access::target::host_buffer>>::type;

template <typename DataType, int Dimensions, access::mode AccessMode,
          access::target Target = access::target::global_buffer,
          bool is_device
#ifdef __SYCL_DEVICE_ONLY__
          = true
#else
          = false
#endif
          >
/// This class will have sizeof = 16 and alignof = 8 on both device and host
class alignas(8) accessor
    : public base_accessor<DataType, Dimensions, AccessMode, Target, is_device>,
      detail::padding<16 -
                      sizeof(base_accessor<DataType, Dimensions, AccessMode,
                                           Target, is_device>)> {

  using base =
      base_accessor<DataType, Dimensions, AccessMode, Target, is_device>;

public:
  //   using typename base::accessor;
  template<typename Buffer>
  accessor(Buffer &b, auto &cgh) : base{b} {
    cgh.add_accessor(*this);
  }
};

template <typename Buffer>
accessor(Buffer, auto &)
    -> accessor<typename Buffer::value_type, Buffer::rank(),
                access::mode::read_write, access::target::global_buffer>;

detail::assert_equal_layout<
    accessor<int, 1, access::mode::read_write, access::target::global_buffer,
             /*is_device=*/true>,
    accessor<int, 1, access::mode::read_write, access::target::host_buffer,
             /*is_device=*/false>
             ,16, 8
    >accessor_check;

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ACCESSOR_HPP
