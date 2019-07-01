#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_DEBUG_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_DEBUG_HPP

/** \file

    Some debugging utilities for ACAP
*/

#include <algorithm>
#include <array>
#include <atomic>

#include <experimental/mdspan>

/** \defgroup acap Extensions for Xilinx ACAP Versal

    Some classes and functions to support devices including CPU, FPGA
    & AI Engine CGRA
    @{
*/

/** \defgroup acap_debug Debugging support for ACAP

    Some classes and functions to help debuging ACAP applications,
    such as verifying BSP forward progress
    @{
*/

namespace trisycl::vendor::xilinx::acap::debug {

/// @}
/// \ingroup acap_debug
/// @{

/** Verify that an SPMD program has a bulk-synchronous behaviour
    across all the tiles of the CGRA.

    If there are some discrepancies, display the tile id which is too
    late or too early.
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
      more than one execution step each other.

      This functions has to be called in a BSP synchronous way by all
      the tiles.
  */
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

/// @} End the acap_debug Doxygen group

/// @} End the acap Doxygen group

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_DEBUG_HPP
