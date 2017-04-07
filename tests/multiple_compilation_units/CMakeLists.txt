cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (multiple_compilation_units) # The name of our project

#declare_trisycl_test(TARGET parallel_for
#                     SOURCES parallel_for.cc parallel_for_other.C
#                     FORCE_SYNCRONOUS_EXEC
#                     TEST_REGEX
#"Group id = 0
#Local id = 0 \\(global id = 0\\)
#Local id = 1 \\(global id = 1\\)
#Group id = 1
#Local id = 0 \\(global id = 2\\)
#Local id = 1 \\(global id = 3\\)
#Group id = 2
#Local id = 0 \\(global id = 4\\)
#Local id = 1 \\(global id = 5\\)
#Group id = 3
#Local id = 0 \\(global id = 6\\)
#Local id = 1 \\(global id = 7\\)
#Group id = 4
#Local id = 0 \\(global id = 8\\)
#Local id = 1 \\(global id = 9\\)
#6 8 11")
