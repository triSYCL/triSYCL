cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (platform) # The name of our project

declare_trisycl_test(TARGET default_platform)
declare_trisycl_test(TARGET get_devices)
if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET opencl_platform USES_OPENCL)
endif(${TRISYCL_OPENCL})
