#ifndef TRISYCL_SYCL_VENDOR_XILINX_CONFIG_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_CONFIG_HPP

/** \file

    Control the configuration of some implementation of Xilinx-related
    features.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifndef TRISYCL_XILINX_AIE_FIBER_EXECUTOR_THREADS
/// Define the number of threads used in the fiber executor by default
/// if undefined in the compiler option
#define TRISYCL_XILINX_AIE_FIBER_EXECUTOR_THREADS 8
#endif

#ifndef TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER
/// Use a fiber to run the tile core program by default if undefined
/// in the compiler option
#define TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER 1
#endif

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_CONFIG_HPP
