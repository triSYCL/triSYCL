#ifndef TRISYCL_SYCL_ROUNDING_MODE_HPP
#define TRISYCL_SYCL_ROUNDING_MODE_HPP

/** \file

    Define rounding modes used in SYCL

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl {

/// Rounding mode for vector conversions
enum class rounding_mode {
  automatic, /// Default rounding mode, rtz for integers, rte for floating-point
  rte,       /// Round to nearest even
  rtz,       /// Round towards zero
  rtp,       /// Round towards positive infinity
  rtn        /// Round towards negative infinity
};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ROUNDING_MODE_HPP
