project(queue) # The name of our project

declare_trisycl_test(TARGET default_queue CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET double_wait)
declare_trisycl_test(TARGET explicit_selector CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET queue)
declare_trisycl_test(TARGET wait TEST_REGEX
"First
Second")

if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET opencl_queue USES_OPENCL CATCH2_WITH_MAIN)
endif(${TRISYCL_OPENCL})
