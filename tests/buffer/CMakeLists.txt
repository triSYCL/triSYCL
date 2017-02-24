cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (buffer) # The name of our project

declare_trisycl_test(TARGET associative_containers)
declare_trisycl_test(TARGET buffer_get_count)
#declare_trisycl_test(TARGET buffer_map_allocator)
declare_trisycl_test(TARGET buffer_set_final_data_1)
declare_trisycl_test(TARGET buffer_set_final_data)
declare_trisycl_test(TARGET buffer_shared_ptr)
declare_trisycl_test(TARGET buffer_sizes)
declare_trisycl_test(TARGET buffer_unique_ptr)
declare_trisycl_test(TARGET global_buffer_host_access)
declare_trisycl_test(TARGET global_buffer_set_final_data)
declare_trisycl_test(TARGET global_buffer)
declare_trisycl_test(TARGET read_write_buffer)
declare_trisycl_test(TARGET shared_buffer)