#ifndef TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_ACCESSOR_SHEPHERD_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_ACCESSOR_SHEPHERD_HPP

/** \file The OpenCL SYCL buffer accessor shepherd detail behind the scene
    tracking the buffer usage for the accessor.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "CL/sycl/access.hpp"
#include "CL/sycl/accessor/detail/accessor_base.hpp"
#include "CL/sycl/buffer/detail/buffer_accessor_view.hpp"
#include "CL/sycl/command_group/detail/task.hpp"
#include "CL/sycl/detail/debug.hpp"

namespace cl::sycl {

class handler;

namespace detail {

// Forward declaration of detail::buffer for use in the following
template <typename T, int Dimensions> class buffer;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** Keep track of a buffer while used through an accessor.

    Splitting the accessor in a light-weight accessor and a heavy
    buffer accessor shepherd in the back-stage allows a simpler code
    transformation in the device compiler since the buffer accessor
    shepherd disappears.
*/
template <typename Accessor,
          // All the other parameters can be inferred from the Accessor
          typename T = typename Accessor::value_type,
          int Dimensions = Accessor::dimensionality,
          access::mode Mode = Accessor::mode,
          access::target Target = Accessor::target>
class buffer_accessor_shepherd :
    public detail::accessor_base,
    public std::enable_shared_from_this<buffer_accessor_shepherd<Accessor>>,
    public detail::debug<buffer_accessor_shepherd<Accessor>> {
  /** Keep a reference to the accessed buffer

      Beware that it owns the buffer, which means that this class
      has to be destroyed to release the buffer and potentially
      unblock a kernel at the end of its execution
  */
  std::shared_ptr<detail::buffer<T, Dimensions>> buf;

public:

  /// Construct a host accessor shepherd from an existing buffer
  buffer_accessor_shepherd(std::shared_ptr<detail::buffer<T, Dimensions>>
                           target_buffer)
    : buf { target_buffer }
  {
    target_buffer->template track_access_mode<Mode>();
    TRISYCL_DUMP_T("Create a host buffer_accessor_shepherd write = "
                   << Accessor::is_write_access());
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
    cl::sycl::context ctx;
    buf->call_update_buffer_state(ctx, Mode,  buf->get_size());
#endif
  }


  /// Construct a device accessor from an existing buffer
  buffer_accessor_shepherd(std::shared_ptr<detail::buffer<T, Dimensions>>
                           target_buffer,
                           handler &command_group_handler)
    : buf { target_buffer }
  {
    target_buffer->template track_access_mode<Mode>();
    TRISYCL_DUMP_T("Create a kernel buffer_accessor_shepherd write = "
                   << Accessor::is_write_access());
    static_assert(Target == access::target::global_buffer
                  || Target == access::target::constant_buffer,
                  "access target should be global_buffer or constant_buffer "
                  "when a handler is used");
    // Register the buffer to the task dependencies
    task = buffer_add_to_task(buf, &command_group_handler,
                              Accessor::is_write_access());
  }


  /** Register the accessor shepherd once a \c std::shared_ptr is
      created on it

      This is to be called from outside once the object is created. It
      has been tried directly inside the contructor, but calling \c
      shared_from_this() from the constructor dead-lock with
      libstdc++6

      \todo Double-check with the C++ committee on this issue.
  */
  void register_accessor() {
    if (!task->get_queue()->is_host()) {
      // To keep alive this accessor shepherd in the following lambdas
      auto shepherd = this->shared_from_this();
      // Attach the accessor to the task and get its order
      set_order(task->register_accessor(shepherd));
#ifdef TRISYCL_OPENCL
      /* Before running the kernel, make sure the cl_mem behind this
         accessor is up-to-date on the device if needed and pass it to
         the kernel */
      task->add_prelude([=] {
          shepherd->copy_in_cl_buffer();
        });
      // After running the kernel, deal with some copy-back if needed
      task->add_postlude([=] {
          /* Even if this function does nothing, it is required to
             have the capture of acc to keep the accessor alive across
             the kernel execution up to the execution postlude.

             Note that this is what will destroy the accessor shepherd
             after the kernel execution */
          shepherd->copy_back_cl_buffer();
        });
#endif
    }
  }


  /// Get the buffer used to create the accessor
  detail::buffer<T, Dimensions> &get_buffer() {
    return *buf;
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
    buf->call_update_buffer_state(ctx, Mode, buf->get_size());
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

#endif // TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_ACCESSOR_SHEPHERD_HPP
