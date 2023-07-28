#ifndef TRISYCL_TRISYCL_HPP
#define TRISYCL_TRISYCL_HPP

/** \file

    \mainpage

    This is the main triSYCL C++ header file to experiment with the
    SYCL specification.

    It declares everything into the \c ::trisycl namespace instead of
    \c ::cl::sycl for SYCL 1.2.1 to express triSYCL specific extensions
    and to be used in full or partially with other SYCL
    implementations without naming conflict.

    You can use CL/sycl.hpp or sycl.hpp headers instead to use this
    implementation with the usual \c ::cl::sycl or \c ::sycl namespaces.

    For more information about SYCL:
    http://www.khronos.org/sycl/

    For more information on this project and to access to the source of
    this file, look at https://github.com/triSYCL/triSYCL

    The Doxygen version of the implementation itself is in
    http://trisycl.github.io/triSYCL/Doxygen/triSYCL/html and
    http://trisycl.github.io/triSYCL/Doxygen/triSYCL/triSYCL-implementation-refman.pdf


    Ronan at keryell dot FR

    Copyright 2014--2015 Advanced Micro Devices, Inc.

    Copyright 2015--2019 Xilinx, Inc.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


/** Some global triSYCL configuration */
#include "trisycl/detail/global_config.hpp"
#include "trisycl/detail/default_classes.hpp"

/** Related to triSYCL/Intel SYCL compiler offload wrapping and binary/program
    management
*/
#include "trisycl/detail/program_manager.hpp"

/* All the SYCL components, one per file */
#include "trisycl/access.hpp"
#include "trisycl/accessor.hpp"
#include "trisycl/allocator.hpp"
#include "trisycl/address_space.hpp"
#include "trisycl/buffer.hpp"
#include "trisycl/context.hpp"
#include "trisycl/device.hpp"
#include "trisycl/device_runtime.hpp"
#include "trisycl/device_selector.hpp"
#include "trisycl/error_handler.hpp"
#include "trisycl/event.hpp"
#include "trisycl/exception.hpp"
#include "trisycl/group.hpp"
#include "trisycl/half.hpp"
#include "trisycl/handler.hpp"
#include "trisycl/h_item.hpp"
#include "trisycl/id.hpp"
#include "trisycl/image.hpp"
#include "trisycl/item.hpp"
#include "trisycl/math.hpp"
#include "trisycl/nd_item.hpp"
#include "trisycl/nd_range.hpp"
#include "trisycl/opencl_types.hpp"
#include "trisycl/parallelism.hpp"
#include "trisycl/pipe.hpp"
#include "trisycl/pipe_reservation.hpp"
#include "trisycl/platform.hpp"
#include "trisycl/program.hpp"
#include "trisycl/queue.hpp"
#include "trisycl/range.hpp"
#include "trisycl/static_pipe.hpp"
#include "trisycl/vec.hpp"

// Some includes at the end to break some dependencies
#include "trisycl/device_selector/detail/device_selector_tail.hpp"
#include "trisycl/context/detail/context_tail.hpp"
#include "trisycl/device/detail/device_tail.hpp"
#include "trisycl/queue/detail/queue_tail.hpp"
#ifdef TRISYCL_OPENCL
#include "trisycl/device/detail/opencl_device_tail.hpp"
#endif
#include "trisycl/platform/detail/platform_tail.hpp"
#include "trisycl/platform/detail/host_platform_tail.hpp"
#ifdef TRISYCL_OPENCL
#include "trisycl/platform/detail/opencl_platform_tail.hpp"
#endif

// Some include files for Xilinx-specific features

/// For Xilinx FPGA
#include "trisycl/vendor/Xilinx/fpga.hpp"

/// For Xilinx ACAP
#include "trisycl/vendor/Xilinx/acap.hpp"

// Xilinx-specific extension for some graphics support
#include "trisycl/vendor/Xilinx/graphics.hpp"

// An extension about constexpr host introspection API
//#include "trisycl/extension/ce/platform.hpp"

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_TRISYCL_HPP
