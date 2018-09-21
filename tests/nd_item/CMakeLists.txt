cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (nd_item) # The name of our project

declare_trisycl_test(TARGET nd_item TEST_REGEX
" 10
 5
 0
 10
 5
 0
 7
7 
7
 2
2 
2
 1
1 
1
 2
2 
 100 10
 2 5
 0 0
 100 10
 2 5
 0 0
 99 9
99 9 
999
 1 4
1 4 
9
 49 1
49 1 
99
 50 2
50 2 
 10 9 12
 2 3 6
 1 2 3
 10 9 12
 2 3 6
 1 2 3
 9 8 11
9 8 11 
788
 0 0 2
0 0 2 
12
 4 2 1
4 2 1 
29
 5 3 2
5 3 2 
1
0
0
1
1
0")
