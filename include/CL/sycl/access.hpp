#ifndef TRISYCL_SYCL_ACCESS_HPP
#define TRISYCL_SYCL_ACCESS_HPP

/** \file The OpenCL SYCL access naming space

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

// SYCL dwells in the trisycl namespace
namespace trisycl {

/** \addtogroup data Data access and storage in SYCL

    @{
*/

/** Describe the type of access by kernels.

    \todo This values should be normalized to allow separate compilation
    with different implementations?
*/
namespace access {
  /* By using "enum mode" here instead of "enum struct mode", we have for
     example "write" appearing both as trisycl::access::mode::write and
     trisycl::access::write, instead of only the last one. This seems
     more conform to the specification. */

  /// This describes the type of the access mode to be used via accessor
  enum class mode {
    read = 42, /**< Read-only access. Insist on the fact that
                    read_write != read + write */
    write, ///< Write-only access, but previous content *not* discarded
    read_write, ///< Read and write access
    discard_write, ///< Write-only access and previous content discarded
    discard_read_write, /**< Read and write access and previous
                             content discarded*/
    atomic ///< Atomic access
  };


  /** The target enumeration describes the type of object to be accessed
      via the accessor
  */
  enum class target {
    global_buffer = 2014, //< Just pick a random number...
    constant_buffer,
    local,
    image,
    host_buffer,
    host_image,
    image_array,
    pipe,
    blocking_pipe
  };


  /** Precise the address space a barrier needs to act on
  */
  enum class fence_space : char {
    local_space,
    global_space,
    global_and_local
  };

}

/// @} End the data Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ACCESS_HPP
