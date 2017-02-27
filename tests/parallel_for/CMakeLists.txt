cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (parallel_for) # The name of our project

declare_trisycl_test(TARGET hierarchical_new FORCE_SYNCRONOUS_EXEC TEST_REGEX
"Group id = 0
Local id = 0 \\(global id = 0\\)
Local id = 1 \\(global id = 1\\)
Global id = 0
Global id = 1
Group id = 1
Local id = 0 \\(global id = 2\\)
Local id = 1 \\(global id = 3\\)
Global id = 2
Global id = 3
Group id = 2
Local id = 0 \\(global id = 4\\)
Local id = 1 \\(global id = 5\\)
Global id = 4
Global id = 5
Group id = 3
Local id = 0 \\(global id = 6\\)
Local id = 1 \\(global id = 7\\)
Global id = 6
Global id = 7
Group id = 4
Local id = 0 \\(global id = 8\\)
Local id = 1 \\(global id = 9\\)
Global id = 8
Global id = 9")
declare_trisycl_test(TARGET hierarchical FORCE_SYNCRONOUS_EXEC TEST_REGEX
"Group id = 0
Local id = 0 \\(global id = 0\\)
Local id = 1 \\(global id = 1\\)
Group id = 1
Local id = 0 \\(global id = 2\\)
Local id = 1 \\(global id = 3\\)
Group id = 2
Local id = 0 \\(global id = 4\\)
Local id = 1 \\(global id = 5\\)
Group id = 3
Local id = 0 \\(global id = 6\\)
Local id = 1 \\(global id = 7\\)
Group id = 4
Local id = 0 \\(global id = 8\\)
Local id = 1 \\(global id = 9\\)")
declare_trisycl_test(TARGET initializer_list)
declare_trisycl_test(TARGET integer)
declare_trisycl_test(TARGET item_no_offset)
declare_trisycl_test(TARGET item)