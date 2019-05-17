#.rst:
# FindTriSYCL
#---------------
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.

#########################
#  FindTriSYCL.cmake
#########################
#
# Tools for finding and building with triSYCL.
#
# Requite CMake version 3.5 or higher

cmake_minimum_required (VERSION 3.5)
project(triSYCL CXX) # The name of the project (forward declare language)

#######################
#  set_target_cxx_std
#######################
#
#  Sets the specified target to support the required C++ standard level.
#
#  targetName : Name of the target to be set.
#  cxxStdYear : The year of the required C++ standard (e.g.: 17)
#
function(set_target_cxx_std targetName cxxStdYear)
  if(cxx_std_${cxxStdYear} IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set_property(TARGET ${targetName}
      PROPERTY
        INTERFACE_COMPILE_FEATURES cxx_std_${cxxStdYear})
  else()
    if(MSVC)
      set(cxxStdFlag  /std:c++${cxxStdYear})
    else()
      set(cxxStdFlag -std=c++${cxxStdYear})
    endif()
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(${cxxStdFlag} compilerHasCxxStdFlag)
    if(compilerHasCxxStdFlag)
      message(STATUS "CMake compile features not available for the current \
toolchain or CMake version, setting C++ standard level directly via compiler \
flags. Please be aware that setting flags doesn't check the actual C++${cxxStdYear} \
standard support provided by the underlying toolchain, e.g.: build may fail \
on unsupported features.")
      set_property(TARGET ${targetName}
        PROPERTY
          INTERFACE_COMPILE_OPTIONS ${cxxStdFlag})
    else()
      message(WARNING "Compiler seems to be unable to accept ${cxxStdFlag},
build will probably fail. Please set CMAKE_CXX_FLAGS to some sensible value for \
your toolchain.")
    endif()
  endif()
endfunction(set_target_cxx_std)

add_library(_trisycl_cxxfeatures INTERFACE)
add_library(triSYCL::cxxfeatures ALIAS _trisycl_cxxfeatures)

# Check that a supported host compiler can be found
if(CMAKE_COMPILER_IS_GNUCXX)
  # Require at least gcc 5.4
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.4)
    message(FATAL_ERROR
      "host compiler - Not found! (gcc version must be at least 5.4)")
  else()
    message(STATUS "host compiler - gcc ${CMAKE_CXX_COMPILER_VERSION}")

    set_target_cxx_std(_trisycl_cxxfeatures 17)
    target_compile_options(_trisycl_cxxfeatures
      INTERFACE
        # Turn on all warnings:
        -Wall
        -Wextra
        # Disable specific warnings:
        # warning: type qualifiers ignored on function return type
        -Wno-ignored-qualifiers
        # warning: comparison between signed and unsigned integer expressions
        -Wno-sign-compare
        # warning: ‘<OpenCL func>’ is deprecated
        -Wno-deprecated-declarations
        # warning: unused parameter ‘<id>’
        -Wno-unused-parameter
        # warning: ignoring attributes on template argument
        -Wno-ignored-attributes
    )
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # Require at least clang 3.9
  if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 3.9)
    message(FATAL_ERROR
      "host compiler - Not found! (clang version must be at least 3.9)")
  else()
    message(STATUS "host compiler - clang ${CMAKE_CXX_COMPILER_VERSION}")

    set_target_cxx_std(_trisycl_cxxfeatures 17)
    target_compile_options(_trisycl_cxxfeatures
      INTERFACE
        # Turn on all warnings
        -Wall
        -Wextra
        # Disable specific warnings:
        # warning: 'const' type qualifier on return type has no effect
        -Wno-ignored-qualifiers
        # warning: comparison between signed and unsigned integer expressions
        -Wno-sign-compare
        # warning: ‘<OpenCL func>’ is deprecated
        -Wno-deprecated-declarations
        # warning: unused parameter ‘<id>’
        -Wno-unused-parameter
        # warning: suggest braces around initialization of subobject
        -Wno-missing-braces
        # warning: unused variable '<id>'
        -Wno-unused-variable
        # warning: instantiation of variable '<templated id>' required here,
        # but no definition is available
        -Wno-undefined-var-template
    )
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # Change to /std:c++latest once Boost::funtional is fixed
  # (1.63.0 with toolset v141 not working)
  set_target_cxx_std(_trisycl_cxxfeatures 14)
  # Replace default Warning Level 3 with 4 (/Wall is pretty-much useless on MSVC
  # system headers are plagued with warnings)
  string(REGEX REPLACE "/W[0-9]" "/W4" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

  target_compile_options(_trisycl_cxxfeatures
      INTERFACE
        # Disabling (default) Warning Level 3 output
        # warning C4996: Call to '<algorithm name>' with parameters that may be
        # unsafe - this call relies on the caller to check that the passed values
        # are correct.
        /wd4996
        # warning C4267: '=': conversion from 'size_t' to 'int', possible loss of data
        /wd4267
        # warning C4244: '=': conversion from 'size_t' to 'double',
        # possible loss of data
        /wd4244
        # warning C4305: '<op>': truncation from 'double' to 'float'
        /wd4305
        # warning C4101: '<id>': unreferenced local variable
        /wd4101
        # warning C4700: uninitialized local variable '<id>' used
        /wd4700
        # warning C4189: '<id>': local variable is initialized but not referenced
        /wd4189
        # Disabling Warning Level 4 output
        # warning C4100: '<param>': unreferenced formal parameter
        /wd4100
        # warning C4459: declaration of '<id>' hides global declaration
        /wd4459
        # warning C4127: conditional expression is constant
        /wd4127
        # warning C4456: declaration of '<id>' hides previous local declaration
        /wd4456
    )
else()
  message(WARNING
    "host compiler - Not found! (triSYCL supports GCC, Clang and MSVC)")
endif()

#triSYCL options
option(TRISYCL_OPENMP "triSYCL multi-threading with OpenMP" ON)
option(TRISYCL_TBB "triSYCL multi-threading with TBB" OFF)
option(TRISYCL_OPENCL "triSYCL OpenCL interoperability mode" OFF)
option(TRISYCL_NO_ASYNC "triSYCL use synchronous kernel execution" OFF)
option(TRISYCL_DEBUG "triSYCL use debug mode" OFF)
option(TRISYCL_DEBUG_STRUCTORS "triSYCL trace of object lifetimes" OFF)
option(TRISYCL_TRACE_KERNEL "triSYCL trace of kernel execution" OFF)
option(TRISYCL_INCLUDE_DIR  "triSYCL include directory" OFF)

mark_as_advanced(TRISYCL_OPENMP)
mark_as_advanced(TRISYCL_TBB)
mark_as_advanced(TRISYCL_OPENCL)
mark_as_advanced(TRISYCL_NO_ASYNC)
mark_as_advanced(TRISYCL_DEBUG)
mark_as_advanced(TRISYCL_DEBUG_STRUCTORS)
mark_as_advanced(TRISYCL_TRACE_KERNEL)
mark_as_advanced(TRISYCL_INCLUDE_DIR)

#triSYCL definitions
set(CL_SYCL_LANGUAGE_VERSION 121 CACHE STRING VERSION
  "Host language version to be used by triSYCL (default is: 121)")
set(TRISYCL_CL_LANGUAGE_VERSION 121 CACHE STRING VERSION
  "Device language version to be used by triSYCL (default is: 121)")
set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

if(NOT TRISYCL_INCLUDE_DIR)
  set(TRISYCL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
endif()

if(EXISTS ${TRISYCL_INCLUDE_DIR})
  message(STATUS "Found triSYCL include directory: " ${TRISYCL_INCLUDE_DIR})
else()
  message(FATAL_ERROR "triSYCL include directory - no found! "
    ${TRISYCL_INCLUDE_DIR})
endif()


# Find OpenCL package
if(TRISYCL_OPENCL)
  find_package(OpenCL REQUIRED)
  if(UNIX)
    set(BOOST_COMPUTE_INCPATH /usr/include/compute CACHE PATH
      "Path to Boost.Compute headers (default is: /usr/include/compute)")
  endif(UNIX)
endif()

# Find OpenMP package
if(TRISYCL_OPENMP)
  find_package(OpenMP REQUIRED)
endif()

# Find TBB package
if(TRISYCL_TBB)
  find_package(TBB REQUIRED)
endif()

# Find Boost
set(BOOST_REQUIRED_COMPONENTS chrono log)
if(TRISYCL_OPENCL)
  list(APPEND BOOST_REQUIRED_COMPONENTS filesystem)
endif()
find_package(Boost 1.58 REQUIRED COMPONENTS ${BOOST_REQUIRED_COMPONENTS})

# If debug or trace we need boost log
if(TRISYCL_DEBUG OR TRISYCL_DEBUG_STRUCTORS OR TRISYCL_TRACE_KERNEL)
  set(LOG_NEEDED ON)
else()
  set(LOG_NEEDED OFF)
endif()


message(STATUS "triSYCL OpenMP:                   ${TRISYCL_OPENMP}")
message(STATUS "triSYCL TBB:                      ${TRISYCL_TBB}")
message(STATUS "triSYCL OpenCL:                   ${TRISYCL_OPENCL}")
message(STATUS "triSYCL synchronous execution:    ${TRISYCL_NO_ASYNC}")
message(STATUS "triSYCL debug mode:               ${TRISYCL_DEBUG}")
message(STATUS "triSYCL object trace:             ${TRISYCL_DEBUG_STRUCTORS}")
message(STATUS "triSYCL kernel trace:             ${TRISYCL_TRACE_KERNEL}")

find_package(Threads REQUIRED)

#######################
#  add_sycl_to_target
#######################
#
#  Sets the proper flags and includes for the target compilation.
#
#  targetName : Name of the target to add a SYCL to.
#
function(add_sycl_to_target targetName)
  # Add include directories to the "#include <>" paths
  target_include_directories (${targetName} PUBLIC
    ${TRISYCL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    $<$<BOOL:${TRISYCL_OPENCL}>:${OpenCL_INCLUDE_DIRS}>
    $<$<BOOL:${TRISYCL_OPENCL}>:${BOOST_COMPUTE_INCPATH}>)

  # Link dependencies
  target_link_libraries(${targetName} PUBLIC
    triSYCL::cxxfeatures
    $<$<BOOL:${TRISYCL_OPENCL}>:${OpenCL_LIBRARIES}>
    Threads::Threads
    $<$<BOOL:${LOG_NEEDED}>:Boost::log>
    Boost::chrono
    $<$<BOOL:${TRISYCL_OPENCL}>:Boost::filesystem>) #Required by BOOST_COMPUTE_USE_OFFLINE_CACHE.

  # Compile definitions
  target_compile_definitions(${targetName} PUBLIC
    $<$<BOOL:${TRISYCL_NO_ASYNC}>:TRISYCL_NO_ASYNC>
    $<$<BOOL:${TRISYCL_OPENCL}>:TRISYCL_OPENCL>
    $<$<BOOL:${TRISYCL_OPENCL}>:BOOST_COMPUTE_USE_OFFLINE_CACHE>
    $<$<BOOL:${TRISYCL_DEBUG}>:TRISYCL_DEBUG>
    $<$<BOOL:${TRISYCL_DEBUG_STRUCTORS}>:TRISYCL_DEBUG_STRUCTORS>
    $<$<BOOL:${TRISYCL_TRACE_KERNEL}>:TRISYCL_TRACE_KERNEL>
    $<$<BOOL:${LOG_NEEDED}>:BOOST_LOG_DYN_LINK>)

  # C++ and OpenMP requirements
  target_compile_options(${targetName} PUBLIC
    ${TRISYCL_COMPILE_OPTIONS}
    $<$<BOOL:${TRISYCL_OPENMP}>:${OpenMP_CXX_FLAGS}>)

  if(${TRISYCL_OPENMP})
    set_target_properties(${targetName}
      PROPERTIES
      LINK_FLAGS ${OpenMP_CXX_FLAGS})
  endif(${TRISYCL_OPENMP})

  # C++ and TBB requirements
  if(${TRISYCL_TBB})
    target_compile_definitions(${targetName} PUBLIC -DTRISYCL_TBB)

    target_include_directories(${targetName} PUBLIC ${TBB_INCLUDE_DIR})
    target_link_libraries(${targetName} PUBLIC ${TBB_LIBRARIES})
  endif(${TRISYCL_TBB})

endfunction(add_sycl_to_target)
