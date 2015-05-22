set(PARALLEL_FOR_ITEM_SEPARATE_SOURCE_SOURCES
    parallel_for_item_separate_source_host.cpp
	parallel_for_item_separate_source_device.cpp )

set(PARALLEL_FOR_ITEM_SEPARATE_SOURCE_HEADERS
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


add_executable(parallel_for_item_separate_source ${PARALLEL_FOR_ITEM_SEPARATE_SOURCE_SOURCES} ${PARALLEL_FOR_ITEM_SEPARATE_SOURCE_HEADERS})
