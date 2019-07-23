#ifndef TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_BASE_HPP
#define TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_BASE_HPP

/** \file The OpenCL SYCL buffer accessor_base<> detail behind the
    scene to store the data-independent of the accessor

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "triSYCL/command_group/detail/task.hpp"
#include "triSYCL/detail/debug.hpp"

namespace trisycl {

class handler;

namespace detail {

struct task;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The buffer accessor base is here mainly to be accessed from the
    task without dependency on the buffer data types
*/
class accessor_base {

protected:

  /// The task where the accessor is used in
  std::shared_ptr<detail::task> task;

  /// The declaration order of the accessor
  std::size_t order;

public:

  /// Set the order of the accessor
  void set_order(std::size_t o) {
    order = o;
  }


  /// Get the order of the accessor
  auto get_order() {
    return order;
  }


#ifdef TRISYCL_OPENCL
  /// Get the boost::compute::buffer or throw if unset
  virtual boost::compute::buffer get_cl_buffer() const = 0;

  // Virtual to call the real destructor
  virtual ~accessor_base() {}
#endif

};

/// @} End the data Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_BASE_HPP
