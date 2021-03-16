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

#include <boost/type_index.hpp>

// Only when the common debug or trace infrastructure is required
#if defined(TRISYCL_DEBUG) || defined(TRISYCL_TRACE_KERNEL)
#include <sstream>
#include <string>
#include <thread>

#include <boost/log/trivial.hpp>

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

    Also trace the copy and move assignments.

    \param T is the real type name to be used in the debug output.
*/
template <typename T>
struct debug {
  /// Get the pretty name of T itself
  static auto constexpr type_pretty_name() {
    return boost::typeindex::type_id<T>().pretty_name();
  }

#ifdef TRISYCL_DEBUG_STRUCTORS

  /// Trace the construction
  debug() {
    TRISYCL_DUMP("Constructor of " << type_pretty_name() << " "
                 << static_cast<void*>(this));
  }


  /// Trace the copy construction
  debug(debug const& old) {
    TRISYCL_DUMP("Copy of " << type_pretty_name() << " into "
                 << static_cast<void*>(this) << " from "
                 << static_cast<const void*>(&old));
  }


  /// Trace the copy assignment
  debug& operator=(const debug& rhs) {
    TRISYCL_DUMP("Copy assignment of " << type_pretty_name()
                 << " into " << static_cast<void*>(this)
                 << " from " << static_cast<const void*>(&rhs));
    return *this;
  }


  /// Trace the move construction
  debug(debug&& old) {
    TRISYCL_DUMP("Move of " << type_pretty_name() << " into "
                 << static_cast<void*>(this) << " from "
                 << static_cast<void*>(&old));
  }


  /// Trace the move assignment
  debug& operator=(debug&& rhs) {
    TRISYCL_DUMP("Move assignment of " << type_pretty_name()
                 << " into " << static_cast<void*>(this)
                 << " from " << static_cast<void*>(&rhs));
    return *this;
  }


  /// Trace the destruction
  ~debug() {
    TRISYCL_DUMP("~ Destructor of " << type_pretty_name() << " "
                 << static_cast<void*>(this));
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
    for (auto e : static_cast<const T&>(*this))
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
