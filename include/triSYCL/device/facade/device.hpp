#ifndef TRISYCL_SYCL_DEVICE_DETAIL_DEVICE_FACADE_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_DETAIL_DEVICE_FACADE_DEVICE_HPP

/** \file A SYCL device facade to implement more easily a SYCL
    device concept

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/detail/shared_ptr_implementation.hpp"

namespace trisycl::facade {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/// SYCL device
template <typename UserFacing, typename ImplementationDetail>
class device
  /* Use the underlying device implementation that can be shared in the
     SYCL model */
  : public detail::shared_ptr_implementation<UserFacing, ImplementationDetail> {

public:

  /// The type encapsulating the implementation
  using implementation_t =
    detail::shared_ptr_implementation<UserFacing, ImplementationDetail>;

  /// The real detail type doing the work behind the scene
  using detail_t = ImplementationDetail;

  /// Import the constructors
  using implementation_t::implementation_t;

};

/// @} to end the Doxygen group

}

#endif // TRISYCL_SYCL_DEVICE_DETAIL_DEVICE_FACADE_DEVICE_HPP
