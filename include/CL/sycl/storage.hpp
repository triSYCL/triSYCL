#ifndef TRISYCL_SYCL_STORAGE_HPP
#define TRISYCL_SYCL_STORAGE_HPP

/** \file The OpenCL SYCL storage<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

namespace cl {
namespace sycl {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** Abstract the way storage is managed to allow the programmer to control
    the storage management of buffers

    \param T
    the type of the elements of the underlying data

    The user is responsible for ensuring that their storage class
    implementation is thread-safe.
*/
template <typename T>
struct storage {
  /// \todo Extension to SYCL specification: provide pieces of STL
  /// container interface?
  using element = T;
  using value_type = T;


  /** Method called by SYCL system to get the number of elements of type T
      of the underlying data

      \todo This is inconsistent in the specification with get_size() in
      buffer which returns the byte size. Is it to be renamed to
      get_count()?
  */
  virtual std::size_t get_size() = 0;


  /** Method called by the SYCL system to know where that data is held in
      host memory

      \return the address or nullptr if SYCL has to manage the temporary
      storage of the data.
  */
  virtual T* get_host_data() = 0;


  /** Method called by the SYCL system at the point of construction to
      request the initial contents of the buffer

      \return the address of the data to use or nullptr to skip this data
      initialization
  */
  virtual const T* get_initial_data() = 0;


  /** Method called at the point of construction to request where the
      content of the buffer should be finally stored to

      \return the address of where the buffer will be written to in host
      memory.

      If the address is nullptr, then this phase is skipped.

      If get_host_data() returns the same pointer as get_initial_data()
      and/or get_final_data() then the SYCL system should determine whether
      copying is actually necessary or not.
  */
  virtual T* get_final_data() = 0;


  /** Method called when the associated memory object is destroyed.

      This method is only called once, so if a memory object is copied
      multiple times, only when the last copy of the memory object is
      destroyed is the destroy method called.

      Exceptions thrown by the destroy method will be caught and ignored.
  */
  virtual void destroy() = 0;


  /** \brief Method called when a command_group which accesses the data is
      added to a queue

     After completed is called, there may be further calls of
      in_use() if new work is enqueued that operates on the memory object.
  */
  virtual void in_use() = 0;


  /// Method called when the final enqueued command has completed
  virtual void completed() = 0;
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

#endif // TRISYCL_SYCL_ACCESSOR_HPP
