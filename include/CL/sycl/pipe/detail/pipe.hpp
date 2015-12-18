#ifndef TRISYCL_SYCL_PIPE_DETAIL_PIPE_HPP
#define TRISYCL_SYCL_PIPE_DETAIL_PIPE_HPP

/** \file The OpenCL SYCL pipe<> details

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <mutex>
#include <deque>

/* The debug mode of boost/circular_buffer.hpp has a nasty side effect
   in multithread applications using several iterators at the same
   time even in read-only mode because the library tracks them for
   debugging purpose in a... non-thread safe way */
#define BOOST_CB_DISABLE_DEBUG
#include <boost/circular_buffer.hpp>

namespace cl {
namespace sycl {
namespace detail {

/** \addtogroup data Data access and storage in SYCL
    @{
*/


/// A private description of a reservation station
template <typename T>
struct reserve_id {
  /// Start of the reservation in the pipe storage
  typename boost::circular_buffer<T>::iterator start;

  /// Number of elements in the reservation
  std::size_t size;

  /* True when the reservation has been committed and is ready to be
     released */
  bool ready = false;

  /** Track a reservation not committed yet

      \param[in] start point to the start of the reservation in the
      pipe storage

      \param[in] size is the number of elements in the reservation
  */
  reserve_id(typename boost::circular_buffer<T>::iterator start,
             std::size_t size) : start { start }, size { size } {}

};


/** Implement a pipe object

    Use some mutable members so that the pipe object can be changed even
    when the accessors are captured in a lambda.
*/
template <typename T>
struct pipe : public detail::debug<pipe<T>> {
  using value_type = T;

  /// Implement the pipe with a circular buffer
  using implementation_t = boost::circular_buffer<value_type>;

  /// The circular buffer to store the elements
  boost::circular_buffer<value_type> cb;

  /** To protect the access to the circular buffer.

      In case the object is capture in a lambda per copy, make it
      mutable. */
  mutable std::mutex cb_mutex;

  std::deque<reserve_id<value_type>> qrid;
  using rid_iterator = typename decltype(qrid)::iterator;

  /// Create a pipe as a circular buffer of the required capacity
  pipe(std::size_t capacity) : cb { capacity } { }


  /** Return the maximum number of elements that can fit in the pipe
   */
  std::size_t capacity() const {
    // No lock required since it is fixed and set at construction time
    return cb.capacity();
  }


private:

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
    return cb.size() - reserved_for_writing();
  }


  /** Test if the pipe is empty

      This is obviously a volatile value which is constrained by
      restricted relativity.

      Note that on some devices it may be costly to implement on the
      write side (for example on FPGA).
   */
  bool empty() const {
    TRISYCL_DUMP_T("empty() cb.size() = " << cb.size()
                   << " size() = " << size());
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


public:

  /// The size() method used outside needs to lock the datastructure
  std::size_t size_with_lock() const {
    std::lock_guard<std::mutex> lg { cb_mutex };
    return size();
  }


  /// The empty() method used outside needs to lock the datastructure
  bool empty_with_lock() const {
    std::lock_guard<std::mutex> lg { cb_mutex };
    return empty();
  }


  // The full() method used outside needs to lock the datastructure
  bool full_with_lock() const {
    std::lock_guard<std::mutex> lg { cb_mutex };
    return full();
  }


  /** Try to write a value to the pipe

      \param[in] value is what we want to write

      \return true on success

      \todo provide a && version
  */
  bool write(const T &value) {
    std::lock_guard<std::mutex> lg { cb_mutex };
    // TRISYCL_DUMP_T("Write pipe full = " << full()
    //                << " value = " << value);
    if (full())
      return false;
    cb.push_back(value);
    TRISYCL_DUMP_T("Write pipe front = " << cb.front()
                   << " back = " << cb.back());
    return true;
  }


  /** Try to read a value from the pipe

      \param[out] value is the reference to where to store what is
      read

      \return true on success
  */
  bool read(T &value) {
    std::lock_guard<std::mutex> lg { cb_mutex };
    TRISYCL_DUMP_T("Read pipe empty = " << empty());

    if (empty())
      return false;
    TRISYCL_DUMP_T("Read pipe front = " << cb.front()
                   << " back = " << cb.back());
    value = cb.front();
    TRISYCL_DUMP_T("Read pipe value = " << value
                   << "address = " << &(cb.front()));
    cb.pop_front();
    return true;
  }


  /** Compute the amount of elements blocked by write reservations, not yet
      committed

      This includes some normal writes to pipes between/after
      un-committed reservations

      This function assumes that the data structure is locked
  */
std::size_t reserved_for_writing() const {
  if (qrid.empty())
    // No on-going reservation
    return 0;
  else
    /* The reserved size is from the first element of the first
       on-going reservation up to the end of the pipe content */
    return cb.end() - qrid.front().start;
}


  /** Reserve some part of the pipe for writing

      \param[in] s is the number of element to reserve

      \param[out] rid is an iterator to a description of the
      reservation that has been done if successful

      \return true if the reservation was successful

      \todo implement reservation for reading
  */
  bool reserve(std::size_t s,
               rid_iterator &rid)  {
    // Lock the pipe to avoid being disturbed
    std::lock_guard<std::mutex> lg { cb_mutex };

    if (s == 0)
      // Empty reservation requested, so nothing to do
      return false;

    /* Do not use a difference here because it is only about unsigned
       values */
    if (cb.size() + s <= capacity()) {
      TRISYCL_DUMP_T("Before reservation cb.size() = " << cb.size()
                     << " size() = " << size());

      /* If there is enough room in the pipe, just create default
         values in it to do the reservation */
      for (std::size_t i = 0; i != s; ++i)
        cb.push_back();
      /* Compute the location of the first element a posteriori since
         it may not exist a priori if cb was empty before */
      auto first = cb.end() - s;
      /* Add a description of the reservation at the end of the
         reservation queue */
      qrid.emplace_back(first, s);
      // Return the iterator to the last reservation descriptor
      rid = qrid.end() - 1;
      TRISYCL_DUMP_T("After reservation cb.size() = " << cb.size()
                     << " size() = " << size());
      return true;
    }
    else
      // Not enough room in the pipe for the reservation
      return false;
  }


  /** Process the reservations that are ready to be released in the
      reservation queue
  */
  void move_reservation_forward() {
    // Lock the pipe to avoid nuisance
    std::lock_guard<std::mutex> lg { cb_mutex };

    for (;;) {
      if (qrid.empty())
        // No pending reservation, so nothing to do
        break;
      // Get the first reservation
      const auto &rid = qrid.front();
      if (!rid.ready)
        /* If the reservation is not ready to be released, stop
           because it is blocking all the following in the queue
           anyway */
        break;
      // Remove the reservation to be released from the queue
      qrid.pop_front();
      /* ...and process the next reservation to see if it is ready to
         be released too */
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
