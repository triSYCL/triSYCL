cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (id) # The name of our project

declare_trisycl_test(TARGET id TEST_REGEX
"Result:
1
5 7
3
jj has changed
 0 1
 1 0
 0 1
 1 1
jj via e = 5
jj via e = 3
cjj via e = 5
cjj via e = 3
 6 5
 4 4
 0 1
 12 15
 6 2
 1 2 3
 5 6
zeroid = 0, 0, 0")
declare_trisycl_test(TARGET id_any_dimension CATCH2_WITH_MAIN TEST_REGEX
" 1 2 3 4
 2 4 7 12
 1 2 3 4 5
 7 8 9 10 11 12")
