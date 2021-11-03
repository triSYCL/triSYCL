#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ACCESSOR_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ACCESSOR_HPP

/** \file

    SYCL accessor for AIE tile device

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <utility>

#include "device/detail/device.hpp"
#include "hardware.hpp"
#include "triSYCL/device/facade/device.hpp"

/// \ingroup aie
/// @{

namespace trisycl::vendor::xilinx::acap::aie {

template <typename DataType, int Dimensions, access::mode AccessMode,
          access::target Target = access::target::global_buffer>
class device_accessor {
  static_assert(Dimensions == 1, "only 1d is supported for now");
  mutable hw::dev_ptr<DataType> Ptr;
  int Size;

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

  DataType *begin() const { return Ptr.get(); }
  DataType *end() const { return begin() + Size; }
  DataType &operator[](size_t index) const { return Ptr[index]; }
  size_t get_count() const { return Size; }
  size_t get_size() const { return Size * sizeof(DataType); }
};

class empty {};

template <size_t Size> class non_zero_padding { char padding[Size]; };

/// This should be used as a base class or it wont be able to have 0 padding
/// added.
template <size_t Size>
class padding
    : std::conditional<Size == 0, empty, non_zero_padding<Size>>::type {};

/** Create a SYCL-like device view of an AI Engine CGRA with some layout

    \param Layout is the layout description of the machine to
    instantiate with the physical size
*/

template <typename Buffer>
using base_accessor_device =
    device_accessor<typename Buffer::value_type, Buffer::rank(),
                    access::mode::read_write>;

template <typename Buffer>
using base_accessor_host =
    ::trisycl::accessor<typename Buffer::value_type, Buffer::rank(),
                        access::mode::read_write, access::target::host_buffer>;

template <typename Buffer, bool is_device = false>
using base_accessor =
    typename std::conditional<is_device, base_accessor_device<Buffer>,
                              base_accessor_host<Buffer>>::type;

template <typename Buffer, bool is_device
#ifdef __SYCL_DEVICE_ONLY__
                           = true
#else
                           = false
#endif
>
class accessor
    : public base_accessor<Buffer, is_device>,
    padding<std::max(sizeof(base_accessor_device<Buffer>), sizeof(base_accessor_host<Buffer>)
     - sizeof(base_accessor<Buffer, is_device>))> {

  using base = base_accessor<Buffer, is_device>;

public:
  //   using typename base::accessor;
  accessor(Buffer &b, auto &cgh) : base{b} {}
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ACCESSOR_HPP
