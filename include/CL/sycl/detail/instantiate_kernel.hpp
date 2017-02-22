#ifndef TRISYCL_SYCL_DETAIL_INSTANTIATE_KERNEL_HPP
#define TRISYCL_SYCL_DETAIL_INSTANTIATE_KERNEL_HPP

/** \file A function to outline a SYCL kernel at a specific place

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <boost/compute.hpp>

namespace cl {
namespace sycl {
namespace detail {

/** Avoid the interprocedural optimization to remove these arguments
    in the kernel instantiation by relying on some dummy external
    function.

    Using \c __attribute__((used)) in \c instantiate_kernel does not
    prevent aggressive optimizations to remove the arguments, so use
    this hack instead.

    The name of the function itself should give a hint to an end user
    trying to use this code as is without processing by the device
    compiler
*/
extern void
to_be_massaged_by_outlining_compiler_(boost::compute::kernel &k,
                                      boost::compute::command_queue &q);


/** Instantiate the template code

    To have a clear view of what a kernel is at the LLVM IR level even
    when a lot of optimizations are done (-O3), call the kernel from
    this function marked as "noinline", otherwise the kernel code is
    inlined directly where it is used.
*/
template <typename KernelName,
          typename Functor>
__attribute__((noinline))
void instantiate_kernel(boost::compute::kernel &k,
                        boost::compute::command_queue &q,
                        Functor f) noexcept {
  /* The outlining compiler is expected to do some massage here and to
     insert some calls to \c serialize_arg and so on using \c k and \c
     q */
  f();
  to_be_massaged_by_outlining_compiler_(k, q);
}

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

#endif // TRISYCL_SYCL_DETAIL_INSTANTIATE_KERNEL_HPP
