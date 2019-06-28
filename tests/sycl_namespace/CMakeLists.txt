cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (sycl_namespace) # The name of our project

declare_trisycl_test(TARGET demo_parallel_matrix_add)
