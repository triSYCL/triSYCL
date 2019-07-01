#ifndef TRISYCL_SYCL_DETAIL_UNIMPLEMENTED_HPP
#define TRISYCL_SYCL_DETAIL_UNIMPLEMENTED_HPP

/** \file Deal with unimplemented features
    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <iostream>

namespace trisycl::detail {

/** \addtogroup helpers Some helpers for the implementation
    @{
*/

/** Display an "unimplemented" message

    Can be changed to call assert(0) or whatever.
*/
inline void unimplemented(const char *func, const char *file, int line) {
#ifndef NDEBUG
  std::cerr << "Error: using a non implemented feature : " << func << std::endl
            << "at " << file << ":" << line << std::endl
            << "Please contribute to the open source implementation. :-)"
            << std::endl;
#endif
}

#define TRISYCL_UNIMPL trisycl::detail::unimplemented(__func__, __FILE__, __LINE__)
/// @} End the helpers Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_UNIMPLEMENTED_HPP
