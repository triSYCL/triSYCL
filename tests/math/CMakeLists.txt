cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (math) # The name of our project

declare_trisycl_test(TARGET math)
if(${BUILD_OPENCL})
  declare_trisycl_test(TARGET opencl_type USES_OPENCL TEST_REGEX "x: 0, y: 1, z: 2")
endif(${BUILD_OPENCL})
