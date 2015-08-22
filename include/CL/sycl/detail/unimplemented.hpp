#ifndef TRISYCL_SYCL_DETAIL_UNIMPLEMENTED_HPP
#define TRISYCL_SYCL_DETAIL_UNIMPLEMENTED_HPP

/** \file Deal with unimplemented features
    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <iostream>

namespace cl {
namespace sycl {
namespace detail {

/** \addtogroup debug_trace Debugging and tracing support
    @{
*/

/** Display an "unimplemented" message

    Can be changed to call assert(0) or whatever.
*/
inline void unimplemented() {
  std::cerr << "Error: using a non implemented feature!!!" << std::endl
            << "Please contribute to the open source implementation. :-)"
            << std::endl;
}

/// @} End the debug_trace Doxygen group

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

#endif // TRISYCL_SYCL_DETAIL_UNIMPLEMENTED_HPP
