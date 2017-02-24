cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (examples) # The name of our project

declare_trisycl_test(TARGET demo_parallel_matrices_add)
declare_trisycl_test(TARGET demo_parallel_matrix_add)
declare_trisycl_test(TARGET parallel_matrix_add)
declare_trisycl_test(TARGET parallel_vector_add)
declare_trisycl_test(TARGET simpler_parallel_matrix_add)
declare_trisycl_test(TARGET vector_add)