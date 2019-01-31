cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (parallel_for) # The name of our project

declare_trisycl_test(TARGET capture_scalars)
declare_trisycl_test(TARGET hierarchical_new)
declare_trisycl_test(TARGET hierarchical)
declare_trisycl_test(TARGET initializer_list)
declare_trisycl_test(TARGET item_no_offset)
declare_trisycl_test(TARGET item)
