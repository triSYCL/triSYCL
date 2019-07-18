#ifndef TRISYCL_SYCL_DETAIL_ALL_TRUE_HPP
#define TRISYCL_SYCL_DETAIL_ALL_TRUE_HPP

/** \file Add all_true wrapper.
    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl::detail {

template <bool...> struct bool_pack;

template <bool... v>
using all_true = std::is_same<bool_pack<true, v...>, bool_pack<v..., true>>;

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_ALL_TRUE_HPP
