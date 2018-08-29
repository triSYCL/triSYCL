cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (nd_range) # The name of our project

declare_trisycl_test(TARGET nd_range TEST_REGEX
" 6 4 8
 3 2 4
 0 0 0
 6 4 8
 3 2 4
 0 0 0
 2 2 2
 2 2 2
 4
 2
 3
 10 20
 2 5
 0 0
 1
 6 4
 6 5 7
1
0
0
1")

