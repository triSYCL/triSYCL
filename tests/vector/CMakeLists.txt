cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (vector) # The name of our project

declare_trisycl_test(TARGET operators)
declare_trisycl_test(TARGET vec)
declare_trisycl_test(TARGET vecas)
declare_trisycl_test(TARGET vecacc)
declare_trisycl_test(TARGET cl_types)
