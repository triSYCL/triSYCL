#ifndef TRISYCL_SYCL_DEVICE_DETAIL_OPENCL_DEVICE_TAIL_HPP
#define TRISYCL_SYCL_DEVICE_DETAIL_OPENCL_DEVICE_TAIL_HPP

namespace trisycl::detail {

#define TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(name, result) \
  case info::device::name : return result;

#define TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(name, field)         \
  case info::device::name : return d.get_info<CL_DEVICE_##field>();

inline std::any opencl_device::get_info(info::device param) const {
  switch (param) {
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(
  device_type,
  d.get_info<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_CPU
  ? info::device_type::cpu : info::device_type::gpu);
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(vendor_id, VENDOR_ID)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(max_compute_units, MAX_COMPUTE_UNITS )
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(max_work_item_dimensions,
                                       MAX_WORK_ITEM_DIMENSIONS)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(
  max_work_item_sizes,
  (::trisycl::id<3> { d.get_info<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]
                    , d.get_info<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[1]
                    , d.get_info<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[2] }))
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(max_work_group_size, MAX_WORK_GROUP_SIZE)

TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(preferred_vector_width_char,
                                       PREFERRED_VECTOR_WIDTH_CHAR)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(preferred_vector_width_short,
                                       PREFERRED_VECTOR_WIDTH_SHORT)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(preferred_vector_width_int,
                                       PREFERRED_VECTOR_WIDTH_INT)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(preferred_vector_width_long,
                                       PREFERRED_VECTOR_WIDTH_LONG)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(preferred_vector_width_float,
                                       PREFERRED_VECTOR_WIDTH_FLOAT)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(preferred_vector_width_double,
                                       PREFERRED_VECTOR_WIDTH_DOUBLE)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(preferred_vector_width_half,
                                    ::trisycl::cl_uint { 0 })

TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(native_vector_width_char,
                                       NATIVE_VECTOR_WIDTH_CHAR)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(native_vector_width_short,
                                       NATIVE_VECTOR_WIDTH_SHORT)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(native_vector_width_int,
                                       NATIVE_VECTOR_WIDTH_INT)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(native_vector_width_long,
                                       NATIVE_VECTOR_WIDTH_LONG)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(native_vector_width_float,
                                       NATIVE_VECTOR_WIDTH_FLOAT)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(native_vector_width_double,
                                       NATIVE_VECTOR_WIDTH_DOUBLE)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(native_vector_width_half,
                                    ::trisycl::cl_uint { 0 })

TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(max_clock_frequency, MAX_CLOCK_FREQUENCY)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(address_bits, ADDRESS_BITS)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(max_mem_alloc_size, MAX_MEM_ALLOC_SIZE)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(image_support, IMAGE_SUPPORT )
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(max_read_image_args, MAX_READ_IMAGE_ARGS)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(max_write_image_args,
                                       MAX_WRITE_IMAGE_ARGS)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(image2d_max_width, IMAGE2D_MAX_WIDTH)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(image2d_max_height, IMAGE2D_MAX_HEIGHT)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(image3d_max_width, IMAGE3D_MAX_WIDTH)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(image3d_max_height, IMAGE3D_MAX_HEIGHT)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(image3d_max_depth, IMAGE3D_MAX_DEPTH)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(image_max_buffer_size, size_t { 0 })
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(image_max_array_size, size_t { 0 })
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(max_samplers, MAX_SAMPLERS)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(max_parameter_size, MAX_PARAMETER_SIZE)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(mem_base_addr_align, MEM_BASE_ADDR_ALIGN)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(half_fp_config,
                                    vector_class<info::fp_config>{} )
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(single_fp_config,
                                    vector_class<info::fp_config>{} )
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(double_fp_config,
                                    vector_class<info::fp_config>{} )
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(global_mem_cache_type,
                                    info::global_mem_cache_type::none )
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(
  global_mem_cache_line_size,
  ::trisycl::cl_uint { static_cast<::trisycl::cl_uint>
      (d.get_info<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>()) })
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(global_mem_cache_size,
                                       GLOBAL_MEM_CACHE_SIZE)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(global_mem_size, GLOBAL_MEM_SIZE)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(max_constant_buffer_size,
                                    ::trisycl::cl_ulong { 0 } )
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(max_constant_args,
                                    ::trisycl::cl_uint { 0 } )
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(local_mem_type, info::local_mem_type::local)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(local_mem_size, LOCAL_MEM_SIZE)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(error_correction_support,
                                       ERROR_CORRECTION_SUPPORT)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(host_unified_memory,
                                       HOST_UNIFIED_MEMORY)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(profiling_timer_resolution,
                                       PROFILING_TIMER_RESOLUTION)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(is_endian_little, ENDIAN_LITTLE)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(is_available, AVAILABLE)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(is_compiler_available,
                                       COMPILER_AVAILABLE)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(is_linker_available, false)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(execution_capabilities,
                                    vector_class<info::execution_capability>{} )
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(queue_profiling, false)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(built_in_kernels,
                                    vector_class<string_class>{} )
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(platform, ::trisycl::platform{} )
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(name, NAME)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(vendor, VENDOR)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(driver_version,
                                    d.get_info<CL_DRIVER_VERSION>())
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(profile, PROFILE)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(version, VERSION)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(opencl_c_version, OPENCL_C_VERSION )
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(extensions, vector_class<string_class>{} )
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(printf_buffer_size, PRINTF_BUFFER_SIZE)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(preferred_interop_user_sync,
                                       PREFERRED_INTEROP_USER_SYNC)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(parent_device, ::trisycl::device{})
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(partition_max_sub_devices,
                                    ::trisycl::cl_uint { 0 })
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(partition_properties,
                                    vector_class<info::partition_property>{})
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(
  partition_affinity_domains,
  vector_class<info::partition_affinity_domain>{})
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(partition_type_property,
                                    info::partition_property::no_partition)
TRISYCL_DEFINE_DEVICE_INFO_TEMPLATE(
  partition_type_affinity_domain,
  info::partition_affinity_domain::not_applicable)
TRISYCL_DEFINE_DEVICE_INFO_CL_TEMPLATE(reference_count, REFERENCE_COUNT)
  default:
    return 0;
  }
}

}

#endif
