#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_DEBUG_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_DEBUG_HPP

/** \file

    Some debugging utilities for ACAP

*/

#include <algorithm>
#include <array>
#include <atomic>

#include <experimental/mdspan>

namespace cl::sycl::vendor::xilinx::acap::debug {

/** Verify that an SPMD program has a bulk-synchronous behaviour
 */
template <typename Geography>
class bsp_checker {
  /// A vector clock to compare differerent tiles together
  std::array<std::atomic<int>, Geography::size> vector_clock_alloc;
  std::experimental::fundamentals_v3::mdspan<std::atomic<int>,
                                             Geography::x_size,
                                             Geography::y_size>
  vector_clock { &vector_clock_alloc[0] };

public:

  /** Check that none of the tiles executing this are separated by
      more than one execution step eachother */
  void check(int x, int y) {
    ++vector_clock(x, y);
    /* While the algorithm execution is not atomic, it is enough to
       find some asynchronous bugs */
    auto [min_element, max_element] =
      std::minmax_element(vector_clock_alloc.cbegin(),
                          vector_clock_alloc.cend());

    if (*max_element - *min_element > 1) {
      TRISYCL_DUMP_T(std::dec << "compute(" << x << ',' << y
                     << ") vector clock min = "
                     << *min_element << ", max = " << *max_element);
      std::terminate();
    }
  }

};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_DEBUG_HPP
