set(RANGE_SOURCES
    range.cpp )

set(RANGE_HEADERS
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


add_executable(range ${RANGE_SOURCES} ${RANGE_HEADERS})
