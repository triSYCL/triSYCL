#ifndef TRISYCL_SYCL_PIPE_DETAIL_PIPE_HPP
#define TRISYCL_SYCL_PIPE_DETAIL_PIPE_HPP

/** \file The OpenCL SYCL pipe<> details

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <mutex>

#include <boost/circular_buffer.hpp>

namespace cl {
namespace sycl {
namespace detail {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** Implement a pipe object
*/
template <typename T>
struct pipe {
  // Implement the pipe with a circular buffer
  boost::circular_buffer<T> cb;
  /// To protect the access to the circular buffer
  std::mutex cb_mutex;

  /// Create a pipe as a circular buffer of the required capacity
  pipe(std::size_t capacity) : cb { capacity } { }



  /** Return the maximum number of elements that can fit in the pipe
   */
  std::size_t capacity() const {
    return cb.capacity();
  }


  /** Get the current number of elements in the pipe

      This is obviously a volatile value which is constrained by
      restricted relativity.

      Note that on some devices it may be costly to implement (for
      example on FPGA).
   */
  std::size_t size() const {
    return cb.size();
  }


  /** Test if the pipe is empty

      This is obviously a volatile value which is constrained by
      restricted relativity.

      Note that on some devices it may be costly to implement on the
      write side (for example on FPGA).
   */
  bool empty() const {
    return cb.empty();
  }


  /** Test if the pipe is full

      This is obviously a volatile value which is constrained by
      restricted relativity.

      Note that on some devices it may be costly to implement on the
      read side (for example on FPGA).
  */
  bool full() const {
    return cb.full();
  }


  /** Try to write a value to the pipe

      \param[in] value is what we want to write

      \return true on success

      \todo provide a && version
  */
  bool write(const T &value) {
    std::lock_guard<std::mutex> lg { cb_mutex };
    // TRISYCL_DUMP_T("Write pipe full = " << cb.full()
    //                << " value = " << value);
    if (cb.full())
      return false;
    cb.push_back(value);
    // TRISYCL_DUMP_T("Write pipe front = " << cb.front()
    //                << " back = " << cb.back());
    return true;
  }


  /** Try to read a value from the pipe

      \param[out] value is the reference to where to store what is
      read

      \return true on success
  */
  bool read(T &value) {
    std::lock_guard<std::mutex> lg { cb_mutex };
    // TRISYCL_DUMP_T("Read pipe empty = " << cb.empty());

    if (cb.empty())
      return false;
    // TRISYCL_DUMP_T("Read pipe front = " << cb.front()
    //                << " back = " << cb.back());
    value = cb.front();
    // TRISYCL_DUMP_T("Read pipe value = " << value);
    cb.pop_front();
    return true;
  }

};

/// @} End the execution Doxygen group

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

#endif // TRISYCL_SYCL_PIPE_DETAIL_PIPE_HPP
