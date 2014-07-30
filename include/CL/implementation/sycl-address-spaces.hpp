/** \file

    Ronan.Keryell at AMD point com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

//#include <boost/operators.hpp>

/** Enumerate the different OpenCL 2 address spaces */
enum address_space {
  generic_address_space,
};


/** Implementation for an OpenCL 2 generic pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
template <typename T>
class GenericImpl {
  // Verify that \a T is really a pointer
  static_assert(std::is_pointer<T>::value,
                "T in generic<T> must be a pointer type");

protected:

  /** Store the real pointer value

      Use a protected member so that the \c generic<> interface class can
      access the pointer field for assignment.

      On an OpenCL device, use really a \c __generic OpenCL 2 attribute
  */
#ifdef __SYCL_DEVICE_ONLY__
  __generic
#endif
  T pointer;


public:

  /** Synthesized default constructors

      This ensures that we can write
      \code{generic<float *> q;}
      without initialization.
  */
  GenericImpl() = default;

  /** Set the address_space identifier that can be queried to know the
      pointer type */
  static auto constexpr address_space = generic_address_space;

  /** Allow converting from a normal pointer

      Use explicit constructor so that we can forbid construction
      from some pointers from other address spaces.

      The negative side effect is that the assignment operator is not
      automatically replaced by the copy/move operator and must be
      explicitly defined in the interface
  */
  explicit GenericImpl(T && v) : pointer(v) { }

  /** Conversion operator to allow this \c generic<> pointer object to be
      used as a normal (\c __generic on OpenCL target) pointer */
  operator
#ifdef __SYCL_DEVICE_ONLY__
  __generic
#endif
 T &() { return pointer; }

};


#if 0
// Try a Boost-based implementation
template <typename T>
struct GenericImpl : boost::dereferenceable<T,
#ifdef __SYCL_DEVICE_ONLY__
                                            __generic
#endif
                                            T &,
                                            GenericImpl> {
  static_assert(std::is_pointer<T>::value,
                "T in generic<T> must be a pointer type");

private:
#ifdef __SYCL_DEVICE_ONLY__
  __generic
#endif
  T pointer;
};
#endif

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/
