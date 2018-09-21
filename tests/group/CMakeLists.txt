cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (group) # The name of our project

declare_trisycl_test(TARGET group TEST_REGEX
" 10
 2
 0
 5
5 
 10
10 
 2
2 
 0
0 
 3
3 
3
 100 10
 2 5
 0 0
 50 2
50 2 
 100 10
100 10 
 2 5
2 5 
 0 0
0 0 
 49 1
49 1 
99
 10 9 12
 2 3 6
 1 2 3
 5 3 2
5 3 2 
 10 9 12
10 9 12 
 2 3 6
2 3 6 
 1 2 3
1 2 3 
 4 2 1
4 2 1 
29
1
0
0
1
0
1")
