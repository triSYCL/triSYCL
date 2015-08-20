#ifndef TRISYCL_SYCL_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_HPP

/** \file The OpenCL SYCL device

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/detail/default_classes.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/exception.hpp"
#include "CL/sycl/info/param_traits.hpp"
#include "CL/sycl/platform.hpp"

namespace cl {
namespace sycl {

class device_selector;
class platform;

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/


namespace info {
/** Device information descriptors

    From specs/latex/headers/deviceInfo.h in the specification

    \todo Should be unsigned int?
*/
enum class device : int {
  device_type,
  vendor_id,
  max_compute_units,
  max_work_item_dimensions,
  max_work_item_sizes,
  max_work_group_size,
  preferred_vector_width_char,
  preferred_vector_width_short,
  preferred_vector_width_int,
  preferred_vector_width_long_long,
  preferred_vector_width_float,
  preferred_vector_width_double,
  preferred_vector_width_half,
  native_vector_witdth_char,
  native_vector_witdth_short,
  native_vector_witdth_int,
  native_vector_witdth_long_long,
  native_vector_witdth_float,
  native_vector_witdth_double,
  native_vector_witdth_half,
  max_clock_frequency,
  address_bits,
  max_mem_alloc_size,
  image_support,
  max_read_image_args,
  max_write_image_args,
  image2d_max_height,
  image2d_max_width,
  image3d_max_height,
  image3d_max_widht,
  image3d_mas_depth,
  image_max_buffer_size,
  image_max_array_size,
  max_samplers,
  max_parameter_size,
  mem_base_addr_align,
  single_fp_config,
  double_fp_config,
  global_mem_cache_type,
  global_mem_cache_line_size,
  global_mem_cache_size,
  global_mem_size,
  max_constant_buffer_size,
  max_constant_args,
  local_mem_type,
  local_mem_size,
  error_correction_support,
  host_unified_memory,
  profiling_timer_resolution,
  endian_little,
  is_available,
  is_compiler_available,
  is_linker_available,
  execution_capabilities,
  queue_properties,
  built_in_kernels,
  platform,
  name,
  vendor,
  driver_version,
  profile,
  device_version,
  opencl_version,
  extensions,
  printf_buffer_size,
  preferred_interop_user_sync,
  parent_device,
  partition_max_sub_devices,
  partition_properties,
  partition_affinity_domain,
  partition_type,
  reference_count
};

enum class device_partition_property : int {
  unsupported,
  partition_equally,
  partition_by_counts,
  partition_by_affinity_domain,
  partition_affinity_domain_next_partitionable
};

enum class device_affinity_domain : int {
  unsupported,
  numa,
  L4_cache,
  L3_cache,
  L2_cache,
  next_partitionable
};

enum class device_partition_type : int {
  no_partition,
  numa,
  L4_cache,
  L3_cache,
  L2_cache,
  L1_cache
};

enum class local_mem_type : int {
  none,
  local,
  global
};

enum class fp_config : int {
  denorm,
  inf_nan,
  round_to_nearest,
  round_to_zero,
  round_to_inf,
  fma,
  correctly_rounded_divide_sqrt,
  soft_float
};

enum class global_mem_cache_type : int {
  none,
  read_only,
  write_only
};

enum class device_execution_capabilities : unsigned int {
  exec_kernel,
  exec_native_kernel
};


using device_fp_config = unsigned int;
using device_exec_capabilities = unsigned int;
using device_queue_properties = unsigned int;


/** Query the return type for get_info() on context stuff

    \todo To be implemented, return always void.
*/
TRISYCL_INFO_PARAM_TRAITS_ANY_T(info::device, void)

}


/** SYCL device

    \todo The implementation is quite minimal for now. :-)
*/
class device {

public:

#ifdef TRISYCL_OPENCL
  /** Construct a device class instance using cl_device_id of the OpenCL
      device

      Return synchronous errors via the SYCL exception
      class.

      Retain a reference to the OpenCL device and if this device was an
      OpenCL subdevice the device should be released by the caller when it
      is no longer needed.

      \todo To be implemented
  */
  explicit device(cl_device_id deviceId) {
    detail::unimplemented();
  }
#endif


  /** Construct a device class instance using the device selector
      provided

      Return errors via C++ exception class.

      \todo To be implemented
  */
  explicit device(const device_selector &deviceSelector) {
    detail::unimplemented();
  }


  /** The default constructor will create an instance of the SYCL host
      device

      Get the default constructors back.
  */
  device() = default;


#ifdef TRISYCL_OPENCL
  /** Return the cl_device_id of the underlying OpenCL platform

      Return synchronous errors via the SYCL exception class.

      Retain a reference to the returned cl_device_id object. Caller
      should release it when finished.

      In the case where this is the SYCL host device it will return a
      nullptr.

      \todo To be implemented
  */
  cl_device_id get() const {
    detail::unimplemented();
    return {};
  }
#endif

  /** Return true if the device is a SYCL host device

      \todo To be implemented
  */
  bool is_host() const {
    detail::unimplemented();
    return true;
  }


  /** Return true if the device is an OpenCL CPU device

      \todo To be implemented
  */
  bool is_cpu() const {
    detail::unimplemented();
    return {};
  }


  /** Return true if the device is an OpenCL GPU device

      \todo To be implemented
  */
  bool is_gpu() const {
    detail::unimplemented();
    return {};
  }


  /** Return true if the device is an OpenCL accelerator device

      \todo To be implemented
  */
  bool is_accelerator() const {
    detail::unimplemented();
    return {};
  }


  /** Return the platform of device

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
  platform get_platform() const {
    detail::unimplemented();
    return {};
  }


  /** Return a list of all available devices

      Return synchronous errors via SYCL exception classes.

      \todo To be implemented
  */
  static vector_class<device>
  get_devices(info::device_type deviceType = info::device_type::all) {
    detail::unimplemented();
    return {};
  }


  /** Query the device for OpenCL info::device info

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
  template <info::device Param>
  typename info::param_traits<info::device, Param>::type
  get_info() const {
    detail::unimplemented();
    return {};
  }


  /** Specify whether a specific extension is supported on the device.

      \todo To be implemented
  */
  bool has_extension(const string_class &extension) const {
    detail::unimplemented();
    return {};
  }


  /** Partition the device into sub devices based upon the properties
      provided

      Return synchronous errors via SYCL exception classes.

      \todo To be implemented
  */
  vector_class<device>
  create_sub_devices(info::device_partition_type partitionType,
                     info::device_partition_property partitionProperty,
                     info::device_affinity_domain affinityDomain) const {
    detail::unimplemented();
    return {};
  }

};

/// @} to end the execution Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DEVICE_HPP
