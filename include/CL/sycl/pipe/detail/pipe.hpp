#ifndef TRISYCL_SYCL_PIPE_DETAIL_PIPE_HPP
#define TRISYCL_SYCL_PIPE_DETAIL_PIPE_HPP

/** \file The OpenCL SYCL pipe<> details

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <mutex>
#include <queue>

#include <boost/circular_buffer.hpp>

namespace cl {
namespace sycl {
namespace detail {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

template <typename T>
struct reserve_id {
  typename boost::circular_buffer<T>::iterator start;
  std::size_t size;
  bool ready = false;
};

/** Implement a pipe object
*/
template <typename T>
struct pipe {
  using value_type = T;

  /// Implement the pipe with a circular buffer
  using implementation_t = boost::circular_buffer<value_type>;

  /// The circular buffer to store the elements
  boost::circular_buffer<value_type> cb;

  /// To protect the access to the circular buffer
  std::mutex cb_mutex;

  /** The amount of elements blocked by reservations, not yet
      committed.

      This include some normal read/write to pipes between
      un-committed reservations */
  std::size_t reserved;

  std::queue<reserve_id<value_type>> qrid;
  using rid_iterator =
    typename std::queue<reserve_id<value_type>>::container_type::iterator;

  /// Create a pipe as a circular buffer of the required capacity
  pipe(std::size_t capacity) : cb { capacity } { }


  /** Return the maximum number of elements that can fit in the pipe
   */
  std::size_t capacity() const {
    return cb.capacity();
  }


  /** Get the current number of elements in the pipe

      This is obviously a volatile value which is constrained by the
      theory of restricted relativity.

      Note that on some devices it may be costly to implement (for
      example on FPGA).
   */
  std::size_t size() const {
    /* The actual number of available elements depends from the
       elements blocked by some reservation.
       This prevents a consumer to read into reserved area. */
    return cb.size() - reserved;
  }


  /** Test if the pipe is empty

      This is obviously a volatile value which is constrained by
      restricted relativity.

      Note that on some devices it may be costly to implement on the
      write side (for example on FPGA).
   */
  bool empty() const {
    // It is empty when the size is zero, taking into account reservations
    return size() ==  0;
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


  bool reserve(std::size_t s,
               rid_iterator &rid)  {
    std::lock_guard<std::mutex> lg { cb_mutex };
    if (size() >= s) {
      for (std::size_t i = 0; i != s; ++i)
        cb.push_back();
      /* Compute the location of the first element a posteriori since
         it does not exist if cb was empty before */
      auto first = cb.back() - (s - 1);
      qrid.emplace_back(first, s);
      rid = qrid.back();
      return true;
    }
    else
      return false;
  }


  void move_reservation_forward() {
    std::lock_guard<std::mutex> lg { cb_mutex };
    if (qrid.empty())
      return;
    auto rid = *qrid.front();
    if (!rid.ready)
      return;
    qrid.pop_front();
    if (qrid.empty())
      reserved -= cb.back() - rid.start + 1;
    else {
      reserved -= qrid.front().start - rid.start + 1;
      this->move_reserve_id_forward();
    }
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
