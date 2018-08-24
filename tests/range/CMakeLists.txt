cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (range) # The name of our project

declare_trisycl_test(TARGET range TEST_REGEX
"Result:
1 
a = 3
b = 4
b = 2
c = 5,6
d = 7,8,9
 1 1 1
d = 1,2,1
d = 2,4,4
Range of dims 1
Range of dims 1
Range of dims 2
Range of dims 3
 1 2 3
 2 5 6
 1 4 16
 1 2 4
 0 2 4
 1 3 5
 1 1 1
0
1
0
1
 0 0 1
 0 1 1
 1 0 0
 1 1 0")
declare_trisycl_test(TARGET range_size)
