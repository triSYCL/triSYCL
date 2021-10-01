project(parallel_for) # The name of our project

declare_trisycl_test(TARGET capture_scalars)
declare_trisycl_test(TARGET hierarchical_new CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET hierarchical CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET initializer_list)
declare_trisycl_test(TARGET item_no_offset)
declare_trisycl_test(TARGET item)
