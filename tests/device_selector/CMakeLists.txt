cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (device_selector) # The name of our project

declare_trisycl_test(TARGET explicit_selector)
declare_trisycl_test(TARGET selector)
if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET opencl_selector USES_OPENCL)
endif(${TRISYCL_OPENCL})
