set(DEMO_PARALLEL_MATRIX_ADD_SOURCES
    demo_parallel_matrix_add.cpp )

set(PARALLEL_MATRIX_ADD_SOURCES
    parallel_matrix_add.cpp )

set(PARALLEL_VECTOR_ADD_SOURCES
    parallel_vector_add.cpp )

set(VECTOR_ADD_SOURCES
    vector_add.cpp )


set(EXAMPLE_HEADERS
    ${triSYCL_HEADERS})

include_directories(
    ${triSYCL_SOURCE_DIR}/include)

if( MSVC )
  set(CMAKE_CXX_FLAGS " -W3 /EHsc" )
elseif( CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS "-Wall -std=c++1y" )
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
  set(CMAKE_CXX_FLAGS "-Wall -std=c++1y" )
endif()

add_executable(demo_parallel_matrix_add ${DEMO_PARALLEL_MATRIX_ADD_SOURCES} ${EXAMPLE_HEADERS})
add_executable(parallel_matrix_add ${PARALLEL_MATRIX_ADD_SOURCES} ${EXAMPLE_HEADERS})
add_executable(parallel_vector_add ${PARALLEL_VECTOR_ADD_SOURCES} ${EXAMPLE_HEADERS})
add_executable(vector_add ${VECTOR_ADD_SOURCES} ${EXAMPLE_HEADERS})
