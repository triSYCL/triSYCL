#ifndef TRISYCL_SYCL_PIPE_DETAIL_PIPE_HPP
#define TRISYCL_SYCL_PIPE_DETAIL_PIPE_HPP

/** \file The OpenCL SYCL pipe<> details

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <deque>

#ifdef TRISYCL_MAKE_BOOST_CIRCULARBUFFER_THREAD_SAFE
/* The debug mode of boost/circular_buffer.hpp has a nasty side effect
   in multithread applications using several iterators at the same
   time even in read-only mode because the library tracks them for
   debugging purpose in a... non-thread safe way

   This is described in https://svn.boost.org/trac/boost/ticket/6277
   and fixed with https://github.com/boostorg/circular_buffer/pull/9
*/
#define BOOST_CB_DISABLE_DEBUG
#endif
#include <boost/circular_buffer.hpp>

namespace trisycl::detail {

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
class pipe : public detail::debug<pipe<T>> {

public:

  using value_type = T;

  /// Implement the pipe with a circular buffer
  using implementation_t = boost::circular_buffer<value_type>;

private:

  /// The circular buffer to store the elements
  boost::circular_buffer<value_type> cb;

  /** To protect the access to the circular buffer.

      In case the object is capture in a lambda per copy, make it
      mutable. */
  mutable std::mutex cb_mutex;

  /// The queue of pending write reservations
  std::deque<reserve_id<value_type>> w_rid_q;

public:
#ifndef _MSC_VER
  using rid_iterator = typename decltype(w_rid_q)::iterator;
#else
  using rid_iterator = typename std::deque<reserve_id<value_type>>::iterator;
#endif

private:

  /// The queue of pending read reservations
  std::deque<reserve_id<value_type>> r_rid_q;

  /// Track the number of frozen elements related to read reservations
  std::size_t read_reserved_frozen;

  /// To signal that a read has been successful
  std::condition_variable read_done;

  /// To signal that a write has been successful
  std::condition_variable write_done;

  /// To control the debug mode, disabled by default
  bool debug_mode = false;

public:

  /// True when the pipe is currently used for reading
  bool used_for_reading = false;

  /// True when the pipe is currently used for writing
  bool used_for_writing = false;

  /// Create a pipe as a circular buffer of the required capacity
  pipe(std::size_t capacity) : cb { capacity }, read_reserved_frozen { 0 } { }


  /** Return the maximum number of elements that can fit in the pipe
   */
  std::size_t capacity() const {
    // No lock required since it is fixed and set at construction time
    return cb.capacity();
  }

private:

  /** Get the current number of elements in the pipe that can be read

      This is obviously a volatile value which is constrained by the
      theory of restricted relativity.

      Note that on some devices it may be costly to implement (for
      example on FPGA).
   */
  std::size_t size() const {
    TRISYCL_DUMP_T("size() cb.size() = " << cb.size()
                   << " cb.end() = " << (void *)&*cb.end()
                   << " reserved_for_reading() = " << reserved_for_reading()
                   << " reserved_for_writing() = " << reserved_for_writing());
    /* The actual number of available elements depends from the
       elements blocked by some reservations.
       This prevents a consumer to read into reserved area. */
    return cb.size() - reserved_for_reading() - reserved_for_writing();
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

      \param[in] blocking specify if the call wait for the operation
      to succeed

      \return true on success

      \todo provide a && version
  */
  bool write(const T &value, bool blocking = false) {
    // Lock the pipe to avoid being disturbed
    std::unique_lock<std::mutex> ul { cb_mutex };
    TRISYCL_DUMP_T("Write pipe full = " << full()
                   << " value = " << value);

    if (blocking)
      /* If in blocking mode, wait for the not full condition, that
         may be changed when a read is done */
      read_done.wait(ul, [&] { return !full(); });
    else if (full())
      return false;

    cb.push_back(value);
    TRISYCL_DUMP_T("Write pipe front = " << cb.front()
                   << " back = " << cb.back()
                   << " cb.begin() = " << (void *)&*cb.begin()
                   << " cb.size() = " << cb.size()
                   << " cb.end() = " << (void *)&*cb.end()
                   << " reserved_for_reading() = " << reserved_for_reading()
                   << " reserved_for_writing() = " << reserved_for_writing());
    // Micro-optimization: unlock before the notification
    // https://en.cppreference.com/w/cpp/thread/condition_variable/notify_all
    ul.unlock();
    // Notify the clients waiting to read something from the pipe
    write_done.notify_all();
    return true;
  }


  /** Try to read a value from the pipe

      \param[out] value is the reference to where to store what is
      read

      \param[in] blocking specify if the call wait for the operation
      to succeed

      \return true on success
  */
  bool read(T &value, bool blocking = false) {
    // Lock the pipe to avoid being disturbed
    std::unique_lock<std::mutex> ul { cb_mutex };
    TRISYCL_DUMP_T("Read pipe empty = " << empty());

    if (blocking)
      /* If in blocking mode, wait for the not empty condition, that
         may be changed when a write is done */
      write_done.wait(ul, [&] { return !empty(); });
    else if (empty())
      return false;

    TRISYCL_DUMP_T("Read pipe front = " << cb.front()
                   << " back = " << cb.back()
                   << " reserved_for_reading() = " << reserved_for_reading());
    if (read_reserved_frozen)
      /** If there is a pending reservation, read the next element to
          be read and update the number of reserved elements */
      value = cb.begin()[read_reserved_frozen++];
    else {
      /* There is no pending read reservation, so pop the read value
         from the pipe */
      value = cb.front();
      cb.pop_front();
    }

    TRISYCL_DUMP_T("Read pipe value = " << value);
    // Micro-optimization: unlock before the notification
    // https://en.cppreference.com/w/cpp/thread/condition_variable/notify_all
    ul.unlock();
    // Notify the clients waiting for some room to write in the pipe
    read_done.notify_all();
    return true;
  }


  /** Compute the amount of elements blocked by read reservations, not yet
      committed

      This includes some normal reads to pipes between/after
      un-committed reservations

      This function assumes that the data structure is locked
  */
  std::size_t reserved_for_reading() const {
    return read_reserved_frozen;
  }


  /** Compute the amount of elements blocked by write reservations, not yet
      committed

      This includes some normal writes to pipes between/after
      un-committed reservations

      This function assumes that the data structure is locked
  */
  std::size_t reserved_for_writing() const {
    if (w_rid_q.empty())
      // No on-going reservation
      return 0;
    else
      /* The reserved size is from the first element of the first
         on-going reservation up to the end of the pipe content */
      return cb.end() - w_rid_q.front().start;
  }


  /** Reserve some part of the pipe for reading

      \param[in] s is the number of element to reserve

      \param[out] rid is an iterator to a description of the
      reservation that has been done if successful

      \param[in] blocking specify if the call wait for the operation
      to succeed

      \return true if the reservation was successful
  */
  bool reserve_read(std::size_t s,
                    rid_iterator &rid,
                    bool blocking = false)  {
    // Lock the pipe to avoid being disturbed
    std::unique_lock<std::mutex> ul { cb_mutex };

    TRISYCL_DUMP_T("Before read reservation cb.size() = " << cb.size()
                   << " size() = " << size());
    if (s == 0)
      // Empty reservation requested, so nothing to do
      return false;

    if (blocking)
      /* If in blocking mode, wait for enough elements to read in the
         pipe for the reservation. This condition can change when a
         write is done */
      write_done.wait(ul, [&] { return s <= size(); });
    else if (s > size())
      // Not enough elements to read in the pipe for the reservation
      return false;

    // Compute the location of the first element of the reservation
    auto first = cb.begin() + read_reserved_frozen;
    // Increment the number of frozen elements
    read_reserved_frozen += s;
    /* Add a description of the reservation at the end of the
       reservation queue */
    r_rid_q.emplace_back(first, s);
    // Return the iterator to the last reservation descriptor
    rid = r_rid_q.end() - 1;
    TRISYCL_DUMP_T("After reservation cb.size() = " << cb.size()
                   << " size() = " << size());
    return true;
  }


  /** Reserve some part of the pipe for writing

      \param[in] s is the number of element to reserve

      \param[out] rid is an iterator to a description of the
      reservation that has been done if successful

      \param[in] blocking specify if the call wait for the operation
      to succeed

      \return true if the reservation was successful
  */
  bool reserve_write(std::size_t s,
                     rid_iterator &rid,
                     bool blocking = false)  {
    // Lock the pipe to avoid being disturbed
    std::unique_lock<std::mutex> ul { cb_mutex };

    TRISYCL_DUMP_T("Before write reservation cb.size() = " << cb.size()
                   << " size() = " << size());
    if (s == 0)
      // Empty reservation requested, so nothing to do
      return false;

    if (blocking)
      /* If in blocking mode, wait for enough room in the pipe, that
         may be changed when a read is done. Do not use a difference
         here because it is only about unsigned values */
      read_done.wait(ul, [&] { return cb.size() + s <= capacity(); });
    else if (cb.size() + s > capacity())
      // Not enough room in the pipe for the reservation
      return false;

    /* If there is enough room in the pipe, just create default values
         in it to do the reservation */
    for (std::size_t i = 0; i != s; ++i)
      cb.push_back();
    /* Compute the location of the first element a posteriori since it
         may not exist a priori if cb was empty before */
    auto first = cb.end() - s;
    /* Add a description of the reservation at the end of the
       reservation queue */
    w_rid_q.emplace_back(first, s);
    // Return the iterator to the last reservation descriptor
    rid = w_rid_q.end() - 1;
    TRISYCL_DUMP_T("After reservation cb.size() = " << cb.size()
                   << " size() = " << size());
    return true;
  }


  /** Process the read reservations that are ready to be released in the
      reservation queue
  */
  void move_read_reservation_forward() {
    // Lock the pipe to avoid nuisance
    std::unique_lock<std::mutex> lock { cb_mutex };

    for (;;) {
      if (r_rid_q.empty())
        // No pending reservation, so nothing to do
        break;
      if (!r_rid_q.front().ready)
        /* If the first reservation is not ready to be released, stop
           because it is blocking all the following in the queue
           anyway */
        break;
      // Remove the reservation to be released from the queue
      r_rid_q.pop_front();
      std::size_t n_to_pop;
      if (r_rid_q.empty())
        // If it was the last one, remove all the reservation
        n_to_pop = read_reserved_frozen;
      else
        // Else remove everything up to the next reservation
        n_to_pop =  r_rid_q.front().start - cb.begin();
      // No longer take into account these reserved slots
      read_reserved_frozen -= n_to_pop;
      // Release the elements from the FIFO
      while (n_to_pop--)
        cb.pop_front();
      // Notify the clients waiting for some room to write in the pipe
      read_done.notify_all();
      /* ...and process the next reservation to see if it is ready to
         be released too */
    }
  }


  /** Process the write reservations that are ready to be released in the
      reservation queue
  */
  void move_write_reservation_forward() {
    // Lock the pipe to avoid nuisance
    std::lock_guard<std::mutex> lg { cb_mutex };

    for (;;) {
      if (w_rid_q.empty())
        // No pending reservation, so nothing to do
        break;
      // Get the first reservation
      const auto &rid = w_rid_q.front();
      if (!rid.ready)
        /* If the reservation is not ready to be released, stop
           because it is blocking all the following in the queue
           anyway */
        break;
      // Remove the reservation to be released from the queue
      w_rid_q.pop_front();
      // Notify the clients waiting to read something from the pipe
      write_done.notify_all();
      /* ...and process the next reservation to see if it is ready to
         be released too */
    }
  }

};

/// @} End the execution Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PIPE_DETAIL_PIPE_HPP
