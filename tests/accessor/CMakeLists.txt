project(accessor) # The name of our project

declare_trisycl_test(TARGET accessor)
declare_trisycl_test(TARGET accessor_sizes CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET demo_parallel_matrix_add)
declare_trisycl_test(TARGET host_accessor)
declare_trisycl_test(TARGET iterators CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET local_accessor_hierarchical_convolution
                     CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET uninitialized_local CATCH2_WITH_MAIN)
