#ifndef TRISYCL_SYCL_CONTEXT_DETAIL_OPENCL_CONTEXT_HPP
#define TRISYCL_SYCL_CONTEXT_DETAIL_OPENCL_CONTEXT_HPP

/** \file The SYCL OpenCL context implementation

    Anastasi at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


#include <boost/compute.hpp>

#include "CL/sycl/detail/default_classes.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/detail/cache.hpp"

#include "CL/sycl/context/detail/context.hpp"
#include "CL/sycl/platform.hpp"
#include "CL/sycl/info/param_traits.hpp"
#include "CL/sycl/exception.hpp"


namespace cl {
namespace sycl {
namespace detail {

/// SYCL OpenCL context
class opencl_context : public detail::context {

  /// User the Boost Compute abstraction of the OpenCL context
  boost::compute::context c;

  /** A boost command_queue associated to an OpenCL context for
      when we need to transfer data but no queue is given
  */
  boost::compute::command_queue q;

  /** A cache to always return the same alive context for a given OpenCL
      context

      C++11 guaranties the static construction is thread-safe
  */
  static detail::cache<cl_context, detail::opencl_context> cache;

public:

  /// Return the underlying cl_context of the cl::sycl::context
  cl_context get() const override {
    return c.get();
  }


  /// Return the underlying boost::compute::context of the cl::sycl::context
  boost::compute::context &get_boost_compute() override {
    return c;
  }


  /// Return the queue that is associated to the context
  boost::compute::command_queue &get_boost_queue() {
    return q;
  }


  /// Return false because the context is not a SYCL host context
  bool is_host() const override {
    return false;
  }


#if 0
  /** Query the context for OpenCL info::context info

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
  template <info::context Param>
  typename info::param_traits<info::context, Param>::type
  get_info() const override{
    detail::unimplemented();
    return {};
  }
#endif


  /** Return the platform of the context

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
  cl::sycl::platform get_platform() const override {
    detail::unimplemented();
    return {};
  }


  /** Returns the set of devices that are part of this context.

      \todo To be implemented
  */
  vector_class<cl::sycl::device> get_devices() const override {
    detail::unimplemented();
    return {};
  }

  /// Get a singleton instance of the opencl_context
  static std::shared_ptr<opencl_context>
  instance(const boost::compute::context &c) {
    return cache.get_or_register(c.get(),
                                 [&] { return new opencl_context { c }; });
  }

private:

  /// Only the instance factory can build it
  opencl_context(const boost::compute::context &c) :
    c { c },
    q { boost::compute::command_queue { c, c.get_device() } } {}


public:
  /// Unregister from the cache on destruction
  ~opencl_context() override {
    cache.remove(c.get());
  }

};

/* Allocate the cache here but since this is a pure-header library,
   use a weak symbol so that only one remains when SYCL headers are
   used in different compilation units of a program
*/
TRISYCL_WEAK_ATTRIB_PREFIX
detail::cache<cl_context, detail::opencl_context> opencl_context::cache
TRISYCL_WEAK_ATTRIB_SUFFIX;

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif //TRISYCL_SYCL_CONTEXT_DETAIL_OPENCL_CONTEXT_HPP
