#ifndef TRISYCL_SYCL_DETAIL_SINGLETON_HPP
#define TRISYCL_SYCL_DETAIL_SINGLETON_HPP

/** \file Mix-in to add a singleton implementation with an instance() method

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <functional>
#include <memory>

#include <boost/core/null_deleter.hpp>
#include <boost/operators.hpp>


namespace trisycl::detail {

/// Provide a singleton factory
template <typename T>
struct singleton {

  /// Get a singleton instance of T
  static std::shared_ptr<T> instance() {
    // C++11 guaranties the static construction is thread-safe
    static T single;
    /** Use a null_deleter since the singleton should not be deleted,
        as allocated in the static area */
    static std::shared_ptr<T> sps { &single,
                                    boost::null_deleter {} };

    return sps;
  }

};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_SINGLETON_HPP
