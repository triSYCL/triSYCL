cmake_minimum_required (VERSION 3.5) # The minimum version of CMake necessary to build this project

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(FindTriSYCL)

# Enable CTest
include(CTest)
enable_testing()

include_directories(${PROJECT_SOURCE_DIR}/include) # All targets inherit SYCL include dir
include_directories(${PROJECT_SOURCE_DIR}/tests/common) # All targets inherit SYCL include dir


# Recurse into tests dir to pick up unit tests
add_subdirectory(tests)
