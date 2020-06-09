#ifndef TRISYCL_SYCL_PIPE_HPP
#define TRISYCL_SYCL_PIPE_HPP

/** \file The SYCL data-flow pipe extension

    Ronan dot Keryell arobase Xilinx point com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include <boost/fiber/buffered_channel.hpp>

namespace trisycl {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** SYCL pipe object

    The member functions can throw for now to help debugging or
    implementation on CPU.

    This is mainly a CPU implementation of the Intel extension
    described in

    - https://github.com/intel/llvm/pull/292

    - https://github.com/intel/llvm/pull/635

    - https://github.com/intel/llvm/issues/832

    \todo Bike-shed the naming & API according to current C++ proposals
    or libraries about static sized FIFO

    This could be implemented on top of
    https://xilinx.github.io/XRT/master/html/streaming_data_support.html
    and/or expose some features from XRT
*/
template <typename Name,
          typename T,
          std::size_t MinCapacity = 0,
          typename SizeT = std::size_t>
class pipe {

  /** Finds the smallest integral power of two not less than the given
      value but at least 2 to fit the requirement from
      boost::fibers::buffered_channel

      \todo At some point use C++20 std::bit_ceil instead of this
      DIY function */
  static constexpr std::size_t bit_ceil_at_least_2(std::size_t value) {
    std::size_t result = 2;
    while (value > result)
      result *= 2;
    return result;
  }

public:

  // The data type transmitted by the pipe
  using value_type = T;

  /** The minimum capacity in value_type of this pipe.

      The underlying implementation requires a power of 2 */
  static constexpr SizeT min_capacity = bit_ceil_at_least_2(MinCapacity);

  // The data type used to represent the capacity itself
  using size_type = SizeT;

private:

  /// A FIFO used to implement the pipe
  static inline boost::fibers::buffered_channel<value_type>
  channel { min_capacity };

public:

  /** Blocking read from the pipe

      \return the value
  */
  static value_type read() {
    return channel.value_pop();
  }


  /** Blocking write to the pipe

      \param[in] value is the data to send to the pipe
  */
  static void write(const value_type &value) {
    channel.push(value);
  }


  /** Non-blocking read from the pipe

      \param[out] success_code is true iff the read was successful

      \return the value read from the pipe or the default one
  */
  static value_type read(bool &success_code) {
    value_type value;
    success_code =
      channel.try_pop(value) == boost::fibers::channel_op_status::success;
    /* The current API might be inefficient if we copy always value back.
       But we should have a copy elision... */
    return value;
  }


  /** Non-blocking write to the pipe

      \param[out] success_code is true iff the read was successful

      \return the value read from the pipe or the default one
  */
  static void write(const value_type &data, bool &success_code) {
      success_code =
        channel.try_push(data) != boost::fibers::channel_op_status::full;
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

#endif // TRISYCL_SYCL_PIPE_HPP
