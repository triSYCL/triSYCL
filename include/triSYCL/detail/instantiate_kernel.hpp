#ifndef TRISYCL_SYCL_DETAIL_INSTANTIATE_KERNEL_HPP
#define TRISYCL_SYCL_DETAIL_INSTANTIATE_KERNEL_HPP

/** \file Some helpers to share information between the SYCL program
    and the device compiler.

    For example, this brings a function to outline a SYCL kernel at a
    specific place.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <tuple>

#include "triSYCL/command_group/detail/task.hpp"

namespace trisycl::detail {

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


/** Instantiate the kernel code

    To have a clear view of what a kernel is at the LLVM IR level even
    when a lot of optimizations are done (-O3), call the kernel from
    this function marked as \c noinline, otherwise the kernel code is
    inlined directly where it is used.

    Actually some optimizations/code restructuring might duplicate the
    call to \c instantiate_kernel but the \c noduplicate attribute
    seems incompatible with \c noinline in Clang/LLVM 3.9.1, because
    the inlining happen when both attributes are used...
*/
template <typename KernelName,
          typename Functor>
__attribute__((noinline)) void
instantiate_kernel(Functor f) noexcept {
  /* The outlining compiler is expected to do some massage here or
     around and to insert some calls to \c serialize_arg and so on */
  f();
}


/** Set the task to be used to launch the kernel

    This is actually a dummy function so that the device compiler can
    just grab the task for passing it to the argument serialization
    functions from the device runtime.

    It is better to use a weak linkage than having this function with
    extern linkage:

    - if the device compiler is used the function is kept by the
      compiler because full link and LTO is not applied before the
      device compiler passes are applied. So there is no partial
      evaluation in the compiler according to what is returned by this
      function;

    - if the device compiler is not used, the code still links and can
      be compiled.
*/
TRISYCL_WEAK_ATTRIB_PREFIX void TRISYCL_WEAK_ATTRIB_SUFFIX
set_kernel_task_marker(detail::task &) noexcept {
}


/** Gather in a single function what is necessary for the device
    compiler to outline the kernel or to set up the kernel and
    serialize the arguments.

    The \c noinline attribute is used to force this to appear as a
    real function the device compiler can massage.
*/
template <typename KernelName,
          typename Kernel>
__attribute__((noinline))
void launch_device_kernel(detail::task &t,
                          Kernel k) noexcept {
  /** Setup the task to be used to launch the kernel.

      This is used by the outlining compiler. */
  detail::set_kernel_task_marker(t);
  /* A simplified version for the device just to be able to extract
     the kernel itself.

     It is not a real outlining but a good approximation to be
     massaged by a compiler later.

     Put the iteration space loops in the kernels for now, which makes
     sense for CPU emulation or FPGA pipelined execution.
  */
  instantiate_kernel<KernelName>(k);
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
