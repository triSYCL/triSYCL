/** \file This is a small class to track constructor/destructor invocations

    Define the TRISYCL_DEBUG CPP flag to have an output.

    To use it in some class C, make C inherit from debug<C>.

    Ronan.Keryell at AMD dot com
 */

#ifdef TRISYCL_DEBUG
#include <iostream>
#include <typeinfo>
#endif

/** \addtogroup debug_trace Debugging and tracing support
    @{
*/

/** Class used to trace the construction and destruction of classes that
    inherit from it

    \param T is the real type name to be used in the debug output.
 */
template <typename T>
struct debug {
#ifdef TRISYCL_DEBUG
  /// Trace the construction with the compiler-dependent mangled named
  debug() {
    std::cerr << " TRISYCL_DEBUG: Constructor of " << typeid(*this).name()
              << " " << (void*) this << std::endl;
  }

  /// Trace the construction with the compiler-dependent mangled named
  ~debug() {
    std::cerr << " TRISYCL_DEBUG: ~ Destructor of " << typeid(*this).name()
              << " " << (void*) this << std::endl;
  }
#endif
};

/// @} End the debug_trace Doxygen group
