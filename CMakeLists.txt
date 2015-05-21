cmake_minimum_required(VERSION 2.6)
project(triSYCL)

set(Boost_INCLUDE_DIR ${triSYCL_SOURCE_DIR}/boost/boost_1_58_0/ CACHE PATH "Boost include dir")

include_directories(${Boost_INCLUDE_DIR}) 

add_subdirectory(include)
add_subdirectory(tests)
