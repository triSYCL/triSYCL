set(SINGLE_TASK_SOURCES
    single_task.cpp )

set(SINGLE_TASK_HEADERS
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


add_executable(single_task ${SINGLE_TASK_SOURCES} ${SINGLE_TASK_HEADERS})
