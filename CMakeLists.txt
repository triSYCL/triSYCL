cmake_minimum_required (VERSION 3.5) # The minimum version of CMake necessary to build this project
project(triSYCL C CXX) # The name of the project (forward declare language )

#
# Behavioural options for the solution
#
  option(TRISYCL_OPENMP "triSYCL multi-threading with OpenMP" ON)
  option(TRISYCL_NO_ASYNC "triSYCL use synchronous kernel execution" OFF)
  option(BUILD_OPENCL "triSYCL build OpenCL tests" ON)
  option(BUILD_XILINX "triSYCL build Xilinx-specific tests" OFF)
  option(TRISYCL_DEBUG "triSCYL use debug mode" OFF)
  option(TRISYCL_DEBUG_STRUCTORS "triSYCL trace of object lifetimes" OFF)

  set(CL_SYCL_LANGUAGE_VERSION 220 CACHE VERSION "Host language version to be used by trisYCL (default is: 220)")
  set(TRISYCL_CL_LANGUAGE_VERSION 220 CACHE VERSION "Device language version to be used by trisYCL (default is: 220) (not used yet)")

#
# Platform/compiler-independant setup
#
  set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Use solution folders in IDEs

  # Find dependencies
  if(TRISYCL_OPENMP)
    find_package(OpenMP REQUIRED)
  endif(TRISYCL_OPENMP)

  if(BUILD_OPENCL)
    find_package(OpenCL REQUIRED)
  endif(BUILD_OPENCL)

  find_package(Boost 1.58 REQUIRED COMPONENTS chrono log)

  # Enable CTest
  include(CTest)
  enable_testing()

  include_directories(${PROJECT_SOURCE_DIR}/include) # All targets inherit SYCL include dir
  include_directories(${PROJECT_SOURCE_DIR}/tests/common) # All targets inherit SYCL include dir

#
# Platform-specific setup
#
  if(UNIX) # If compiling under Linux
    set(BOOST_COMPUTE_INCPATH /usr/include/compute CACHE PATH "Path to Boost.Compute headers (default is: /usr/include/compute)")
    find_package(Threads REQUIRED)
  endif(UNIX)

  if(APPLE) # If compiling under OSX
    find_package(Threads REQUIRED)
  endif(APPLE)

  if(WIN32)
    if(CMAKE_VERSION MATCHES "MSVC")
      message(STATUS "Visual Studio IDE integration detected.")
	  message(STATUS "Disabling CMake's automatic reconfiguration in favor of IDE detection.")
	  set(CMAKE_SUPPRESS_REGENERATION ON CACHE BOOL "Disables pre-build rule of checking script timestamps")
	endif(CMAKE_VERSION MATCHES "MSVC")
  endif(WIN32)

#
# Set compiler-specific variables
#
  # If compiling with MSVC++
  if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options("/std:c++14") # Change to /std:c++latest once Boost::funtional is fixed (1.63.0 with toolset v141 not working)
    string(REGEX REPLACE "/W[0-9]" "/W4" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}) # Replace default Warning Level 3 with 4 (/Wall is pretty-much useless on MSVC, system headers are plagued with warnings)

    # Disabling (default) Warning Level 3 output
    add_compile_options("/wd4996") # warning C4996: Call to '<algorithm name>' with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct.
    add_compile_options("/wd4267") # warning C4267: '=': conversion from 'size_t' to 'int', possible loss of data
    add_compile_options("/wd4244") # warning C4244: '=': conversion from 'size_t' to 'double', possible loss of data
    add_compile_options("/wd4305") # warning C4305: '<op>': truncation from 'double' to 'float'
    #add_compile_options("/wd4101") # warning C4101: '<id>': unreferenced local variable
    #add_compile_options("/wd4700") # warning C4700: uninitialized local variable '<id>' used
    #add_compile_options("/wd4189") # warning C4189: '<id>': local variable is initialized but not referenced

    # Disabling Warning Level 4 output
    #add_compile_options("/wd4100") # warning C4100: '<param>': unreferenced formal parameter
    #add_compile_options("/wd4459") # warning C4459: declaration of '<id>' hides global declaration
    #add_compile_options("/wd4127") # warning C4127: conditional expression is constant
    #add_compile_options("/wd4456") # warning C4456: declaration of '<id>' hides previous local declaration
  endif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

  # If compiling with g++
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options("-std=c++1z") # Use latest available C++ standard
    add_compile_options("-Wall")   # Turn on all warnings
    add_compile_options("-Wextra") # Turn on all warnings

    # Disabling specific warnings
    add_compile_options("-Wno-ignored-attributes") # warning: ignoring attributes on template argument
    add_compile_options("-Wno-sign-compare") # warning: comparison between signed and unsigned integer expressions
    add_compile_options("-Wno-deprecated-declarations") # warning: ‘<OpenCL func>’ is deprecated
    add_compile_options("-Wno-ignored-qualifiers") # warning: type qualifiers ignored on function return type
    add_compile_options("-Wno-unused-parameter") # warning: unused parameter ‘<id>’
  endif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")

  # If compiling with clang
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options("-std=c++1z") # Use latest available C++ standard
    add_compile_options("-Wall")   # Turn on all warnings
    add_compile_options("-Wextra") # Turn on all warnings

    # Disabling specific warnings
    add_compile_options("-Wno-ignored-qualifiers") # warning: 'const' type qualifier on return type has no effect
    add_compile_options("-Wno-sign-compare") # warning: comparison between signed and unsigned integer expressions
    add_compile_options("-Wno-deprecated-declarations") # warning: ‘<OpenCL func>’ is deprecated
    add_compile_options("-Wno-unused-parameter") # warning: unused parameter ‘<id>’
    add_compile_options("-Wno-missing-braces") # warning: suggest braces around initialization of subobject
    add_compile_options("-Wno-unused-variable") # warning: unused variable '<id>'
    add_compile_options("-Wno-undefined-var-template") # warning: instantiation of variable '<templated id>' required here, but no definition is available
  endif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")

# Recurse into tests dir to pick up unit tests
add_subdirectory(tests)
