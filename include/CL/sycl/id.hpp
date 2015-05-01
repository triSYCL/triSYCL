#ifndef TRISYCL_SYCL_ID_HPP
#define TRISYCL_SYCL_ID_HPP

/** \file The OpenCL SYCL id<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/detail/small_array.hpp"

namespace cl {
namespace sycl {

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** Define a multi-dimensional index, used for example to locate a work item

    \todo The definition of id and item seem completely broken in the
    current specification. The whole 3.4.1 is to be updated.

    \todo It would be nice to have [] working everywhere, provide both
    get_...() and get_...(int dim) equivalent to get_...()[int dim]
    Well it is already the case for item. So not needed for id?
    Indeed [] is mentioned in text of page 59 but not in class description.
*/
template <std::size_t dims = 1>
struct id : public detail::small_array_123<std::ptrdiff_t, id<dims>, dims> {
  // Inherit of all the constructors
  using detail::small_array_123<std::ptrdiff_t,
                                id<dims>,
                                dims>::small_array_123;
};


/** Implement a make_id to construct an id<> of the right dimension with
    implicit conversion from an initializer list for example.

    Cannot use a template on the number of dimensions because the implicit
    conversion would not be tried. */
auto make_id(id<1> i) { return i; }
auto make_id(id<2> i) { return i; }
auto make_id(id<3> i) { return i; }


/** Construct an id<> from a function call with arguments, like
    make_id(1, 2, 3) */
template<typename... BasicType>
auto make_id(BasicType... Args) {
  return id<sizeof...(Args)>(Args...);
}

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

#endif // TRISYCL_SYCL_ID_HPP
