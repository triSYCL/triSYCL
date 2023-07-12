#ifndef TRISYCL_SYCL_BUFFER_DETAIL_ACCESSOR_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_ACCESSOR_HPP

/** \file The SYCL buffer accessor<> detail behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "triSYCL/access.hpp"
#include "triSYCL/accessor/detail/accessor_base.hpp"
#include "triSYCL/accessor/facade/accessor.hpp"
#include "triSYCL/command_group/detail/task.hpp"
#include "triSYCL/detail/debug.hpp"

namespace trisycl {

class handler;

namespace detail {

// Forward declaration of detail::buffer for use in accessor
template <typename T, int Dimensions> class buffer;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The buffer accessor abstracts the way buffer data are accessed
    inside a kernel in a multidimensional variable length array way.

    This implementation relies on std::mdspan to provide this nice
    syntax and behavior.

    Right now the aim of this class is just to access to the buffer in
    a read-write mode, even if capturing the accessor from a lambda
    make it const (since in examples we have lambda with [=] without
    mutable lambda).

    \todo Use the access::mode
*/
template <typename T, int Dimensions, access::mode Mode,
          access::target Target /* = access::global_buffer */>
class accessor
    : public detail::accessor_base
    , public facade::accessor<mixin::accessor<T, Dimensions>>
    , public std::enable_shared_from_this<accessor<T, Dimensions, Mode, Target>>
    , public detail::debug<accessor<T, Dimensions, Mode, Target>> {
  /** Keep a reference to the accessed buffer

      Beware that it owns the buffer, which means that the accessor
      has to be destroyed to release the buffer and potentially
      unblock a kernel at the end of its execution
  */
  std::shared_ptr<detail::buffer<T, Dimensions>> buf;

  /// Where most of the user-facing interface dwells
  using facade = facade::accessor<mixin::accessor<T, Dimensions>>;

 public:
#ifdef TODO
  /** Inherit the iterator types from the implementation

      \todo Add iterators to accessors in the specification
  */
  using const_iterator = typename array_view_type::const_iterator;
  using reverse_iterator = typename array_view_type::reverse_iterator;
  using const_reverse_iterator =
      typename array_view_type::const_reverse_iterator;
#endif

  /// Used by the local accessor hack on top of host accessor
  accessor() = default;

  /** Construct a host accessor from an existing buffer

      \todo fix the specification to rename target that shadows
      template parm
  */
  accessor(std::shared_ptr<detail::buffer<T, Dimensions>> target_buffer)
      : facade { target_buffer->access }
      , buf { target_buffer } {
    target_buffer->template track_access_mode<Mode>();
    TRISYCL_DUMP_T("Create a host accessor write = " << is_write_access());
    static_assert(Target == access::target::host_buffer,
                  "without a handler, access target should be host_buffer");
    /* The host needs to wait for all the producers of the buffer to
       have finished */
    buf->wait();

#ifdef TRISYCL_OPENCL
    /* For the host context, we are obligated to update the buffer state
       during the accessors creation, otherwise we have no way of knowing
       if a buffer was modified on the host. This is only true because
       host accessors are blocking
     */
    trisycl::context ctx;
    buf->update_buffer_state(ctx, Mode, facade::get_size(), facade::data());
#endif
  }

  /** Construct a device accessor from an existing buffer

      \todo fix the specification to rename target that shadows
      template parm
  */
  accessor(std::shared_ptr<detail::buffer<T, Dimensions>> target_buffer,
           handler& command_group_handler)
      : facade { target_buffer->access }
      , buf { target_buffer } {
    target_buffer->template track_access_mode<Mode>();
    TRISYCL_DUMP_T("Create a kernel accessor write = " << is_write_access());
    static_assert(Target == access::target::global_buffer ||
                      Target == access::target::constant_buffer,
                  "access target should be global_buffer or constant_buffer "
                  "when a handler is used");
    // Register the buffer to the task dependencies
    task = buffer_add_to_task(buf, &command_group_handler, is_write_access());
  }

  /** Register the accessor once a \c std::shared_ptr is created on it

      This is to be called from outside once the object is created. It
      has been tried directly inside the contructor, but calling \c
      shared_from_this() from the constructor dead-lock with
      libstdc++6

      \todo Double-check with the C++ committee on this issue.
  */
  void register_accessor() {
    if (!task->get_queue()->is_host()) {
      // To keep alive this accessor in the following lambdas
      auto acc = this->shared_from_this();
      // Attach the accessor to the task and get its order
      set_order(task->register_accessor(acc));
#ifdef TRISYCL_OPENCL
      /* Before running the kernel, make sure the cl_mem behind this
         accessor is up-to-date on the device if needed and pass it to
         the kernel */
      task->add_prelude([=] { acc->copy_in_cl_buffer(); });
      // After running the kernel, deal with some copy-back if needed
      task->add_postlude([=] {
        /* Even if this function does nothing, it is required to
           have the capture of acc to keep the accessor alive across
           the kernel execution up to the execution postlude */
        acc->copy_back_cl_buffer();
      });
#endif
    }
  }

  /// Get the buffer used to create the accessor
  detail::buffer<T, Dimensions>& get_buffer() { return *buf; }

  /** Test if the accessor has a read access right

      \todo Strangely, it is not really constexpr because it is not a
      static method...

      \todo to move in the access::mode enum class and add to the
      specification ?
  */
  constexpr bool is_read_access() const {
    return Mode == access::mode::read || Mode == access::mode::read_write ||
           Mode == access::mode::discard_read_write;
  }

  /** Test if the accessor has a write access right

      \todo Strangely, it is not really constexpr because it is not a
      static method...

      \todo to move in the access::mode enum class and add to the
      specification ?
  */
  constexpr bool is_write_access() const {
    return Mode == access::mode::write || Mode == access::mode::read_write ||
           Mode == access::mode::discard_write ||
           Mode == access::mode::discard_read_write;
  }

 private:
#ifdef TRISYCL_OPENCL
  // The following function are used from handler
  friend handler;

  /// Get the boost::compute::buffer or throw if unset
  boost::compute::buffer get_cl_buffer() const override {
    // This throws if not set
    auto ctx = task->get_queue()->get_context();
    return buf->get_cl_buffer(ctx);
  }

  /** Lazily associate a CL buffer to the SYCL buffer and copy data in it
      if required, updates the state of the data in the buffer across contexts
  */
  void copy_in_cl_buffer() {
    /* Create the OpenCL buffer and copy in it the data from the host if
       the buffer doesn't already exists or if the data is not up to date
    */
    auto ctx = task->get_queue()->get_context();
    buf->update_buffer_state(ctx, Mode, facade::get_size(), facade::data());
  }

  /// Does nothing
  void copy_back_cl_buffer() {
    /* The copy back is handled by the host accessor and the buffer destructor.
       We don't need to systematically transfer the data after the
       kernel execution

       \todo Figure out what to do with this function
    */
  }
#endif

protected:
  /// Set later the current buffer associated to this accessor
  void set_buffer(std::shared_ptr<detail::buffer<T, Dimensions>> b) {
    buf = b;
  }
};

/// @} End the data Doxygen group

} // namespace detail
} // namespace trisycl

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_BUFFER_DETAIL_ACCESSOR_HPP
