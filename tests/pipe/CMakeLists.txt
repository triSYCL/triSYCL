cmake_minimum_required (VERSION 3.0) # The minimum version of CMake necessary to build this project
project (pipe) # The name of our project

declare_trisycl_test(TARGET trisycl_iostream_pipe TEST_REGEX
"salut !
hello 42
from inside the kernel... it works too!
")
declare_trisycl_test(TARGET producer_consumer)
