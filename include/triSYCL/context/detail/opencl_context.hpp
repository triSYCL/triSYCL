#ifndef TRISYCL_SYCL_CONTEXT_DETAIL_OPENCL_CONTEXT_HPP
#define TRISYCL_SYCL_CONTEXT_DETAIL_OPENCL_CONTEXT_HPP

/** \file The SYCL OpenCL context implementation

    a-doumoulakis at gmail dot com (Anastasios Doumoulakis)

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


#include <boost/compute.hpp>

#include "triSYCL/detail/default_classes.hpp"
#include "triSYCL/detail/unimplemented.hpp"
#include "triSYCL/detail/cache.hpp"

#include "triSYCL/context/detail/context.hpp"
#include "triSYCL/platform.hpp"
#include "triSYCL/info/param_traits.hpp"
#include "triSYCL/exception.hpp"


namespace trisycl::detail {

/// SYCL OpenCL context
class opencl_context : public detail::context {

  /// User the Boost Compute abstraction of the OpenCL context
  boost::compute::context c;

  /** A boost \c command_queue associated to an OpenCL context for
      when we need to transfer data but no queue is given
      (eg. When an buffer accessor is created)
  */
  boost::compute::command_queue q;

  /** A cache to always return the same alive context for a given OpenCL
      context

      C++11 guaranties the static construction is thread-safe
  */
  static detail::cache<cl_context, detail::opencl_context> cache;

public:

  /// Return the underlying \c cl_context of the \c trisycl::context
  cl_context get() const override {
    return c.get();
  }


  /** Return the underlying \c boost::compute::context
      of the \c trisycl::context
  */
  boost::compute::context &get_boost_compute() override {
    return c;
  }


  /// Return the queue that is associated to the context
  boost::compute::command_queue &get_boost_queue() override {
    return q;
  }


  /// Return false because the context is not a SYCL host context
  bool is_host() const override {
    return false;
  }

#if 0
  /** Query the context for OpenCL \c info::context info

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
  template <info::context Param>
  typename info::param_traits<info::context, Param>::type
  get_info() const override{
    TRISYCL_UNIMPL;
    return {};
  }
#endif

  /** Return the platform of the context

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
  trisycl::platform get_platform() const override {
    return trisycl::platform(c.get_devices()[0].platform());
  }


  /** Returns the set of devices that are part of this context.

      \todo To be implemented
  */
  vector_class<trisycl::device> get_devices() const override {
    vector_class<trisycl::device> devices;

    for (const auto &d : c.get_devices())
      devices.push_back(d);
    return devices;
  }

  trisycl::cl_uint get_reference_count() const override {
    return c.get_info<CL_CONTEXT_REFERENCE_COUNT>();
  }

  /// Get a singleton instance of the \c opencl_context
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

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif //TRISYCL_SYCL_CONTEXT_DETAIL_OPENCL_CONTEXT_HPP
