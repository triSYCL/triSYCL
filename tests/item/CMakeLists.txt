cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (item) # The name of our project

declare_trisycl_test(TARGET item TEST_REGEX
" 7
 3
 2
1
 10 11 12
 2 3 6
 1 2 3
341
 100 10
 2 3
 0 0
302
1
0
0
1
 0")
