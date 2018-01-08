cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (buffer) # The name of our project

declare_trisycl_test(TARGET associative_containers)
declare_trisycl_test(TARGET buffer_get_count)
declare_trisycl_test(TARGET buffer_map_allocator)
declare_trisycl_test(TARGET buffer_set_final_data)
declare_trisycl_test(TARGET buffer_set_final_data_1)
declare_trisycl_test(TARGET buffer_shared_ptr)
declare_trisycl_test(TARGET buffer_sizes)
declare_trisycl_test(TARGET buffer_unique_ptr)
declare_trisycl_test(TARGET buffer_write_order)
declare_trisycl_test(TARGET global_buffer TEST_REGEX "3 5 7 9 11 13")
declare_trisycl_test(TARGET global_buffer_host_access TEST_REGEX "1 2 3 4 5 6")
declare_trisycl_test(TARGET global_buffer_set_final_data)
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
declare_trisycl_test(TARGET uninitialized_buffer)
if(${TRISYCL_OPENCL})
  declare_trisycl_test(TARGET buffer_data_tracking USES_OPENCL TEST_REGEX
" 0 0 0
 3 4 5
 1 1 1")
endif(${TRISYCL_OPENCL})
