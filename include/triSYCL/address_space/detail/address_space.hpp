#ifndef TRISYCL_SYCL_ADDRESS_SPACES_DETAIL_ADDRESS_SPACES_HPP
#define TRISYCL_SYCL_ADDRESS_SPACES_DETAIL_ADDRESS_SPACES_HPP

/** \file

    Implement OpenCL address spaces in SYCL with C++-style.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


/* Use macros for address spaces to be able to retarget more easily to
   different runtimes & compilers.

   For example can have OpenCL address spaces even if the address
   space parsing is off in Clang/LLVM. Look at SPIRAddrSpaceMap in
   tools/clang/lib/Basic/Targets.cpp:
   \code
     static const unsigned SPIRAddrSpaceMap[] = {
         1, // opencl_global
         3, // opencl_local
         2, // opencl_constant
         4, // opencl_generic
         0, // cuda_device
         0, // cuda_constant
         0  // cuda_shared
     };
   \endcode
*/

/// Address space for OpenCL __global
#define TRISYCL_GLOBAL_AS TRISYCL_DEVICE_ONLY(__attribute__((address_space(1))))

/// Address space for OpenCL __local
#define TRISYCL_LOCAL_AS TRISYCL_DEVICE_ONLY(__attribute__((address_space(3))))

/// Address space for OpenCL __constant
#define TRISYCL_CONSTANT_AS                               \
  TRISYCL_DEVICE_ONLY(__attribute__((address_space(2))))

/// Address space for OpenCL __generic
#define TRISYCL_GENERIC_AS                                \
  TRISYCL_DEVICE_ONLY(__attribute__((address_space(4))))

/// Address space for OpenCL __private
#define TRISYCL_PRIVATE_AS


namespace trisycl::detail {

/** \addtogroup address_spaces
    @{
*/

/** Generate a type with some real OpenCL 2 attribute if we are on an
    OpenCL device

    In the general case, do not add any OpenCL address space qualifier */
template <typename T, access::address_space AS>
struct ocl_type { // NOTE: renamed from opencl_type because of MSVC bug
  using type = T;
};

/// Add an attribute for __constant address space
template <typename T>
struct ocl_type<T, access::address_space::constant_space> {
  using type = TRISYCL_CONSTANT_AS T;
};

#if TRISYCL_CL_LANGUAGE_VERSION >= 200
/// Add an attribute for __generic address space
template <typename T>
struct ocl_type<T, generic_space> {
  using type = TRISYCL_GENERIC_AS T;
};
#endif

/// Add an attribute for __global address space
template <typename T>
struct ocl_type<T, access::address_space::global_space> {
  using type = TRISYCL_GLOBAL_AS T;
};

/// Add an attribute for __local address space
template <typename T>
struct ocl_type<T, access::address_space::local_space> {
  using type = TRISYCL_LOCAL_AS T;
};

/// Add an attribute for __private address space
template <typename T>
struct ocl_type<T, access::address_space::private_space> {
  using type = TRISYCL_PRIVATE_AS T;
};


/* Forward declare some classes to allow some recursion in conversion
   operators */
template <typename SomeType, access::address_space SomeAS>
struct address_space_array;

template <typename SomeType, access::address_space SomeAS>
struct address_space_fundamental;

template <typename SomeType, access::address_space SomeAS>
struct address_space_object;

template <typename SomeType, access::address_space SomeAS>
struct address_space_ptr;

/** Dispatch the address space implementation according to the requested type

    \param T is the type of the object to be created

    \param AS is the address space to place the object into or to point to
    in the case of a pointer type
*/
template <typename T, access::address_space AS>
using addr_space =
  typename std::conditional<std::is_pointer<T>::value,
                            address_space_ptr<T, AS>,
  typename std::conditional<std::is_class<T>::value,
                            address_space_object<T, AS>,
  typename std::conditional<std::is_array<T>::value,
                            address_space_array<T, AS>,
                            address_space_fundamental<T, AS>
  >::type>::type>::type;


/** Implementation of the base infrastructure to wrap something in an
    OpenCL address space

    \param T is the type of the basic stuff to be created

    \param AS is the address space to place the object into

    \todo Verify/improve to deal with const/volatile?
*/
template <typename T, access::address_space AS>
struct address_space_base {
  /** Store the base type of the object

      \todo Add to the specification
  */
  using type = T;

  /** Store the base type of the object with OpenCL address space modifier

      \todo Add to the specification
  */
  using opencl_type = typename ocl_type<T, AS>::type;

  /** Set the address_space identifier that can be queried to know the
      pointer type */
  static auto constexpr address_space = AS;

};


/** Implementation of a variable with an OpenCL address space

    \param T is the type of the basic object to be created

    \param AS is the address space to place the object into
*/
template <typename T, access::address_space AS>
struct address_space_variable : public address_space_base<T, AS> {
  /** Store the base type of the object with OpenCL address space modifier

      \todo Add to the specification
  */
  using opencl_type = typename ocl_type<T, AS>::type;

  /// Keep track of the base class as a short-cut
  using super = address_space_base<T, AS>;

protected:

  /* C++11 helps a lot to be able to have the same constructors as the
     parent class here

     \todo Add this to the list of required C++11 features needed for SYCL
  */
  opencl_type variable;

public:

  /** Allow to create an address space version of an object or to convert
      one to be used by the classes inheriting by this one because it is
      not possible to directly initialize a base class member in C++ */
  address_space_variable(const T & v) : variable(v) { }


  /// Put back the default constructors canceled by the previous definition
  address_space_variable() = default;


  /** Conversion operator to allow a address_space_object<T> to be used
      as a T so that all the methods of a T and the built-in operators for
      T can be used on a address_space_object<T> too.

      Use opencl_type so that if we take the address of it, the address
      space is kept.
  */
  operator opencl_type & () { return variable; }

  /// Return the address of the value to implement pointers
  opencl_type * get_address() { return &variable; }

};


/** Implementation of a fundamental type with an OpenCL address space

    \param T is the type of the basic object to be created

    \param AS is the address space to place the object into

    \todo Verify/improve to deal with const/volatile?
*/
template <typename T, access::address_space AS>
struct address_space_fundamental : public address_space_variable<T, AS> {
  /// Keep track of the base class as a short-cut
  using super = address_space_variable<T, AS>;

  /// Inherit from base class constructors
  using super::address_space_variable;


  /** Also request for the default constructors that have been disabled by
      the declaration of another constructor

      This ensures for example that we can write
      \code
        generic<float *> q;
      \endcode
      without initialization.
  */
  address_space_fundamental() = default;


  /** Allow for example assignment of a global<float> to a priv<double>
      for example

     Since it needs 2 implicit conversions, it does not work with the
     conversion operators already define, so add 1 more explicit
     conversion here so that the remaining implicit conversion can be
     found by the compiler.

     Strangely
     \code
     template <typename SomeType, address_space SomeAS>
     address_space_base(addr_space<SomeType, SomeAS>& v)
     : variable(SomeType(v)) { }
     \endcode
     cannot be used here because SomeType cannot be inferred. So use
     address_space_base<> instead

     Need to think further about it...
  */
  template <typename SomeType, trisycl::access::address_space SomeAS>
  address_space_fundamental(address_space_fundamental<SomeType, SomeAS>& v)
  {
    /* Strangely I cannot have it working in the initializer instead, for
       some cases */
    super::variable = SomeType(v);
  }

};


/** Implementation for an OpenCL address space pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.

    All the address space pointers inherit from it, which makes trivial
    the implementation of \c trisycl::multi_ptr<T, AS>
*/
template <typename T, access::address_space AS>
struct address_space_ptr : public address_space_fundamental<T, AS> {
  // Verify that \a T is really a pointer
  static_assert(std::is_pointer<T>::value,
                "T must be a pointer type");

  /// Keep track of the base class as a short-cut
  using super = address_space_fundamental<T, AS>;

  /// Inherit from base class constructors
  using super::address_space_fundamental;

  using pointer_t = typename super::address_space_fundamental::type;
  using reference_t = typename std::remove_pointer_t<pointer_t>&;

  /** Allow initialization of a pointer type from the address of an
      element with the same type and address space
  */
  address_space_ptr(address_space_fundamental<typename std::pointer_traits<T>::element_type, AS> *p)
    : address_space_fundamental<T, AS> { p->get_address() } {}

  /// Put back the default constructors canceled by the previous definition
  address_space_ptr() = default;
};


/** Implementation of an array variable with an OpenCL address space

    \param T is the type of the basic object to be created

    \param AS is the address space to place the object into
*/
template <typename T, access::address_space AS>
struct address_space_array : public address_space_variable<T, AS> {
  /// Keep track of the base class as a short-cut
  using super = address_space_variable<T, AS>;

  /// Inherit from base class constructors
  using super::address_space_variable;


  /** Allow to create an address space array from an array
   */
  address_space_array(const T &array) {
    std::copy(std::begin(array), std::end(array), std::begin(super::variable));
  };


  /** Allow to create an address space array from an initializer list

      \todo Extend to more than 1 dimension
  */
  address_space_array(std::initializer_list<std::remove_extent_t<T>> list) {
    std::copy(std::begin(list), std::end(list), std::begin(super::variable));
  };

};


/** Implementation of an object type with an OpenCL address space

    \param T is the type of the basic object to be created

    \param AS is the address space to place the object into

    The class implementation is just inheriting of T so that all methods
    and non-member operators on T work also on address_space_object<T>

    \todo Verify/improve to deal with const/volatile?

    \todo what about T having some final methods?
*/
template <typename T, access::address_space AS>
//struct address_space_object : public opencl_type<T, AS>::type,
struct address_space_object : public ocl_type<T, AS>::type,
                              public address_space_base<T, AS> {
  /** Store the base type of the object with OpenCL address space modifier

      \todo Add to the specification
  */
  using opencl_type = typename ocl_type<T, AS>::type;

  /* C++11 helps a lot to be able to have the same constructors as the
     parent class here but with an OpenCL address space

     \todo Add this to the list of required C++11 features needed for SYCL
  */
  using opencl_type::opencl_type;

  /** Allow to create an address space version of an object or to
      convert one */
  address_space_object(T && v) : opencl_type(v) { }

  /** Conversion operator to allow a address_space_object<T> to be used
      as a T so that all the methods of a T and the built-in operators for
      T can be used on a address_space_object<T> too.

      Use opencl_type so that if we take the address of it, the address
      space is kept. */
  operator opencl_type & () { return *this; }

};

/// @} End the address_spaces Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ADDRESS_SPACES_DETAIL_ADDRESS_SPACES_HPP
