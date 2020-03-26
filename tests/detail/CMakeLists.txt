cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (detail) # The name of our project

declare_trisycl_test(TARGET fiber_pool)
declare_trisycl_test(TARGET small_array)
