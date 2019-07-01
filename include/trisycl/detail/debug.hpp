#ifndef TRISYCL_SYCL_DETAIL_DEBUG_HPP
#define TRISYCL_SYCL_DETAIL_DEBUG_HPP

/** \file Track constructor/destructor invocations and trace kernel execution

    Define the TRISYCL_DEBUG CPP flag to have an output.

    To use it in some class C, make C inherit from debug<C>.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <iostream>

// The common debug and trace infrastructure
#if defined(TRISYCL_DEBUG) || defined(TRISYCL_TRACE_KERNEL)
#include <sstream>
#include <string>
#include <thread>

#include <boost/log/trivial.hpp>
#include <boost/type_index.hpp>

// To be able to construct string literals like "blah"s
using namespace std::string_literals;

/** Dump a debug message in a formatted way.

    Use an intermediate ostringstream because there are issues with
    BOOST_LOG_TRIVIAL to display C strings
*/
#define TRISYCL_INTERNAL_DUMP(expression) do {       \
    std::ostringstream s;                            \
    s << expression;                                 \
    BOOST_LOG_TRIVIAL(debug) << s.str();             \
  } while(0)
#endif

#ifdef TRISYCL_DEBUG
#define TRISYCL_DUMP(expression) TRISYCL_INTERNAL_DUMP(expression)

/// Same as TRISYCL_DUMP() but with thread id first
#define TRISYCL_DUMP_T(expression)                                      \
  TRISYCL_DUMP("Thread " << std::hex                                    \
               << std::this_thread::get_id() << ": " << expression)
#else
#define TRISYCL_DUMP(expression) do { } while(0)
#define TRISYCL_DUMP_T(expression) do { } while(0)
#endif

namespace trisycl::detail {

/** \addtogroup debug_trace Debugging and tracing support
    @{
*/

/** Class used to trace the construction, copy-construction,
    move-construction and destruction of classes that inherit from it

    \param T is the real type name to be used in the debug output.
*/
template <typename T>
struct debug {
  // To trace the execution of the conSTRUCTORs and deSTRUCTORs
#ifdef TRISYCL_DEBUG_STRUCTORS
  /// Trace the construction with the compiler-dependent mangled named
  debug() {
    TRISYCL_DUMP("Constructor of "
                 << boost::typeindex::type_id<T>().pretty_name()
                 << " " << (void*) this);
  }


  /** Trace the copy construction with the compiler-dependent mangled
      named

      Only add this constructor if T has itself the same constructor,
      otherwise it may prevent the synthesis of default copy
      constructor and assignment.
  */
  template <typename U = T>
  debug(debug const &,
        /* Use intermediate U type to have the type dependent for
           enable_if to work

        \todo Use is_copy_constructible_v when moving to C++17 */
        std::enable_if_t<std::is_copy_constructible<U>::value> * = 0) {
    TRISYCL_DUMP("Copy of " << boost::typeindex::type_id<T>().pretty_name()
                 << " " << (void*) this);
  }


  /** Trace the move construction with the compiler-dependent mangled
      named

      Only add this constructor if T has itself the same constructor,
      otherwise it may prevent the synthesis of default move
      constructor and move assignment.
  */
  template <typename U = T>
  debug(debug &&,
        /* Use intermediate U type to have the type dependent for
           enable_if to work

        \todo Use is_move_constructible_v when moving to C++17 */
        std::enable_if_t<std::is_move_constructible<U>::value> * = 0) {
    TRISYCL_DUMP("Move of " << boost::typeindex::type_id<T>().pretty_name()
                 << " " << (void*) this);
  }


  /// Trace the destruction with the compiler-dependent mangled named
  ~debug() {
    TRISYCL_DUMP("~ Destructor of "
                 << boost::typeindex::type_id<T>().pretty_name()
                 << " " << (void*) this);
  }
#endif
};


/** Wrap a kernel functor in some tracing messages to have start/stop
    information when TRISYCL_TRACE_KERNEL macro is defined */
template <typename KernelName, typename Functor>
auto trace_kernel(Functor f) {
#ifdef TRISYCL_TRACE_KERNEL
  // Inject tracing message around the kernel
  return [=] () mutable {
    /* Since the class KernelName may just be declared and not really
       defined, just use it through a class pointer to have
       typeid().name() not complaining */
    TRISYCL_INTERNAL_DUMP(
      "Kernel started "
      << boost::typeindex::type_id<KernelName *>().pretty_name());
    f();
    TRISYCL_INTERNAL_DUMP(
      "Kernel stopped "
      << boost::typeindex::type_id<KernelName *>().pretty_name());
  };
#else
  // Identity by default
  return f;
#endif
}


/** Class used to display a vector-like type of classes that inherit from
    it

    \param T is the real type name to be used in the debug output.

    Calling the display() method dump the values on std::cout
*/
template <typename T>
struct display_vector {

  /// To debug and test
  void display() const {
#ifdef TRISYCL_DEBUG
    std::cout << boost::typeindex::type_id<T>().pretty_name() << ":";
#endif
    // Get a pointer to the real object
    for (auto e : *static_cast<const T *>(this))
      std::cout << " " << e;
    std::cout << std::endl;
  }

};

/// @} End the debug_trace Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_DEBUG_HPP
