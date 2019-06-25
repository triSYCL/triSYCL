#ifndef TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_DETAIL_UTIL_HPP
#define TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_DETAIL_UTIL_HPP

/** \file Some supporting content for conceptual scope storage

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <type_traits>

namespace trisycl::vendor::trisycl::scope::detail {

template <typename ScopedType, typename StorageType = void>
struct storage_type_trait {
  using type = StorageType;
};


template <typename ScopedType>
struct storage_type_trait<ScopedType, typename ScopedType::storage_type> {
  using type = typename ScopedType::storage_type;
};


/// Retrieve the type of the scoped storage of the object or \c void if none
template <typename ScopedType>
using storage_type_trait_t = typename storage_type_trait<ScopedType>::type;

}


/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_DETAIL_UTIL_HPP
