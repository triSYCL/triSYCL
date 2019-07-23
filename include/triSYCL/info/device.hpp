#ifndef TRISYCL_SYCL_INFO_DEVICE_HPP
#define TRISYCL_SYCL_INFO_DEVICE_HPP

/** \file The OpenCL SYCL device information parameters

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/id.hpp"
#include "triSYCL/opencl_types.hpp"
#include "triSYCL/info/param_traits.hpp"

namespace trisycl {

class device;
class platform;

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

namespace info {

/** Type of devices

    To be used either to define a device type or to select more
    broadly a kind of device

    \todo To be moved in the specification from platform to device

    \todo Add opencl to the specification

    \todo there is no accelerator_selector and custom_accelerator
*/
enum class device_type : unsigned int {
  cpu,
  gpu,
  accelerator,
  custom,
  automatic,
  host,
  opencl,
  all
};


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
  preferred_vector_width_long,
  preferred_vector_width_float,
  preferred_vector_width_double,
  preferred_vector_width_half,
  native_vector_width_char,
  native_vector_width_short,
  native_vector_width_int,
  native_vector_width_long,
  native_vector_width_float,
  native_vector_width_double,
  native_vector_width_half,
  max_clock_frequency,
  address_bits,
  max_mem_alloc_size,
  image_support,
  max_read_image_args,
  max_write_image_args,
  image2d_max_width,
  image2d_max_height,
  image3d_max_width,
  image3d_max_height,
  image3d_max_depth,
  image_max_buffer_size,
  image_max_array_size,
  max_samplers,
  max_parameter_size,
  mem_base_addr_align,
  half_fp_config,
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
  is_endian_little,
  is_available,
  is_compiler_available,
  is_linker_available,
  execution_capabilities,
  queue_profiling,
  built_in_kernels,
  platform,
  name,
  vendor,
  driver_version,
  profile,
  version,
  opencl_c_version,
  extensions,
  printf_buffer_size,
  preferred_interop_user_sync,
  parent_device,
  partition_max_sub_devices,
  partition_properties,
  partition_affinity_domains,
  partition_type_property,
  partition_type_affinity_domain,
  reference_count
};

enum class partition_property : int {
  no_partition,
  partition_equally,
  partition_by_counts,
  partition_by_affinity_domain,
};

enum class partition_affinity_domain : int {
  not_applicable,
  numa,
  L4_cache,
  L3_cache,
  L2_cache,
  L1_cache,
  next_partitionable
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

enum class execution_capability : unsigned int {
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
TRISYCL_INFO_PARAM_TRAITS(info::device::device_type, info::device_type)
TRISYCL_INFO_PARAM_TRAITS(info::device::vendor_id, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::max_compute_units, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::max_work_item_dimensions, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::max_work_item_sizes, trisycl::id<3>)
TRISYCL_INFO_PARAM_TRAITS(info::device::max_work_group_size, std::size_t)

TRISYCL_INFO_PARAM_TRAITS(info::device::preferred_vector_width_char, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::preferred_vector_width_short, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::preferred_vector_width_int, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::preferred_vector_width_long, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::preferred_vector_width_float, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::preferred_vector_width_double, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::preferred_vector_width_half, trisycl::cl_uint)

TRISYCL_INFO_PARAM_TRAITS(info::device::native_vector_width_char, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::native_vector_width_short, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::native_vector_width_int, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::native_vector_width_long, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::native_vector_width_float, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::native_vector_width_double, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::native_vector_width_half, trisycl::cl_uint)

TRISYCL_INFO_PARAM_TRAITS(info::device::max_clock_frequency, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::address_bits, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::max_mem_alloc_size, trisycl::cl_ulong)
TRISYCL_INFO_PARAM_TRAITS(info::device::image_support, bool)

TRISYCL_INFO_PARAM_TRAITS(info::device::max_read_image_args, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::max_write_image_args, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::image2d_max_width, size_t);
TRISYCL_INFO_PARAM_TRAITS(info::device::image2d_max_height, size_t);
TRISYCL_INFO_PARAM_TRAITS(info::device::image3d_max_width, size_t);
TRISYCL_INFO_PARAM_TRAITS(info::device::image3d_max_height, size_t);
TRISYCL_INFO_PARAM_TRAITS(info::device::image3d_max_depth, size_t);
TRISYCL_INFO_PARAM_TRAITS(info::device::image_max_buffer_size, size_t);
TRISYCL_INFO_PARAM_TRAITS(info::device::image_max_array_size, size_t);
TRISYCL_INFO_PARAM_TRAITS(info::device::max_samplers, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::max_parameter_size, size_t)
TRISYCL_INFO_PARAM_TRAITS(info::device::mem_base_addr_align, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::half_fp_config, vector_class<fp_config>)
TRISYCL_INFO_PARAM_TRAITS(info::device::single_fp_config, vector_class<fp_config>)
TRISYCL_INFO_PARAM_TRAITS(info::device::double_fp_config, vector_class<fp_config>)
TRISYCL_INFO_PARAM_TRAITS(info::device::global_mem_cache_type, global_mem_cache_type)
TRISYCL_INFO_PARAM_TRAITS(info::device::global_mem_cache_line_size, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::global_mem_cache_size, trisycl::cl_ulong)
TRISYCL_INFO_PARAM_TRAITS(info::device::global_mem_size, trisycl::cl_ulong)
TRISYCL_INFO_PARAM_TRAITS(info::device::max_constant_buffer_size, trisycl::cl_ulong)
TRISYCL_INFO_PARAM_TRAITS(info::device::max_constant_args, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::local_mem_type, local_mem_type)
TRISYCL_INFO_PARAM_TRAITS(info::device::local_mem_size, trisycl::cl_ulong)
TRISYCL_INFO_PARAM_TRAITS(info::device::error_correction_support, bool)
TRISYCL_INFO_PARAM_TRAITS(info::device::host_unified_memory, bool)
TRISYCL_INFO_PARAM_TRAITS(info::device::profiling_timer_resolution, size_t)
TRISYCL_INFO_PARAM_TRAITS(info::device::is_endian_little, bool)
TRISYCL_INFO_PARAM_TRAITS(info::device::is_available, bool)
TRISYCL_INFO_PARAM_TRAITS(info::device::is_compiler_available, bool)
TRISYCL_INFO_PARAM_TRAITS(info::device::is_linker_available, bool)
TRISYCL_INFO_PARAM_TRAITS(info::device::execution_capabilities, vector_class<execution_capability>)
TRISYCL_INFO_PARAM_TRAITS(info::device::queue_profiling, bool)
TRISYCL_INFO_PARAM_TRAITS(info::device::built_in_kernels, vector_class<string_class>)
TRISYCL_INFO_PARAM_TRAITS(info::device::platform, trisycl::platform)
TRISYCL_INFO_PARAM_TRAITS(info::device::name, string_class)
TRISYCL_INFO_PARAM_TRAITS(info::device::vendor, string_class)
TRISYCL_INFO_PARAM_TRAITS(info::device::driver_version, string_class)
TRISYCL_INFO_PARAM_TRAITS(info::device::profile, string_class)
TRISYCL_INFO_PARAM_TRAITS(info::device::version, string_class)
TRISYCL_INFO_PARAM_TRAITS(info::device::opencl_c_version, string_class)
TRISYCL_INFO_PARAM_TRAITS(info::device::extensions, vector_class<string_class>)
TRISYCL_INFO_PARAM_TRAITS(info::device::printf_buffer_size, size_t)
TRISYCL_INFO_PARAM_TRAITS(info::device::preferred_interop_user_sync, bool)
TRISYCL_INFO_PARAM_TRAITS(info::device::parent_device, trisycl::device)
TRISYCL_INFO_PARAM_TRAITS(info::device::partition_max_sub_devices, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::device::partition_properties, vector_class<partition_property>)
TRISYCL_INFO_PARAM_TRAITS(info::device::partition_affinity_domains, vector_class<partition_affinity_domain>)
TRISYCL_INFO_PARAM_TRAITS(info::device::partition_type_property, partition_property)
TRISYCL_INFO_PARAM_TRAITS(info::device::partition_type_affinity_domain, partition_affinity_domain)
TRISYCL_INFO_PARAM_TRAITS(info::device::reference_count, trisycl::cl_uint)
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_INFO_DEVICE_HPP
