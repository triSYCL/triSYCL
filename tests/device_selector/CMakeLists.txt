project(device_selector) # The name of our project

declare_trisycl_test(TARGET explicit_selector CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET selector CATCH2_WITH_MAIN)

if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET opencl_selector USES_OPENCL CATCH2_WITH_MAIN)
endif(${TRISYCL_OPENCL})
