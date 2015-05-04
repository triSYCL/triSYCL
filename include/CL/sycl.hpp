/** \file

    \mainpage

    This is a simple C++ sequential OpenCL SYCL C++ header file to
    experiment with the OpenCL CL provisional specification.

    For more information about OpenCL SYCL:
    http://www.khronos.org/opencl/sycl/

    The aim of this file is mainly to define the interface of SYCL so that
    the specification documentation can be derived from it through tools
    like Doxygen or Sphinx. This explains why there are many functions and
    classes that are here only to do some forwarding in some inelegant way.
    This file is documentation driven and not implementation-style driven.

    For more information on this project and to access to the source of
    this file, look at https://github.com/amd/triSYCL

    The Doxygen version of the API in
    http://amd.github.io/triSYCL/Doxygen/SYCL/html and
    http://amd.github.io/triSYCL/Doxygen/SYCL/SYCL-API-refman.pdf

    The Doxygen version of the implementation itself is in
    http://amd.github.io/triSYCL/Doxygen/triSYCL/html and
    http://amd.github.io/triSYCL/Doxygen/triSYCL/triSYCL-implementation-refman.pdf


    Ronan.Keryell at AMD point com
    Ronan at keryell dot FR

    Copyright 2014--2015 Advanced Micro Devices, Inc.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


/** The vector type to be used as SYCL vector

    \todo this should be more local, such as SYCL_VECTOR_CLASS or
    _SYCL_VECTOR_CLASS

    \todo use a typedef or a using instead of a macro?

    \todo implement __NO_STD_VECTOR

    \todo Table 3.1 in provisional specification is wrong: VECTOR_CLASS
    not at the right place
 */
#define VECTOR_CLASS std::vector


/** The string type to be used as SYCL string

    \todo this should be more local, such as SYCL_STRING_CLASS or
    _SYCL_STRING_CLASS

    \todo use a typedef or a using instead of a macro?

    \todo implement __NO_STD_STRING

    \todo Table 3.2 in provisional specification is wrong: STRING_CLASS
    not at the right place
 */
#define STRING_CLASS std::string


// SYCL dwells in the cl::sycl namespace
namespace cl {
namespace sycl {

/** \addtogroup data Data access and storage in SYCL

    @{
*/

/** Describe the type of access by kernels.

    \todo This values should be normalized to allow separate compilation
    with different implementations?
*/
namespace access {
  /* By using "enum mode" here instead of "enum struct mode", we have for
     example "write" appearing both as cl::sycl::access::mode::write and
     cl::sycl::access::write, instead of only the last one. This seems
     more conform to the specification. */

  /// This describes the type of the access mode to be used via accessor
  enum mode {
    read = 42, //?< Why not? Insist on the fact that read_write != read + write
    write, ///< Write-only access, but previous content *not* discarded
    read_write, ///< Read and write access
    discard_write, ///< Write-only access and previous content discarded
    discard_read_write ///< Read and write access and previous content discarded
  };


  /** The target enumeration describes the type of object to be accessed
     via the accessor
   */
  enum target {
    global_buffer = 2014, //< Just pick a random number...
    constant_buffer,
    local,
    image,
    host_buffer,
    host_image,
    image_array,
    cl_buffer,
    cl_image
  };


  /** Precise the address space a barrier needs to act on
   */
  enum class address_space : char {
    local,
    global,
    global_and_local
  };

}

/// @} End the data Doxygen group

}
}

/** Some global triSYCL configuration */
#include "CL/sycl/detail/global_config.hpp"


/* All the SYCL components, one per file */
#include "CL/sycl/accessor.hpp"
#include "CL/sycl/address_space.hpp"
#include "CL/sycl/buffer.hpp"
#include "CL/sycl/command_group.hpp"
#include "CL/sycl/context.hpp"
#include "CL/sycl/device.hpp"
#include "CL/sycl/device_selector.hpp"
#include "CL/sycl/error_handler.hpp"
#include "CL/sycl/exception.hpp"
#include "CL/sycl/group.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/image.hpp"
#include "CL/sycl/item.hpp"
#include "CL/sycl/nd_item.hpp"
#include "CL/sycl/nd_range.hpp"
#include "CL/sycl/parallelism.hpp"
#include "CL/sycl/platform.hpp"
#include "CL/sycl/queue.hpp"
#include "CL/sycl/range.hpp"
#include "CL/sycl/storage.hpp"
#include "CL/sycl/vec.hpp"


/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/
