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

/// Type agnostic accessor representation used for automatic data transfers to
/// the device.
struct device_accessor_base {
  /// Address of the data.
  hw::dev_ptr<void> ptr;
  /// Size if bytes of the data.
  int size;
};

/// User-facing accessor on device.
template <typename DataType, int Dimensions, access::mode AccessMode,
          access::target Target = access::target::global_buffer>
class device_accessor : device_accessor_base {
  using base = device_accessor_base;
  static_assert(Dimensions == 1, "only 1d is supported for now");

  /// Get a properly typed pointer to the data.
  DataType* get_ptr() const {
    return reinterpret_cast<DataType*>(base::ptr.get());
  }

public:
  /// Constructor from a buffer. Only exists for compilation purposes and should
  /// never be invoked.
  template <typename Buffer>
  device_accessor(const Buffer&) requires (!std::is_same_v<Buffer, device_accessor>) {
    /// This exists only for the purpose of compilation and should never be
    /// called at runtime.
    assert(false && "should never be executed");
  }
  /// Copy constructor
  device_accessor(const device_accessor&) = default;

  DataType* begin() const { return get_ptr(); }
  DataType* end() const { return begin() + get_count(); }
  DataType& operator[](size_t index) const { return get_ptr()[index]; }
  /// Return the number of elements in the data
  size_t get_count() const { return base::size / sizeof(DataType); }
  /// Return the size in bytes of the data
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
using base_accessor = typename std::conditional_t<
    is_device,
    base_accessor_device<DataType, Dimensions, AccessMode,
                         access::target::global_buffer>,
    base_accessor_host<DataType, Dimensions, AccessMode,
                       access::target::host_buffer>>;

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
/// Because it is the current size of the host side accessor and accessors need
/// to have the same size and alignment on host and device such that host and
/// device accessor can be subsituted in structs containing them like kernel
/// lambda captures.
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
namespace {
detail::assert_equal_layout<
    accessor<int, 1, access::mode::read_write, access::target::global_buffer,
             /*is_device=*/true>,
    accessor<int, 1, access::mode::read_write, access::target::host_buffer,
             /*is_device=*/false>,
    16, 8>
    accessor_check;
}

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ACCESSOR_HPP
