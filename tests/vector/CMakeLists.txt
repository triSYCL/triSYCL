cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (vector) # The name of our project

declare_trisycl_test(TARGET operators)
declare_trisycl_test(TARGET vec)
declare_trisycl_test(TARGET vec_structured_binding)
declare_trisycl_test(TARGET vecalign)
declare_trisycl_test(TARGET vecas)
declare_trisycl_test(TARGET vecconvert)
declare_trisycl_test(TARGET vecacc)
declare_trisycl_test(TARGET cl_types)
declare_trisycl_test(TARGET vecswiz)
declare_trisycl_test(TARGET veclohiswiz)
declare_trisycl_test(TARGET vecmemlayoutalign)
