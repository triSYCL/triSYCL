/** \file

    Implement OpenCL address spaces in SYCL with C++-style.

    Ronan.Keryell at AMD point com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl {
namespace sycl {
namespace trisycl {

/** \addtogroup address_spaces
    @{
*/


/** Generate a type with some real OpenCL 2 attribute if we are on an
    OpenCL device

    In the general case, do not add any OpenCL address space qualifier */
template <typename T, address_space AS>
struct OpenCLType {
  using type = T;
};

/// Add an attribute for __constant address space
template <typename T>
struct OpenCLType<T, constant_address_space> {
  using type =
#ifdef __SYCL_DEVICE_ONLY__
    __constant
#endif
    T;
};

/// Add an attribute for __generic address space
template <typename T>
struct OpenCLType<T, generic_address_space> {
  using type =
#ifdef __SYCL_DEVICE_ONLY__
    __generic
#endif
    T;
};

/// Add an attribute for __global address space
template <typename T>
struct OpenCLType<T, global_address_space> {
  using type =
#ifdef __SYCL_DEVICE_ONLY__
    __global
#endif
    T;
};

/// Add an attribute for __local address space
template <typename T>
struct OpenCLType<T, local_address_space> {
  using type =
#ifdef __SYCL_DEVICE_ONLY__
    __local
#endif
    T;
};

/// Add an attribute for __private address space
template <typename T>
struct OpenCLType<T, private_address_space> {
  using type =
#ifdef __SYCL_DEVICE_ONLY__
    __private
#endif
    T;
};


/** Implementation for an OpenCL address space pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
template <typename T, address_space AS>
class AddressSpaceImpl {
  // Verify that \a T is really a pointer
  static_assert(std::is_pointer<T>::value,
                "T must be a pointer type");

protected:

  /** Store the real pointer value with the right OpenCL address space
      qualifier if any

      Use a protected member so that the interface class can access the
      pointer field for assignment.
  */
  typename OpenCLType<T, AS>::type pointer;


public:

  /** Synthesized default constructors

      This ensures that we can write for example
      \code
        generic<float *> q;
      \endcode
      without initialization.
  */
  AddressSpaceImpl() = default;

  /** Set the address_space identifier that can be queried to know the
      pointer type */
  static auto constexpr address_space = AS;

  /** Allow conversion from a normal pointer
  */
  AddressSpaceImpl(T && v) : pointer(v) { }

  /** Conversion operator to allow for example a \c private<> pointer
      object to be used as a normal pointer (but with \c __private
      qualifier on an OpenCL target)
  */
  operator typename OpenCLType<T, AS>::type &() { return pointer; }

#if 0
  /** Implement the assignment operator because the copy constructor in
      the implementation is made explicit and the assignment operator is
      not automatically synthesized */
  AddressSpaceImpl & operator =(T v) {
    AddressSpaceImpl<T, AS>::pointer = v;
    /* Return the generic pointer so we may chain some side-effect
       operators */
    return *this; }
#endif

};

/// @} End the address_spaces Doxygen group

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
