project(math) # The name of our project

declare_trisycl_test(TARGET math CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET vector_math CATCH2_WITH_MAIN)

if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET opencl_type USES_OPENCL  CATCH2_WITH_MAIN
                       TEST_REGEX "x: 0, y: 1, z: 2")
endif(${TRISYCL_OPENCL})
