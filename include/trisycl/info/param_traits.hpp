#ifndef TRISYCL_SYCL_INFO_PARAM_TRAITS_HPP
#define TRISYCL_SYCL_INFO_PARAM_TRAITS_HPP

/** \file The OpenCL SYCL param_traits

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl::info {

/** Implement a meta-function from (T, value) to T' to express the return type
    value of an OpenCL function of kind (T, value)
*/
template <typename T, T Param>
struct param_traits {
  // By default no return type
};


/// To declare a param_traits returning RETURN_TYPE for function of any T
#define TRISYCL_INFO_PARAM_TRAITS_ANY_T(T, RETURN_TYPE)     \
  template <T Param>                                        \
  struct param_traits<T, Param> {                           \
    using return_type = RETURN_TYPE;                        \
  };


/** To declare a param_traits returning RETURN_TYPE for function taking a
    VALUE of type T
*/
#define TRISYCL_INFO_PARAM_TRAITS(VALUE, RETURN_TYPE)       \
  template <>                                               \
  struct param_traits<decltype(VALUE), VALUE> {             \
    using return_type = RETURN_TYPE;                        \
  };

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_INFO_PARAM_TRAITS_HPP
