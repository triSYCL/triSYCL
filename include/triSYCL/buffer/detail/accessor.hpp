#ifndef TRISYCL_SYCL_BUFFER_DETAIL_ACCESSOR_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_ACCESSOR_HPP

/** \file The OpenCL SYCL buffer accessor<> detail behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif
#include <boost/multi_array.hpp>

#include "triSYCL/access.hpp"
#include "triSYCL/accessor/detail/accessor_base.hpp"
#include "triSYCL/command_group/detail/task.hpp"
#include "triSYCL/detail/debug.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/item.hpp"
#include "triSYCL/nd_item.hpp"

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

    This implementation relies on boost::multi_array to provide this
    nice syntax and behaviour.

    Right now the aim of this class is just to access to the buffer in
    a read-write mode, even if capturing the multi_array_ref from a
    lambda make it const (since in examples we have lambda with [=]
    without mutable lambda).

    \todo Use the access::mode
*/
template <typename T,
          int Dimensions,
          access::mode Mode,
          access::target Target /* = access::global_buffer */>
class accessor :
    public detail::accessor_base,
    public std::enable_shared_from_this<accessor<T,
                                                 Dimensions,
                                                 Mode,
                                                 Target>>,
    public detail::debug<accessor<T,
                                  Dimensions,
                                  Mode,
                                  Target>> {
  /** Keep a reference to the accessed buffer

      Beware that it owns the buffer, which means that the accessor
      has to be destroyed to release the buffer and potentially
      unblock a kernel at the end of its execution
  */
  std::shared_ptr<detail::buffer<T, Dimensions>> buf;

  /// The implementation is a multi_array_ref wrapper
  using array_view_type = boost::multi_array_ref<T, Dimensions>;

  // The same type but writable
  using writable_array_view_type =
    typename std::remove_const<array_view_type>::type;

  /** The way the buffer is really accessed

      Use a mutable member because the accessor needs to be captured
      by value in the lambda which is then read-only. This is to avoid
      the user to use mutable lambda or have a lot of const_cast as
      previously done in this implementation
   */
  mutable array_view_type array;

public:

  /** \todo in the specification: store the dimension for user request

      \todo Use another name, such as from C++17 committee discussions.
   */
  static constexpr auto dimensionality = Dimensions;

  /** \todo in the specification: store the types for user request as STL
      or C++AMP */
  using value_type = T;
  using element = T;
  using reference = typename array_view_type::reference;
  using const_reference = typename array_view_type::const_reference;

  /** Inherit the iterator types from the implementation

      \todo Add iterators to accessors in the specification
  */
  using iterator = typename array_view_type::iterator;
  using const_iterator = typename array_view_type::const_iterator;
  using reverse_iterator = typename array_view_type::reverse_iterator;
  using const_reverse_iterator =
    typename array_view_type::const_reverse_iterator;


  /** Construct a host accessor from an existing buffer

      \todo fix the specification to rename target that shadows
      template parm
  */
  accessor(std::shared_ptr<detail::buffer<T, Dimensions>> target_buffer) :
    buf { target_buffer }, array { target_buffer->access } {
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
    buf->update_buffer_state(ctx, Mode, get_size(), array.data());
#endif
  }


  /** Construct a device accessor from an existing buffer

      \todo fix the specification to rename target that shadows
      template parm
  */
  accessor(std::shared_ptr<detail::buffer<T, Dimensions>> target_buffer,
           handler &command_group_handler) :
    buf { target_buffer }, array { target_buffer->access } {
    target_buffer->template track_access_mode<Mode>();
    TRISYCL_DUMP_T("Create a kernel accessor write = " << is_write_access());
    static_assert(Target == access::target::global_buffer
                  || Target == access::target::constant_buffer,
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
      task->add_prelude([=] {
          acc->copy_in_cl_buffer();
        });
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


  /** Return a range object representing the size of the buffer in
      terms of number of elements in each dimension as passed to the
      constructor

      \todo Move on
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564 and
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
  */
  auto get_range() const {
    /* Interpret the shape which is a pointer to the first element as an
       array of Dimensions elements so that the range<Dimensions>
       constructor is happy with this collection

       \todo Add also a constructor in range<> to accept a const
       std::size_t *?
    */
    return range<Dimensions> {
      *(const std::size_t (*)[Dimensions])(array.shape())
        };
  }


  /** Returns the total number of elements behind the accessor

      Equal to get_range()[0] * ... * get_range()[Dimensions-1].

      \todo Move on
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564 and
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
  */
  auto get_count() const {
    return array.num_elements();
  }


  /** Returns the size of the underlying buffer storage in bytes

      \todo Move on
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564 and
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
  */
  auto get_size() const {
    return get_count()*sizeof(value_type);
  }


  /** Use the accessor with integers à la [][][]

      Use array_view_type::reference instead of auto& because it does not
      work in some dimensions.
   */
  reference operator[](std::size_t index) {
    return array[index];
  }


  /** Use the accessor with integers à la [][][]

      Use array_view_type::reference instead of auto& because it does not
      work in some dimensions.
   */
  reference operator[](std::size_t index) const {
    return array[index];
  }


  /// To use the accessor with [id<>]
  auto &operator[](id<dimensionality> index) {
    return array(index);
  }


  /// To use the accessor with [id<>]
  auto &operator[](id<dimensionality> index) const {
    return array(index);
  }


  /// To use an accessor with [item<>]
  auto &operator[](item<dimensionality> index) {
    return (*this)[index.get()];
  }


  /// To use an accessor with [item<>]
  auto &operator[](item<dimensionality> index) const {
    return (*this)[index.get()];
  }


  /** To use an accessor with an [nd_item<>]

      \todo Add in the specification because used by HPC-GPU slide 22
  */
  auto &operator[](nd_item<dimensionality> index) {
    return (*this)[index.get_global()];
  }

  /** To use an accessor with an [nd_item<>]

      \todo Add in the specification because used by HPC-GPU slide 22
  */
  auto &operator[](nd_item<dimensionality> index) const {
    return (*this)[index.get_global()];
  }


  /** Get the first element of the accessor

      Useful with an accessor on a scalar for example.

      \todo Add in the specification
  */
  reference operator*() {
    return *array.data();
  }


  /** Get the first element of the accessor

      Useful with an accessor on a scalar for example.

      \todo Add in the specification?

      \todo Add the concept of 0-dim buffer and accessor for scalar
      and use an implicit conversion to value_type reference to access
      the value with the accessor?
  */
  reference operator*() const {
    return *array.data();
  }


  /// Get the buffer used to create the accessor
  detail::buffer<T, Dimensions> &get_buffer() {
    return *buf;
  }


  /** Test if the accessor has a read access right

      \todo Strangely, it is not really constexpr because it is not a
      static method...

      \todo to move in the access::mode enum class and add to the
      specification ?
  */
  constexpr bool is_read_access() const {
    return Mode == access::mode::read
      || Mode == access::mode::read_write
      || Mode == access::mode::discard_read_write;
  }


  /** Test if the accessor has a write access right

      \todo Strangely, it is not really constexpr because it is not a
      static method...

      \todo to move in the access::mode enum class and add to the
      specification ?
  */
  constexpr bool is_write_access() const {
    return Mode == access::mode::write
      || Mode == access::mode::read_write
      || Mode == access::mode::discard_write
      || Mode == access::mode::discard_read_write;
  }


  /** Return the pointer to the data

      \todo Implement the various pointer address spaces
  */
  auto
  get_pointer() const {
    return array.data();
  }


  /** Forward all the iterator functions to the implementation

      \todo Add these functions to the specification

      \todo The fact that the lambda capture make a const copy of the
      accessor is not yet elegantly managed... The issue is that
      begin()/end() dispatch is made according to the accessor
      constness and not from the array member constness...

      \todo try to solve it by using some enable_if on array
      constness?

      \todo The issue is that the end may not be known if it is
      implemented by a raw OpenCL cl_mem... So only provide on the
      device the iterators related to the start? Actually the accessor
      needs to know a part of the shape to have the multidimentional
      addressing. So this only require a size_t more...

      \todo Factor out these in a template helper

      \todo Do we need this in detail::accessor too or only in accessor?
  */


  // iterator begin() { return array.begin(); }
  iterator begin() const {
    return const_cast<writable_array_view_type &>(array).begin();
  }


  // iterator end() { return array.end(); }
  iterator end() const {
    return const_cast<writable_array_view_type &>(array).end();
  }


  // const_iterator begin() const { return array.begin(); }


  // const_iterator end() const { return array.end(); }


  const_iterator cbegin() const { return array.begin(); }


  const_iterator cend() const { return array.end(); }


  // reverse_iterator rbegin() { return array.rbegin(); }
  reverse_iterator rbegin() const {
    return const_cast<writable_array_view_type &>(array).rbegin();
  }


  // reverse_iterator rend() { return array.rend(); }
  reverse_iterator rend() const {
    return const_cast<writable_array_view_type &>(array).rend();
  }


  // const_reverse_iterator rbegin() const { return array.rbegin(); }


  // const_reverse_iterator rend() const { return array.rend(); }


  const_reverse_iterator crbegin() const { return array.rbegin(); }


  const_reverse_iterator crend() const { return array.rend(); }

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
    buf->update_buffer_state(ctx, Mode, get_size(), array.data());
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

#endif // TRISYCL_SYCL_BUFFER_DETAIL_ACCESSOR_HPP
