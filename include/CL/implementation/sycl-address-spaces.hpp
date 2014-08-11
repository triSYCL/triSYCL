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


/* Forward declare some classes to allow some recursion in conversion
   operators */
template <typename SomeType, address_space SomeAS>
struct AddressSpaceFundamentalImpl;

template <typename SomeType, address_space SomeAS>
struct AddressSpaceObjectImpl;

template <typename SomeType, address_space SomeAS>
struct AddressSpacePointerImpl;

/** Dispatch the address space implementation according to the requested type

    \param T is the type of the object to be created

    \param AS is the address space to place the object into or to point to
    in the case of a pointer type
*/
template <typename T, address_space AS>
using AddressSpaceImpl =
  typename std::conditional<std::is_pointer<T>::value,
                            AddressSpacePointerImpl<T, AS>,
                            typename std::conditional<std::is_class<T>::value,
                                                      AddressSpaceObjectImpl<T, AS>,
                                                      AddressSpaceFundamentalImpl<T, AS>
                                                      >::type>::type;


/** Implementation of the base infrastructure to wrap something in an
    OpenCL address space

    \param T is the type of the basic stuff to be created

    \param AS is the address space to place the object into

    \todo Verify/improve to deal with const/volatile?
*/
template <typename T, address_space AS>
struct AddressSpaceBaseImpl {
  /** Store the base type of the object

      \todo Add to the specification
  */
  using type = T;

  /** Store the base type of the object with OpenCL address space modifier

      \todo Add to the specification
  */
  using opencl_type = typename OpenCLType<T, AS>::type;

  /** Set the address_space identifier that can be queried to know the
      pointer type */
  static auto constexpr address_space = AS;

private:

  /* C++11 helps a lot to be able to have the same constructors as the
     parent class here

     \todo Add this to the list of required C++11 features needed for SYCL
  */
  opencl_type variable;

public:

  /** Allow to creating an address space version of an object or to
      convert one */
  AddressSpaceBaseImpl(const T & v) : variable(v) { }


  /** Also request for the default constructors that have been disabled by
      the declaration of another constructor

      This ensures for example that we can write
      \code
        generic<float *> q;
      \endcode
      without initialization.
  */
  AddressSpaceBaseImpl() = default;


  /** Allow for example assignment of a global<float> to a priv<double>
      for example

     Since it needs 2 implicit conversions, it does not work with the
     conversion operators already define, so add 1 more explicit
     conversion here so that the remaining implicit conversion can be
     found by the compiler.

     Strangely
     \code
     template <typename SomeType, address_space SomeAS>
     AddressSpaceBaseImpl(AddressSpaceImpl<SomeType, SomeAS>& v)
     : variable(SomeType(v)) { }
     \endcode
     cannot be used here because SomeType cannot be inferred. So use
     AddressSpaceBaseImpl<> instead
  */
  template <typename SomeType, cl::sycl::address_space SomeAS>
  AddressSpaceBaseImpl(AddressSpaceBaseImpl<SomeType, SomeAS>& v)
    : variable(SomeType(v)) { }


  /** Conversion operator to allow a AddressSpaceObjectImpl<T> to be used
      as a T so that all the methods of a T and the built-in operators for
      T can be used on a AddressSpaceObjectImpl<T> too */
  operator T & () { return variable; }

};


/** Implementation for an OpenCL address space pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.

    All the address space pointers inherit from it, which makes trivial
    the implementation of cl::sycl::multi_ptr<T, AS>
*/
template <typename T, address_space AS>
struct AddressSpacePointerImpl : public AddressSpaceBaseImpl<T, AS> {
  // Verify that \a T is really a pointer
  static_assert(std::is_pointer<T>::value,
                "T must be a pointer type");

  // Inherit from base class constructors
  using AddressSpaceBaseImpl<T, AS>::AddressSpaceBaseImpl;

};


/** Implementation of a fundamental type with an OpenCL address space

    \param T is the type of the basic object to be created

    \param AS is the address space to place the object into

    \todo Verify/improve to deal with const/volatile?
*/
template <typename T, address_space AS>
struct AddressSpaceFundamentalImpl : public AddressSpaceBaseImpl<T, AS> {

  // Inherit from base class constructors
  using AddressSpaceBaseImpl<T, AS>::AddressSpaceBaseImpl;

};


/** Implementation of an object type with an OpenCL address space

    \param T is the type of the basic object to be created

    \param AS is the address space to place the object into

    The class implementation is just inheriting of T so that all methods
    and non-member operators on T work also on AddressSpaceObjectImpl<T>

    \todo Verify/improve to deal with const/volatile?

    \todo what about T having some final methods?
*/
template <typename T, address_space AS>
struct AddressSpaceObjectImpl : public OpenCLType<T, AS>::type {
  /** Store the base type of the object

      \todo Add to the specification
  */
  using type = T;

  /** Store the base type of the object with OpenCL address space modifier

      \todo Add to the specification
  */
  using opencl_type = typename OpenCLType<T, AS>::type;


  /* C++11 helps a lot to be able to have the same constructors as the
     parent class here

     \todo Add this to the list of required C++11 features needed for SYCL
  */
  using type::type;

  /** Allow to creating an address space version of an object or to
      convert one */
  AddressSpaceObjectImpl(T && v) : T(v) { }

  /** Conversion operator to allow a AddressSpaceObjectImpl<T> to be used
      as a T so that all the methods of a T and the built-in operators for
      T can be used on a AddressSpaceObjectImpl<T> too */
  operator T & () { return this; }

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
