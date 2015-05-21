set(PARALLEL_FOR_HIERARCHICAL_SOURCES
    hierarchical.cpp )

set(PARALLEL_FOR_INITIALIZER_LIST_SOURCES
    initializer_list.cpp )

set(PARALLEL_FOR_INTEGER_SOURCES
    integer.cpp )

set(PARALLEL_FOR_ITEM_SOURCES
    item.cpp )

set(VECTOR_HEADERS
    ${triSYCL_HEADERS})
	
include_directories(
    ${triSYCL_SOURCE_DIR}/include)

if( MSVC )
  set(CMAKE_CXX_FLAGS " -W3 /EHsc" )
elseif( CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS "-Wall -std=c++0x" )
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
  set(CMAKE_CXX_FLAGS "-Wall -std=c++11" )
endif()


add_executable(parallel_for_hierarchical ${PARALLEL_FOR_HIERARCHICAL_SOURCES} ${VECTOR_HEADERS})
add_executable(parallel_for_initializer_list ${PARALLEL_FOR_INITIALIZER_LIST_SOURCES} ${VECTOR_HEADERS})
add_executable(parallel_for_integer ${PARALLEL_FOR_INTEGER_SOURCES} ${VECTOR_HEADERS})
add_executable(parallel_for_item ${PARALLEL_FOR_ITEM_SOURCES} ${VECTOR_HEADERS})
