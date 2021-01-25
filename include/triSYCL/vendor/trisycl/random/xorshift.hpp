#ifndef TRISYCL_SYCL_VENDOR_TRISYCL_RANDOM_XORSHIFT_HPP
#define TRISYCL_SYCL_VENDOR_TRISYCL_RANDOM_XORSHIFT_HPP

/** \file A simple and fast pseudo-random generator based on binary shifts
    and exclusive or

    Warning: do not even think about using it in any secure application!

    https://en.wikipedia.org/wiki/Xorshift

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstdint>
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

  /// The tuple representing the state for xorshift128
  struct state_128_t {
    std::uint32_t x, y, z, w;
  };

  /// The type of the internal state of the generator
  using state_t = std::conditional_t
    <bit_size == 32, std::uint32_t, std::conditional_t
     <bit_size == 64, std::uint64_t, std::conditional_t
      <bit_size == 128, state_128_t, void>>>;

  static_assert(!std::is_void_v<state_t>, "Bit size not implemented");

  /// The internal state of the pseudo random generator
  state_t state;

  /// Compute a new pseudo random integer
  const state_t& operator()() {
    if constexpr (bit_size == 32) {
      /* Pick the one of type I with best bit equidistribution from
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
    else {
      // The xor128 from Marsaglia, p. 5 Section "4 Summary"
      auto t = state.x^(state.x<<11);
      state.x = state.y;
      state.y = state.z;
      state.z = state.w;
      state.w = state.w^(state.w>>19)^(t^(t>>8));
    }
    return state;
  }

};

}

#endif // TRISYCL_SYCL_VENDOR_TRISYCL_RANDOM_XORSHIFT_HPP
