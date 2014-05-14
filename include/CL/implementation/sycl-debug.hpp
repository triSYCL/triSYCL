/** \file This is a small class to track constructor/destructor invocations

    Define the TRISYCL_DEBUG CPP flag to have an output.

    To use it in some class C, make C inherit from debug<C>.

    Ronan.Keryell at AMD dot com
 */

#ifdef TRISYCL_DEBUG
#include <iostream>
#include <typeinfo>
#endif

template <typename T>
struct debug {
#ifdef TRISYCL_DEBUG
  debug() {
    std::cerr << " TRISYCL_DEBUG: Constructor of " << typeid(*this).name()
              << " " << (void*) this << std::endl;
  }

  ~debug() {
    std::cerr << " TRISYCL_DEBUG: ~ Destructor of " << typeid(*this).name()
              << " " << (void*) this << std::endl;
  }
#endif
};
