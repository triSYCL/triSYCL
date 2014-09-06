/** \file This is a small class to track constructor/destructor invocations

    Define the TRISYCL_DEBUG CPP flag to have an output.

    To use it in some class C, make C inherit from debug<C>.

    Ronan.Keryell at AMD dot com

    Copyright 2014 Advanced Micro Devices, Inc.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifdef TRISYCL_DEBUG
#include <iostream>
#include <typeinfo>
#endif

namespace cl {
namespace sycl {
namespace trisycl {

/** \addtogroup debug_trace Debugging and tracing support
    @{
*/

/** Class used to trace the construction and destruction of classes that
    inherit from it

    \param T is the real type name to be used in the debug output.
 */
template <typename T>
struct Debug {
#ifdef TRISYCL_DEBUG
  /// Trace the construction with the compiler-dependent mangled named
  Debug() {
    std::cerr << " TRISYCL_DEBUG: Constructor of " << typeid(*this).name()
              << " " << (void*) this << std::endl;
  }

  /// Trace the construction with the compiler-dependent mangled named
  ~Debug() {
    std::cerr << " TRISYCL_DEBUG: ~ Destructor of " << typeid(*this).name()
              << " " << (void*) this << std::endl;
  }
#endif
};


/** Class used to display a vector-like type of classes that inherit from
    it

    \param T is the real type name to be used in the debug output.

    Calling the display() method dump the values on std::cout
 */
template <typename T>
struct DisplayVector {
  /// To debug and test
  void display() {
    std::cout << typeid(T).name() << ":";
    // Get a pointer to the real object
    for (auto e : *static_cast<T *>(this))
      std::cout << " " << e;
    std::cout << std::endl;
  }
};

/// @} End the debug_trace Doxygen group

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
