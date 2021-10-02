project(device) # The name of our project

declare_trisycl_test(TARGET default_device CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET get_info CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET type CATCH2_WITH_MAIN)

if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET opencl_device USES_OPENCL CATCH2_WITH_MAIN)
endif(${TRISYCL_OPENCL})
