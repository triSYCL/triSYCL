#ifndef TRISYCL_SYCL_VENDOR_TRISYCL_RANDOM_XORSHIFT_HPP
#define TRISYCL_SYCL_VENDOR_TRISYCL_RANDOM_XORSHIFT_HPP

/** \file A simple and fast pseudo-random generator based on binary shifts
    and exclusive or

    Warning: do not even think about using it in any secure application!

    https://en.wikipedia.org/wiki/Xorshift

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <array>
#include <cstdint>
#include <limits>
#include <type_traits>

/// Random generators
namespace trisycl::vendor::trisycl::random {

/** A possible implementation of the xorshift RNGs from

    Marsaglia, George (July 2003). "Xorshift RNGs". Journal of
    Statistical Software. 8 (14). doi:10.18637/jss.v008.i14

    Panneton, François; L'Écuyer, Pierre (October 2005). "On the
    xorshift random number generators" (PDF). ACM Transactions on
    Modeling and Computer Simulation. 15
    (4):346–361. doi:10.1145/1113316.1113319
*/
template <int Nbits = 32>
struct xorshift {
  /// Bit size of the random generator data type
  static auto constexpr bit_size = Nbits;

  /// The default initial internal state of the pseudo random generator
  static auto constexpr initial_state = [] {
    if constexpr (bit_size == 32) {
      /* Default value for xor32 from Marsaglia,
         p. 4 Section "3 Application to Xorshift RNGs" */
      return std::uint32_t { 2463534242 };
    } else if constexpr (bit_size == 64) {
      /* Default value for xor32 from Marsaglia,
         p. 4 Section "3 Application to Xorshift RNGs" */
      return std::uint64_t { 88172645463325252 };
    } else if constexpr (bit_size == 128) {
        /// Default value for xor128 from Marsaglia, p. 5 Section "4 Summary"
      return std::array<std::uint32_t, 4>
        { 123456789, 362436069, 521288629, 88675123 };
    } else
      return nullptr;
  }();

  /// The type of the internal state of the generator
  using value_type = std::remove_const_t<decltype(initial_state)>;

  /// The type of the result
  using result_type = value_type;

  static_assert(!std::is_same_v<value_type, std::nullptr_t>,
                "Bit size not implemented");

  /// The internal state of the pseudo random generator
  value_type state = initial_state;


  /// The minimum returned value
  static auto constexpr min() {
    // It cannot return 0
    return std::numeric_limits<result_type>::min() + 1;
  };


  /// The maximum returned value
  static auto constexpr max() {
    return std::numeric_limits<result_type>::max();
  };


  /** Initialize the internal state from a user-given value

      Do not use the value 0 or the output is always 0
  */
  xorshift(const value_type& s)
    : state { s }
  {}


  xorshift() = default;


  /// Compute a new pseudo random integer
  const result_type& operator()() {
    if constexpr (bit_size == 32) {
      /* Pick the one of type "I" with best bit equidistribution from
         Panneton & L'Écuyer, Section "5.1 Equidistribution
         properties"

         X4 = (I + Ra )(I + Lb )(I + Rc )
      */
      state ^= state >> 7;
      state ^= state << 1;
      state ^= state >> 9;
    }
    else if constexpr (bit_size == 64) {
      /* The xor64 from Marsaglia, p. 4 Section "3 Application to
         Xorshift RNGs" */
      state ^= state << 13;
      state ^= state >> 7;
      state ^= state << 17;
    }
    else if constexpr (bit_size == 128) {
      // The xor128 from Marsaglia, p. 5 Section "4 Summary"
      auto t = state[0]^(state[0]<<11);
      state[0] = state[1];
      state[1] = state[2];
      state[2] = state[3];
      state[3] = state[3]^(state[3]>>19)^(t^(t>>8));
    }
    else
      // Just to error in that case
      return nullptr;

    return state;
  }

};

}

#endif // TRISYCL_SYCL_VENDOR_TRISYCL_RANDOM_XORSHIFT_HPP
