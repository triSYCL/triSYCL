cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (device) # The name of our project

declare_trisycl_test(TARGET default_device)
declare_trisycl_test(TARGET get_info)
declare_trisycl_test(TARGET type)
if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET opencl_device USES_OPENCL)
endif(${TRISYCL_OPENCL})
