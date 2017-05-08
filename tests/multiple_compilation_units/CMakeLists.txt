cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (multiple_compilation_units) # The name of our project

declare_trisycl_test(TARGET parallel_for
                     SOURCES parallel_for.cc parallel_for_other.C)
