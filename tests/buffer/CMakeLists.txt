project(buffer) # The name of our project

declare_trisycl_test(TARGET associative_containers CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET buffer_get_count CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET buffer_map_allocator CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET buffer_set_final_data CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET buffer_set_final_data_1 CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET buffer_shared_ptr CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET buffer_sizes CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET buffer_unique_ptr CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET buffer_write_order)
declare_trisycl_test(TARGET global_buffer TEST_REGEX "3 5 7 9 11 13")
declare_trisycl_test(TARGET global_buffer_host_access TEST_REGEX "1 2 3 4 5 6")
declare_trisycl_test(TARGET global_buffer_set_final_data CATCH2_WITH_MAIN)
declare_trisycl_test(TARGET read_write_buffer TEST_REGEX
"buffer \"a\" is read_only: 0
buffer \"b\" is read_only: 0
buffer \"c\" is read_only: 0
buffer \"cc\" is read_only: 1
buffer \"vb\" is read_only: 0")
declare_trisycl_test(TARGET shared_buffer TEST_REGEX
"buffer \"a\" use_count\\(\\) is: 20
buffer \"a\" is read_only: 0
buffer \"a\" use_count\\(\\) is: 20
buffer \"b\" use_count\\(\\) is: 20
buffer \"b\" is read_only: 0
buffer \"a\" use_count\\(\\) is: 20
buffer \"b\" use_count\\(\\) is: 20
buffer \"c\" use_count\\(\\) is: 20
buffer \"a\" use_count\\(\\) is: 20
buffer \"b\" use_count\\(\\) is: 20
buffer \"c\" use_count\\(\\) is: 20
buffer \"c\" is read_only: 0
buffer \"a\" use_count\\(\\) is: 20
buffer \"b\" use_count\\(\\) is: 20
buffer \"c\" use_count\\(\\) is: 20
buffer \"a\" use_count\\(\\) is: 20
buffer \"z\" use_count\\(\\) is: 20
buffer \"z\" is read_only: 0")
declare_trisycl_test(TARGET uninitialized_buffer CATCH2_WITH_MAIN)

if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET buffer_data_tracking USES_OPENCL TEST_REGEX
" 0 0 0
 3 4 5
 1 1 1")
endif(${TRISYCL_OPENCL})
