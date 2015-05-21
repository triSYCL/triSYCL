set(GROUP_SOURCES
    group.cpp )

set(GROUP_HEADERS
    ${triSYCL_HEADERS})
	
include_directories(
    ${triSYCL_SOURCE_DIR}/include)

if( MSVC )
  set(CMAKE_CXX_FLAGS " -W3 /EHsc" )
elseif( CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS "-Wall -std=c++0x" )
elseif( CMAKE_CXX_COMPILER_GROUP STREQUAL "Clang" )
  set(CMAKE_CXX_FLAGS "-Wall -std=c++11" )
endif()


add_executable(group ${GROUP_SOURCES} ${ND_GROUP_HEADERS})
