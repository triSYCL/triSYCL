project(platform) # The name of our project

declare_trisycl_test(TARGET default_platform CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET get_devices CATCH2_WITH_MAIN)

if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET opencl_platform USES_OPENCL CATCH2_WITH_MAIN)
endif(${TRISYCL_OPENCL})
