#ifndef TRISYCL_SYCL_DETAIL_CACHE_HPP
#define TRISYCL_SYCL_DETAIL_CACHE_HPP

/** \file A simple thread-safe cache

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>
#include <mutex>
#include <unordered_map>

namespace trisycl::detail {


/** A simple thread safe cache mechanism to cache std::shared_ptr of
    values indexed by keys

    Since internally only std::weak_ptr are stored, this does not
    prevent object deletion but it is up to the programmer not to use
    this cache to retrieve deleted objects.
*/
template <typename Key, typename Value>
class cache {

public:

  /// The type of the keys used to indexed the cache
  using key_type = Key;

  /// The base type of the values stored in the cache
  using value_type = Value;

private:

  /// The caching storage
  std::unordered_map<key_type, std::weak_ptr<value_type>> c;

  /// To make the cache thread-safe
  std::mutex m;

public:

  /** Get a value stored in the cache if present or insert by calling
      a generator function

      \param[in] k is the key used to retrieve the value

      \param[in] create_element is the function to be called if the
      key is not found in the cache to generate a value which is
      inserted for the key. This function has to produce a value
      convertible to a shared_ptr

      \return a shared_ptr to the value retrieved or inserted
  */
  template <typename Functor>
  std::shared_ptr<value_type> get_or_register(const key_type &k,
                                              Functor &&create_element) {
    std::lock_guard<std::mutex> lg { m };

    auto i = c.find(k);
    if (i != c.end())
      if (auto observe = i->second.lock())
        // Returns \c shared_ptr only if target object is still alive
        return observe;

    // Otherwise create and insert a new element
    std::shared_ptr<value_type> e { create_element() };
    c.insert({ k, e });
    return e;
  }


  /** Remove an entry from the cache

      \param[in] k is the key associated to the value to remove from
      the cache
  */
  void remove(const key_type &k) {
    std::lock_guard<std::mutex> lg { m };
    c.erase(k);
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

#endif // TRISYCL_SYCL_DEVICE_CACHE_HPP
