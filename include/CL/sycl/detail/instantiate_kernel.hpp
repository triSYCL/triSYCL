#ifndef TRISYCL_SYCL_DETAIL_INSTANTIATE_KERNEL_HPP
#define TRISYCL_SYCL_DETAIL_INSTANTIATE_KERNEL_HPP

/** \file A function to outline a SYCL kernel at a specific place

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl {
namespace sycl {
namespace detail {

#ifdef TRISYCL_DEVICE
/** Use a Clang user annotation attribute to mark arguments that takes
    a kernel so the compiler can outline the kernels

    __attribute__((annotate("noinline"))) does not help at the Functor
    parameter to avoid inlining of the final kernel use
 */
#define __TRISYCL_KERNEL_MARK __attribute__((annotate("__triSYCL_kernel")))
#else
#define __TRISYCL_KERNEL_MARK
#endif

/** Instantiate the template code

    To have a clear view of what a kernel is at the LLVM IR level even
    when a lot of optimizations are done (-O3), call the kernel from
    this function marked as "noinline", otherwise the kernel code is
    inlined directly where it is used.
 */
template <typename KernelName,
          typename Functor>
__attribute__((noinline))
void instantiate_kernel(Functor f __TRISYCL_KERNEL_MARK) noexcept {
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
