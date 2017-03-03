cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (jacobi) # The name of our project

declare_trisycl_test(TARGET jacobi2d-st-cplx-var)
declare_trisycl_test(TARGET jacobi2d-st-fxd)
declare_trisycl_test(TARGET jacobi2d-st-gen-var)
declare_trisycl_test(TARGET jacobi2d-st-var)
declare_trisycl_test(TARGET jacobi2d-tile)
declare_trisycl_test(TARGET jacobi2d)
