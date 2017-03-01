cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (single_task) # The name of our project

declare_trisycl_test(TARGET single_task TEST_REGEX "1234")
