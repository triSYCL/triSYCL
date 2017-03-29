#ifndef TRISYCL_SYCL_CONTEXT_DETAIL_OPENCL_CONTEXT_HPP
#define TRISYCL_SYCL_CONTEXT_DETAIL_OPENCL_CONTEXT_HPP

/** \file The SYCL OpenCL context implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


#include <boost/compute.hpp>

#include "CL/sycl/detail/default_classes.hpp"

#include "CL/sycl/platform.hpp"
#include "CL/sycl/info/param_traits.hpp"
#include "CL/sycl/exception.hpp"
#include "CL/sycl/context/detail/context.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/detail/cache.hpp"


namespace cl {
namespace sycl {
namespace detail {

class opencl_context : public detail::context {

    boost::compute::context c;

    /** A cache to always return the same alive context for a given OpenCL
      context

      C++11 guaranties the static construction is thread-safe
  */
  static detail::cache<cl_context, detail::opencl_context> cache;

private:
  opencl_context(const boost::compute::context &c) : c { c } {}
  
  
public:
    cl_context get() const override {
	return c.get();
    }

    boost::compute::context get_boost_compute() const override {
	return c;
    }

    bool is_host() const override {
	return false;
    }
    
#if 0
    template <info::context Param>
    typename info::param_traits<info::context, Param>::type
    get_info() const override{
	detail::unimplemented();
	return {};
    }
#endif
    
    cl::sycl::platform get_platform() const override {
	detail::unimplemented();
	return {};
    }

    vector_class<cl::sycl::device>
    get_devices() const override {
	detail::unimplemented();
	return {};
    }

  ///// Get a singleton instance of the opencl_queue
  static std::shared_ptr<opencl_context>
  instance(const boost::compute::context &c) {
    return cache.get_or_register(c.get(),
                                 [&] { return new opencl_context { c }; });
  }

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
