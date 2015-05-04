/** \file

    This is a simple C++ sequential OpenCL SYCL implementation to
    experiment with the OpenCL CL provisional specification.

    Ronan.Keryell at AMD point com

    Copyright 2014 Advanced Micro Devices, Inc.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <deque>
#include <functional>
#include <type_traits>
#include <iostream>

#include <boost/multi_array.hpp>

/// \todo Move into files really using it
#include "CL/sycl/detail/debug.hpp"

namespace cl {
namespace sycl {
namespace trisycl {

template <typename T,
          std::size_t dimensions,
          access::mode mode,
          access::target target = access::global_buffer>
struct AccessorImpl;
class Task;

}
}
}

/// triSYCL implementation dwells in the cl::sycl::trisycl namespace
namespace cl {
namespace sycl {

// Forward declaration for the buffer<> and the accessor<>
template <std::size_t dims> struct item;
template <std::size_t dims> struct nd_item;

namespace trisycl {

/** \addtogroup data
    @{
*/


/// @} to end the data Doxygen group

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
