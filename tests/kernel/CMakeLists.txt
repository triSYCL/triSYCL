project(kernel) # The name of our project

declare_trisycl_test(TARGET functor)
declare_trisycl_test(TARGET functor_item)

if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET opencl_kernel USES_OPENCL CATCH2_WITH_MAIN)
  declare_trisycl_test(TARGET opencl_kernel_empty USES_OPENCL CATCH2_WITH_MAIN)
  declare_trisycl_test(TARGET opencl_kernel_empty_set_args USES_OPENCL
                       CATCH2_WITH_MAIN)
  declare_trisycl_test(TARGET opencl_kernel_single_task_vector_add_args_42
                       USES_OPENCL CATCH2_WITH_MAIN)
  declare_trisycl_test(TARGET opencl_kernel_vector_add USES_OPENCL
                       CATCH2_WITH_MAIN)
  declare_trisycl_test(TARGET opencl_kernel_vector_add_args USES_OPENCL
                       CATCH2_WITH_MAIN)
  declare_trisycl_test(TARGET opencl_kernel_vector_add_args_42 USES_OPENCL
                       CATCH2_WITH_MAIN)
endif(${TRISYCL_OPENCL})
