#ifndef TRISYCL_SYCL_PARALLELISM_HPP
#define TRISYCL_SYCL_PARALLELISM_HPP

/** \file

    Implement parallel constructions to launch kernels

    Ronan at keryell dot FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/parallelism/detail/parallelism.hpp"

namespace cl {
namespace sycl {

/** \addtogroup parallelism
    @{
*/


/** SYCL single_task launches a computation without parallelism at launch
    time.

    \param F specify the kernel to be launched as a single_task

    \param KernelName is a class type that defines the name to be used for
    the underlying kernel

    \todo Right now the implementation does nothing else that forwarding
    the execution of the given functor
*/
template <typename KernelName = std::nullptr_t>
void single_task(std::function<void(void)> F) {
  CurrentTask->schedule(F);
}


/** SYCL parallel_for launches a data parallel computation with parallelism
    specified at launch time by a range<>

    \param global_size is the full size of the range<>

    \param N dimensionality of the iteration space

    \param f is the kernel functor to execute

    \param KernelName is a class type that defines the name to be used for
    the underlying kernel

    Unfortunately, to have implicit conversion to work on the range, the
    function can not be templated, so instantiate it for all the
    dimensions
*/
#define TRISYCL_parallel_for_functor_GLOBAL(N)                          \
  template <typename KernelName = std::nullptr_t,                       \
            typename ParallelForFunctor>                                \
  void parallel_for(range<N> global_size,                               \
                    ParallelForFunctor f) {                             \
    CurrentTask->schedule([=] { detail::parallel_for(global_size, f); }); \
  }

TRISYCL_parallel_for_functor_GLOBAL(1)
TRISYCL_parallel_for_functor_GLOBAL(2)
TRISYCL_parallel_for_functor_GLOBAL(3)


/** A variation of SYCL parallel_for to take into account a nd_range<>

    \param r defines the iteration space with the work-group layout and
    offset

    \param Dimensions dimensionality of the iteration space

    \param f is the kernel functor to execute

    \param ParallelForFunctor is the kernel functor type

    \param KernelName is a class type that defines the name to be used for
    the underlying kernel
*/
template <typename KernelName,
          std::size_t Dimensions,
          typename ParallelForFunctor>
void parallel_for(nd_range<Dimensions> r, ParallelForFunctor f) {
  CurrentTask->schedule([=] { detail::parallel_for(r, f); });
}


  /** SYCL parallel_for launches a data parallel computation with
    parallelism specified at launch time by 1 range<> and an offset

    \param global_size is the global size of the range<>

    \param offset is the offset to be add to the id<> during iteration

    \param f is the kernel functor to execute

    \param ParallelForFunctor is the kernel functor type

    \param KernelName is a class type that defines the name to be used for
    the underlying kernel

    Unfortunately, to have implicit conversion to work on the range, the
    function can not be templated, so instantiate it for all the
    dimensions
*/
#define TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(N)     \
  template <typename KernelName = std::nullptr_t,       \
            typename ParallelForFunctor>                \
  void parallel_for(range<N> global_size,               \
                    id<N> offset,                       \
                    ParallelForFunctor f) {             \
    CurrentTask->schedule([=] {                         \
        detail::parallel_for_global_offset(global_size, \
                                           offset,      \
                                           f); });      \
  }

TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(1)
TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(2)
TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(3)


/// SYCL parallel_for version that allows a Program object to be specified
/// \todo To be implemented
/* template <typename Range, typename Program, typename ParallelForFunctor>
void parallel_for(Range r, Program p, ParallelForFunctor f) {
  /// \todo deal with Program
  parallel_for(r, f);
}
*/


/** Loop on the work-groups

    \param r defines the iteration space with the work-group layout and
    offset

    \param Dimensions dimensionality of the iteration space

    \param f is the kernel functor to execute

    \param ParallelForFunctor is the kernel functor type

    \param KernelName is a class type that defines the name to be used for
    the underlying kernel
*/
template <typename KernelName = std::nullptr_t,
          std::size_t Dimensions = 1,
          typename ParallelForFunctor>
void parallel_for_workgroup(nd_range<Dimensions> r,
                            ParallelForFunctor f) {
  CurrentTask->schedule([=] { detail::parallel_for_workgroup(r, f); });
}


/// Loop on the work-items inside a work-group
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void parallel_for_workitem(group<Dimensions> g, ParallelForFunctor f) {
  detail::parallel_for_workitem(g, f);
}

}
}

/// @} End the parallelism Doxygen group

#endif // TRISYCL_SYCL_PARALLELISM_HPP
