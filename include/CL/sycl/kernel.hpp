#ifndef TRISYCL_SYCL_KERNEL_HPP
#define TRISYCL_SYCL_KERNEL_HPP

/** \file The OpenCL SYCL nd_range<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include "CL/sycl/kernel/detail/kernel.hpp"

namespace cl {
namespace sycl {

// SYCL2.1: Trivial kernel wrapper for separate compilation
template<class KernelFunction, class Index, typename... Ts>
class Kernel
{
    std::function<KernelFunction> f_;

public:
    Kernel(KernelFunction f) :
        f_{ f }
    {
    }

    /**
    * Create helper kernel object with captured args set
    */
    auto operator() (
        Ts... ts)
    {
        return detail::KernelHelper<KernelFunction, Index, Ts...>(f_, ts...);
    }
};

// SYCL2.1: Trivial program class stub
class Program
{
public:
    template<class KernelFunctor, typename Index, typename...Ts>
    auto getKernel(KernelFunctor f)
    {
        return Kernel<KernelFunctor, Index, Ts... >(f);
    }
};
/// @} End the parallelism Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_KERNEL_HPP
