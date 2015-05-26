set(READ_WRITE_BUFFER_SOURCES
    read_write_buffer.cpp )

set(SHARED_BUFFER_SOURCES
    shared_buffer.cpp )


set(GROUP_HEADERS
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


add_executable(read_write_buffer ${READ_WRITE_BUFFER_SOURCES} ${ND_GROUP_HEADERS})
add_executable(shared_buffer ${SHARED_BUFFER_SOURCES} ${ND_GROUP_HEADERS})
