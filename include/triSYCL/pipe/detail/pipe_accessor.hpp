#ifndef TRISYCL_SYCL_PIPE_DETAIL_PIPE_ACCESSOR_HPP
#define TRISYCL_SYCL_PIPE_DETAIL_PIPE_ACCESSOR_HPP

/** \file The OpenCL SYCL pipe accessor<> detail behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>
#include <type_traits>

#include "triSYCL/access.hpp"
#include "triSYCL/detail/debug.hpp"
#include "triSYCL/pipe/detail/pipe.hpp"
#include "triSYCL/pipe_reservation/detail/pipe_reservation.hpp"

namespace trisycl {

class handler;

namespace detail {

// Forward declaration of detail::accessor to declare the specialization
template <typename T,
          int Dimensions,
          access::mode Mode,
          access::target Target>
class accessor;
/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The accessor abstracts the way pipe data are accessed inside a
    kernel
*/
template <typename T,
          access::mode AccessMode,
          access::target Target>
class pipe_accessor :
    public detail::debug<detail::pipe_accessor<T, AccessMode, Target>> {

public:

  static constexpr auto rank = 1;
  static constexpr auto mode = AccessMode;
  static constexpr auto target = Target;

  static constexpr bool blocking =
    (target == trisycl::access::target::blocking_pipe);

  /// The STL-like types
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;

private:

  /// The real pipe implementation behind the hood
  std::shared_ptr<detail::pipe<T>> implementation;

  /** Store the success status of last pipe operation

      It is not impacted by reservation success.

      It does exist even if the pipe accessor is not evaluated in a
      boolean context for, but a use-def analysis can optimise it out
      in that case and not use some storage

      Use a mutable state here so that it can work with a [=] lambda
      capture without having to declare the whole lambda as mutable
  */
  bool mutable ok = false;

public:

  /** Construct a pipe accessor from an existing pipe
   */
  pipe_accessor(const std::shared_ptr<detail::pipe<T>> &p)
    : implementation { p } {
    //    TRISYCL_DUMP_T("Create a kernel pipe accessor write = "
    //                 << is_write_access());
    // Verify that the pipe is not already used in the requested mode
    if (mode == access::mode::write)
      if (implementation->used_for_writing)
        /// \todo Use pipe_exception instead
        throw std::logic_error { "The pipe is already used for writing." };
      else
        implementation->used_for_writing = true;
    else
      if (implementation->used_for_reading)
        throw std::logic_error { "The pipe is already used for reading." };
      else
        implementation->used_for_reading = true;
  }


  /** Construct a pipe accessor from an existing pipe

      For now the handler is not used.
  */
  pipe_accessor(const std::shared_ptr<detail::pipe<T>> &p,
                handler &command_group_handler)
    : pipe_accessor(p) {}


  pipe_accessor() = default;


  /// Return the maximum number of elements that can fit in the pipe
  std::size_t capacity() const {
    return implementation->capacity();
  }

  /** Get the current number of elements in the pipe

      This is obviously a volatile value which is constrained by
      restricted relativity.

      Note that on some devices it may be costly to implement (for
      example on FPGA).
   */
  std::size_t size() const {
    return implementation->size_with_lock();
  }


  /** Test if the pipe is empty

      This is obviously a volatile value which is constrained by
      restricted relativity.

      Note that on some devices it may be costly to implement on the
      write side (for example on FPGA).
   */
  bool empty() const {
    return implementation->empty_with_lock();
  }


  /** Test if the pipe is full

      This is obviously a volatile value which is constrained by
      restricted relativity.

      Note that on some devices it may be costly to implement on the
      read side (for example on FPGA).
  */
  bool full() const {
    return implementation->full_with_lock();
  }


  /** In an explicit bool context, the accessor gives the success
      status of the last access

      It is not impacted by reservation success.

      The explicitness is related to avoid \code some_pipe <<
      some_value \endcode to be interpreted as \code some_bool <<
      some_value \endcode when the type of \code some_value \endcode
      is not the same type as the pipe type.

      \return true on success of the previous read or write operation
  */
  explicit operator bool() const {
    return ok;
  }


  /** Try to write a value to the pipe

      \param[in] value is what we want to write

      \return this so we can apply a sequence of write for example
      (but do not do this on a non blocking pipe...)

      \todo provide a && version

      This function is const so it can work when the accessor is
      passed by copy in the [=] kernel lambda, which is not mutable by
      default
  */
  const pipe_accessor &write(const value_type &value) const {
    static_assert(mode == access::mode::write,
                  "'.write(const value_type &value)' method on a pipe accessor"
                  " is only possible with write access mode");
    ok = implementation->write(value, blocking);
    // Return a reference to *this so we can apply a sequence of write
    return *this;
  }


  /** Some syntactic sugar to use \code a << v \endcode instead of
      \code a.write(v) \endcode */
  const pipe_accessor &operator<<(const value_type &value) const {
    static_assert(mode == access::mode::write,
                  "'<<' operator on a pipe accessor is only possible"
                  " with write access mode");
    // Return a reference to *this so we can apply a sequence of >>
    return write(value);
  }


  /** Try to read a value from the pipe

      \param[out] value is the reference to where to store what is
      read

      \return \code this \endcode so we can apply a sequence of read
      for example (but do not do this on a non blocking pipe...)

      This function is const so it can work when the accessor is
      passed by copy in the [=] kernel lambda, which is not mutable by
      default
  */
  const pipe_accessor &read(value_type &value) const {
    static_assert(mode == access::mode::read,
                  "'.read(value_type &value)' method on a pipe accessor"
                  " is only possible with read access mode");
    ok = implementation->read(value, blocking);
    // Return a reference to *this so we can apply a sequence of read
    return *this;
  }


  /** Read a value from a blocking pipe

      \return the read value directly, since it cannot fail on
      blocking pipe

      This function is const so it can work when the accessor is
      passed by copy in the [=] kernel lambda, which is not mutable by
      default
  */
  value_type read() const {
    static_assert(mode == access::mode::read,
                  "'.read()' method on a pipe accessor is only possible"
                  " with read access mode");
    static_assert(blocking,
                  "'.read()' method on a pipe accessor is only possible"
                  " with a blocking pipe");
    value_type value;
    implementation->read(value, blocking);
    return value;
  }


  /** Some syntactic sugar to use \code a >> v \endcode instead of
      \code a.read(v) \endcode */
  const pipe_accessor &operator>>(value_type &value) const {
    static_assert(mode == access::mode::read,
                  "'>>' operator on a pipe accessor is only possible"
                  " with read access mode");
    // Return a reference to *this so we can apply a sequence of >>
    return read(value);
  }


  detail::pipe_reservation<pipe_accessor> reserve(std::size_t size) const {
    return { *implementation, size };
  }


  /// Set debug mode
  void set_debug(bool enable) const {
    implementation->debug_mode = enable;
  }


  auto &get_pipe_detail() {
    return implementation;
  }


  ~pipe_accessor() {
    /// Free the pipe for a future usage for the current mode
    if (mode == access::mode::write)
      implementation->used_for_writing = false;
    else
      implementation->used_for_reading = false;
  }

};

/// @} End the data Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PIPE_DETAIL_PIPE_ACCESSOR_HPP
