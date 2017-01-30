#ifndef TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP

/** \file The OpenCL SYCL buffer<> detail implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include <boost/multi_array.hpp>

#include "CL/sycl/access.hpp"
#include "CL/sycl/buffer/detail/accessor.hpp"
#include "CL/sycl/buffer/detail/buffer_base.hpp"
#include "CL/sycl/buffer/detail/buffer_waiter.hpp"
#include "CL/sycl/range.hpp"

namespace cl {
namespace sycl {
namespace detail {

template<typename Iterator>
struct iterator_value_type
{
  using value_type  = typename std::remove_pointer_t<typename std::iterator_traits<Iterator>::pointer>;
  constexpr static bool is_const    = std::is_const<value_type>::value;
};




/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** A SYCL buffer is a multidimensional variable length array (à la C99
    VLA or even Fortran before) that is used to store data to work on.

    In the case we initialize it from a pointer, for now we just wrap the
    data with boost::multi_array_ref to provide the VLA semantics without
    any storage.
*/
template <typename T,
          std::size_t Dimensions = 1>
class buffer : public detail::buffer_base,
               public detail::debug<buffer<T, Dimensions>> {
public:

  // Extension to SYCL: provide pieces of STL container interface
  using element = T;
  using value_type = T;

private:

  /** If some allocation is requested, it is managed by this multi_array
      to ease initialization from data */
  boost::multi_array<T, Dimensions> allocation;

  // \todo Replace U and D somehow by T and Dimensions
  // To allow allocation access
  template <typename U,
            std::size_t D,
            access::mode Mode,
            access::target Target /* = access::global_buffer */>
    friend class detail::accessor;


  /** This is the multi-dimensional interface to the data that may point
      to either allocation in the case of storage managed by SYCL itself
      or to some other memory location in the case of host memory or
      storage<> abstraction use
  */
  boost::multi_array_ref<T, Dimensions> access;

  // How to copy back data on buffer destruction, can be modified with set_final_data( ... )
  boost::optional<std::function<void(void)>> final_write_back = boost::none;

  // Track if the buffer memory is provided as host memory
  bool data_host = false;
  // Track if data should be copied if a modification occurs
  bool copy_if_modified = false;
  // Track if data have been modified
  bool modified  = false;
  // Track if data are modifiable
  bool const_buffer = false;

public:

  /// Create a new read-write buffer of size \param r
  buffer(const range<Dimensions> &r) : buffer_base { false },
                                       allocation { r },
                                       access { allocation }
                                       {}


  /** Create a new read-write buffer from \param host_data of size
      \param r without further allocation */
  buffer(T *host_data, const range<Dimensions> &r) :
    buffer_base { false },
    allocation { r },
    access { allocation }
  {
    allocation.assign(host_data, host_data + access.num_elements());

    final_write_back = [=] {
      std::copy_n(access.data(), access.num_elements(), host_data);
    };
  }


  /** Create a new read-only buffer from \param host_data of size \param r
      without further allocation

      \todo Clarify the semantics in the spec. What happens if the
      host change the host_data after buffer creation?
  */
  buffer(const T *host_data, const range<Dimensions> &r) :
    /* \todo Need to solve this const buffer issue in a clean way

       Just allocate memory? */
    buffer_base { false },
    allocation { r },
    access { allocation }
    //data_host { true },
    //copy_if_modified { true }
    {
      allocation.assign(host_data, host_data + access.num_elements());
    }


  /** Create a new buffer with associated memory, using the data in
      host_data

      The ownership of the host_data is shared between the runtime and the
      user. In order to enable both the user application and the SYCL
      runtime to use the same pointer, a cl::sycl::mutex_class is
      used.
  */
  buffer(shared_ptr_class<T> &host_data,
         const range<Dimensions> &r)
    : buffer_base { false },
    allocation { r },
    access { allocation }
    {
      allocation.assign(host_data.get(), host_data.get() + access.num_elements());
      final_write_back = [=] {
        std::copy_n(access.data(), access.num_elements(), host_data.get());
      };
    }

private:
  template<typename Iterator, bool Mode>
  typename std::enable_if_t<Mode, void> constructor_for_iterator(Iterator begin){
    final_write_back = boost::none;
  }

  template<typename Iterator, bool Mode>
  typename std::enable_if_t<!Mode, void> constructor_for_iterator(Iterator begin){
    final_write_back = [=] {
      std::copy_n(access.data(), access.num_elements(), begin);
    };
  }

public:

  /// Create a new allocated 1D buffer from the given elements
  template <typename Iterator>
  buffer(Iterator start_iterator, Iterator end_iterator) :
    buffer_base { false },
    // The size of a multi_array is set at creation time
    allocation { boost::extents[std::distance(start_iterator, end_iterator)] },
    access { allocation }
    // if iterators are const ones, then we do not write back
    {
      /* Then assign allocation since this is the only multi_array
         method with this iterator interface */
      allocation.assign(start_iterator, end_iterator);
      constructor_for_iterator<Iterator, iterator_value_type<Iterator>::is_const>(start_iterator);
    }

  

  /** Create a new sub-buffer without allocation to have separate
      accessors later

      \todo To implement and deal with reference counting
  buffer(buffer<T, Dimensions> b,
         index<Dimensions> base_index,
         range<Dimensions> sub_range)
  */

  /// \todo Allow CLHPP objects too?
  ///
  /*
  buffer(cl_mem mem_object,
         queue from_queue,
         event available_event)
  */

  /** The buffer content may be copied back on destruction to some
      final location */
  
  private:
    /* *
     *
     *  returns true if a write back should trigered, false otherwise
     *
     * */
    inline bool do_write_back() {
      return modified && !data_host && final_write_back;
    }

  public:

  ~buffer() {
    // triggers a write-back if do_write_back() is true
    if(do_write_back())
      (*final_write_back)();
  }

  void mark_as_written(){modified = true;}

  // Use BOOST_DISABLE_ASSERTS at some time to disable range checking

  template <access::mode Mode,
            access::target Target = access::target::host_buffer>
  void get_access() {
    if(Mode == access::mode::write
        || Mode == access::mode::read_write
        || Mode == access::mode::discard_write
        || Mode == access::mode::discard_read_write
        || Mode == access::mode::atomic)
    {
      assert(!const_buffer);
      modified = true;
      if(copy_if_modified){
        //TODO
        detail::unimplemented();
        assert(false);
      }
    }
  }

 /** Return a range object representing the size of the buffer in
      terms of number of elements in each dimension as passed to the
      constructor
  */
  auto get_range() const {
    /* Interpret the shape which is a pointer to the first element as an
       array of Dimensions elements so that the range<Dimensions>
       constructor is happy with this collection

       \todo Add also a constructor in range<> to accept a const
       std::size_t *?
    */
    return range<Dimensions> {
      *(const std::size_t (*)[Dimensions])(allocation.shape())
        };
  }


  /** Returns the total number of elements in the buffer

      Equal to get_range()[0] * ... * get_range()[dimensions-1].
  */
  auto get_count() const {
    return access.num_elements();
  }


  /** Returns the size of the buffer storage in bytes

      \todo rename to something else. In
      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0122r0.pdf
      it is named bytes() for example
  */
  auto get_size() const {
    return get_count()*sizeof(value_type);
  }

/*  void set_final_data(std::function<void(void)> f) {
    final_write_back.reset(f);
  }*/


  /** Set the weak pointer to copy back data on buffer deletion

      \todo Add a write kernel dependency on the buffer so the buffer
      destructor has to wait for the kernel execution if the buffer is
      also accessed through a write accessor
  */
  void set_final_data(std::weak_ptr<T> && final_data) {
    final_write_back = [=] {
      if (auto sptr = final_data.lock())
      {
        std::copy_n(access.data(), access.num_elements(), sptr.get());
      }
    };
  }

  void set_final_data(std::shared_ptr<T> && final_data) {
    final_write_back = [=] {
      std::copy_n(access.data(), access.num_elements(), final_data.get());
    };
  }

  void set_final_data(T* final_data) {
    final_write_back = [=] {
      std::copy_n(access.data(), access.num_elements(), final_data);
    };
  }

  void set_final_data(std::nullptr_t){
    final_write_back = boost::none;
  }

  template <typename Iterator>
  void set_final_data(Iterator final_data) {
    static_assert(std::is_same<typename iterator_value_type<Iterator>::value_type , T>::value, "buffer type mismatch");
    static_assert(!(iterator_value_type<Iterator>::is_const), "const iterator are not allowed");
    final_write_back = [=] {
      std::copy_n(access.data(), access.num_elements(), final_data);
    };
  }

private:

  /** Get a \c future to wait from inside the \c cl::sycl::buffer in
      case there is something to copy back to the host

      \return A \c future in the \c optional if there is something to
      wait for, otherwise an empty \c optional
  */
  boost::optional<std::future<void>> get_destructor_future() {
    boost::optional<std::future<void>> f;
    /* If there is only 1 shared_ptr user of the buffer, this is the
       caller of this function, the \c buffer_waiter, so there is no
       need to get a \ future otherwise there will be a dead-lock if
       there is only 1 thread waiting for itself.

       Since \c use_count() is applied to a \c shared_ptr just created
       for this purpose, it actually increase locally the count by 1,
       so check for 1 + 1 use count instead...
    */
    if (shared_from_this().use_count() > 2)
    {
      // if the buffer's destruction triggers a write-back, wait
      if(do_write_back())
      {
        // Create a promise to wait for
        notify_buffer_destructor = std::promise<void> {};
        // And return the future to wait for it
        f = notify_buffer_destructor->get_future();
      }
    }
    return f;
  }


  // Allow buffer_waiter destructor to access get_destructor_future()
  // friend detail::buffer_waiter<T, Dimensions>::~buffer_waiter();
  /* \todo Work around to Clang bug
     https://llvm.org/bugs/show_bug.cgi?id=28873 cannot use destructor
     here */
  friend detail::buffer_waiter<T, Dimensions>;

};


/** Proxy function to avoid some circular type recursion

    \return a shared_ptr<task>

    \todo To remove with some refactoring
*/
template <typename BufferDetail>
static std::shared_ptr<detail::task>
buffer_add_to_task(BufferDetail buf,
                   handler *command_group_handler,
                   bool is_write_mode) {
    return buf->add_to_task(command_group_handler, is_write_mode);
  }

/// @} End the data Doxygen group

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

#endif // TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP
