cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (accessor) # The name of our project

declare_trisycl_test(TARGET accessor)
declare_trisycl_test(TARGET accessor_sizes)
declare_trisycl_test(TARGET demo_parallel_matrix_add)
declare_trisycl_test(TARGET iterators)
declare_trisycl_test(TARGET local_accessor_hierarchical_convolution)
declare_trisycl_test(TARGET uninitialized_local)
