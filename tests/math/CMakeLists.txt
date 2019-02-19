cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (math) # The name of our project

declare_trisycl_test(TARGET math)
declare_trisycl_test(TARGET vector_math)
if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET opencl_type USES_OPENCL TEST_REGEX "x: 0, y: 1, z: 2")
endif(${TRISYCL_OPENCL})
