cmake_minimum_required(VERSION 2.6)
project(triSYCL-tests)

include_directories(
    ${triSYCL_SOURCE_DIR}/tests/common/)

add_subdirectory(examples)

add_subdirectory(address_spaces)
add_subdirectory(buffer)
add_subdirectory(id)
add_subdirectory(item)
add_subdirectory(nd_item)
add_subdirectory(nd_range)
add_subdirectory(parallel_for)
add_subdirectory(queue)
add_subdirectory(range)
add_subdirectory(single_task)
add_subdirectory(vector)