cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (kernel) # The name of our project

declare_trisycl_test(TARGET functor_item)
declare_trisycl_test(TARGET functor)
if(${BUILD_OPENCL})
  declare_trisycl_test(TARGET opencl_kernel_empty_set_args USES_OPENCL)
  declare_trisycl_test(TARGET opencl_kernel_empty USES_OPENCL)
  declare_trisycl_test(TARGET opencl_kernel_vector_add_args_42 USES_OPENCL)
  declare_trisycl_test(TARGET opencl_kernel_vector_add_args USES_OPENCL)
  declare_trisycl_test(TARGET opencl_kernel_vector_add USES_OPENCL)
  declare_trisycl_test(TARGET opencl_kernel USES_OPENCL)
endif(${BUILD_OPENCL})