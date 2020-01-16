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

    You can use CL/sycl.hpp or SYCL/sycl.hpp headers instead to use this
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

    Copyright 2015--2020 Xilinx, Inc.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


/** Some global triSYCL configuration */
#include "triSYCL/detail/global_config.hpp"
#include "triSYCL/detail/default_classes.hpp"


/* All the SYCL components, one per file */
#include "triSYCL/access.hpp"
#include "triSYCL/accessor.hpp"
#include "triSYCL/allocator.hpp"
#include "triSYCL/address_space.hpp"
#include "triSYCL/buffer.hpp"
#include "triSYCL/context.hpp"
#include "triSYCL/device.hpp"
#include "triSYCL/device_runtime.hpp"
#include "triSYCL/device_selector.hpp"
#include "triSYCL/error_handler.hpp"
#include "triSYCL/event.hpp"
#include "triSYCL/exception.hpp"
#include "triSYCL/group.hpp"
#include "triSYCL/half.hpp"
#include "triSYCL/handler.hpp"
#include "triSYCL/h_item.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/image.hpp"
#include "triSYCL/item.hpp"
#include "triSYCL/math.hpp"
#include "triSYCL/nd_item.hpp"
#include "triSYCL/nd_range.hpp"
#include "triSYCL/opencl_types.hpp"
#include "triSYCL/parallelism.hpp"
#include "triSYCL/pipe.hpp"
#include "triSYCL/pipe_reservation.hpp"
#include "triSYCL/platform.hpp"
#include "triSYCL/program.hpp"
#include "triSYCL/queue.hpp"
#include "triSYCL/range.hpp"
#include "triSYCL/static_pipe.hpp"
#include "triSYCL/vec.hpp"

// Some includes at the end to break some dependencies
#include "triSYCL/device_selector/detail/device_selector_tail.hpp"
#include "triSYCL/context/detail/context_tail.hpp"
#include "triSYCL/device/detail/device_tail.hpp"
#include "triSYCL/queue/detail/queue_tail.hpp"
#ifdef TRISYCL_OPENCL
#include "triSYCL/device/detail/opencl_device_tail.hpp"
#endif
#include "triSYCL/platform/detail/platform_tail.hpp"
#include "triSYCL/platform/detail/host_platform_tail.hpp"
#ifdef TRISYCL_OPENCL
#include "triSYCL/platform/detail/opencl_platform_tail.hpp"
#endif

// Some include files for Xilinx-specific features, such as for FPGA
#include "triSYCL/vendor/Xilinx/fpga.hpp"

// An extension about constexpr host introspection API
//#include "triSYCL/extension/ce/platform.hpp"

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_TRISYCL_HPP
