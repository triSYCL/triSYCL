cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (2014-04-21-HPC-GPU_Meetup) # The name of our project

declare_trisycl_test(TARGET slide_7 TEST_REGEX "1234")
declare_trisycl_test(TARGET slide_9-20 TEST_REGEX "1234")
declare_trisycl_test(TARGET slide_13 TEST_REGEX "= 3")
declare_trisycl_test(TARGET slide_14 TEST_REGEX "= 3")
declare_trisycl_test(TARGET slide_17-18 TEST_REGEX "1234")
declare_trisycl_test(TARGET slide_21 TEST_REGEX "1234")
declare_trisycl_test(TARGET slide_22 TEST_REGEX "= 10")
declare_trisycl_test(TARGET slide_23 TEST_REGEX "1234")
declare_trisycl_test(TARGET slide_26 TEST_REGEX "= 42")