#ifndef TRISYCL_SYCL_DETAIL_INSTANTIATE_KERNEL_HPP
#define TRISYCL_SYCL_DETAIL_INSTANTIATE_KERNEL_HPP

/** \file A function to outline a SYCL kernel at a specific place

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <tuple>

#include "CL/sycl/command_group/detail/task.hpp"

namespace cl {
namespace sycl {
namespace detail {

/** Avoid the interprocedural optimization to remove these arguments
    in the kernel instantiation by relying on some dummy static variables.

    Using \c __attribute__((used)) does not work on arguments but only
    on static variable, so use this function.
*/
auto prevent_arguments_from_optimization = [] (auto & ...args) {
  /* Just keep track of the address of all the given objects,
     otherwise the objects are copied, may throw, are registered for
     destruction with \c atexit(), etc. */
  static auto __attribute__((used)) keep = std::make_tuple(& args...);
};


/** Instantiate the template code

    To have a clear view of what a kernel is at the LLVM IR level even
    when a lot of optimizations are done (-O3), call the kernel from
    this function marked as "noinline", otherwise the kernel code is
    inlined directly where it is used.
*/
template <typename KernelName,
          typename Functor>
__attribute__((noinline))
void instantiate_kernel(
#ifdef TRISYCL_GENERATE_KERNEL_CALLER
      /* If we generate the kernel calling code, pass the task
         information too */
                        detail::task &task,
#endif
                        Functor f) noexcept {
#ifdef TRISYCL_GENERATE_KERNEL_CALLER
  /* Keep the task alive here even through interprocedural dead code
     elimination because Clang/LLVM do not know that the device
     compiler will further massage deeply this code by removing the
     body and replace it by something completely different... */
  prevent_arguments_from_optimization(task);
#endif

  /* The outlining compiler is expected to do some massage here and to
     insert some calls to \c serialize_arg and so on using \c k and \c
     q */
  f();
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
