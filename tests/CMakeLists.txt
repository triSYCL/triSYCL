include(CMakeParseArguments) # Helper macro to define new functions

function(declare_trisycl_test)

  # Setup argument parsing
  set(options USES_OPENCL FORCE_SYNCRONOUS_EXEC XILINX_SPECIFIC)
  set(oneValueArgs TARGET TEST_REGEX)
  set(multiValueArgs HEADERS SOURCES KERNELS)

  # Parse function arguments
  cmake_parse_arguments(declare_trisycl_test "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Declare source code files
  set (Files_HDRS ${declare_trisycl_test_HEADERS})

  # If sources are defined, use them; otherwise default to TARGET.cpp
  if("${declare_trisycl_test_SOURCES}" STREQUAL "")
    set (Files_SRCS "${PROJECT_SOURCE_DIR}/${declare_trisycl_test_TARGET}.cpp")
  else("${declare_trisycl_test_SOURCES}" STREQUAL "")
    set (Files_SRCS ${declare_trisycl_test_SOURCES})
  endif("${declare_trisycl_test_SOURCES}" STREQUAL "")

  set_source_files_properties(${Files_SRCS} PROPERTIES LANGUAGE CXX ) # Needed because of sloppy file naming in multiple_compilation_units test

  # Variable to hold ALL files to build and be visible in IDE
  set (Files_BUILD ${Files_HDRS} ${Files_SRCS})

  # Create filters for IDEs
  source_group ("Headers" FILES ${Files_HDRS})
  source_group ("Sources" FILES ${Files_SRCS})

  set(TARGET_NAME "${PROJECT_NAME}_${declare_trisycl_test_TARGET}") # Disambiguates sources with identical names from different directories
  set(TEST_NAME "${PROJECT_NAME}/${declare_trisycl_test_TARGET}") # Save for multiple uses later

  # Specify executable sources
  add_executable (${TARGET_NAME} ${Files_BUILD})

  # Add include directories to the "#include <>" paths
  target_include_directories (${TARGET_NAME} PUBLIC #$<$<BOOL:${declare_trisycl_test_USES_OPENCL}>:${OpenCL_INCLUDE_DIRS}> # include OpenCL headers if example uses it
                                                    ${OpenCL_INCLUDE_DIRS}
                                                    $<$<BOOL:${UNIX}>:${BOOST_COMPUTE_INCPATH}>
                                                    ${Boost_INCLUDE_DIRS})

  # Link dependencies
  target_link_libraries (${TARGET_NAME} PUBLIC #$<$<BOOL:${declare_trisycl_test_USES_OPENCL}>:${OpenCL_LIBRARIES}> # Link to OpenCL if example requires
                                               ${OpenCL_LIBRARIES}
                                               $<$<NOT:$<BOOL:${WIN32}>>:Threads::Threads> # If not on Windows, use some threading model
                                               $<$<BOOL:${WIN32}>:Boost::disable_autolinking> # Boost can auto-link when used with MSVC, but relies on relative paths which CMake does not really like
                                               $<$<OR:$<BOOL:${TRISYCL_DEBUG}>,$<BOOL:${TRISYCL_DEBUG_STRUCTORS}>>:Boost::log> # Link to Boost.log when triSYCL in debug mode
                                               Boost::chrono)

  # Compile definitions
  target_compile_definitions(${TARGET_NAME} PUBLIC $<$<BOOL:${TRISYCL_NO_ASYNC}>:TRISYCL_NO_ASYNC> # If opting for synchronous task exec
                                                   $<$<BOOL:${declare_trisycl_test_USES_OPENCL}>:TRISYCL_OPENCL> # If the test uses OpenCL
                                                   $<$<BOOL:${TRISYCL_DEBUG}>:TRISYCL_DEBUG>
                                                   $<$<BOOL:${TRISYCL_DEBUG_STRUCTORS}>:TRISYCL_DEBUG_STRUCTORS>
												   $<$<BOOL:${WIN32}>:BOOST_MULTI_ARRAY_NO_GENERATORS>)

  # OpenMP requirements
  target_compile_options(${TARGET_NAME} PUBLIC $<$<BOOL:${TRISYCL_OPENMP}>:${OpenMP_CXX_FLAGS}>)

  if(${TRISYCL_OPENMP} AND (NOT WIN32))
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS ${OpenMP_CXX_FLAGS}) # Does not support generator expressions
  endif(${TRISYCL_OPENMP} AND (NOT WIN32))

  # Add unit test
  add_test(NAME ${TEST_NAME}
           COMMAND ${TARGET_NAME}
           WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

  # If test uses regular expression
  if(NOT ${declare_trisycl_test_TEST_REGEX} STREQUAL "")
    set_tests_properties (${TEST_NAME} PROPERTIES PASS_REGULAR_EXPRESSION ${declare_trisycl_test_TEST_REGEX})
  endif(NOT ${declare_trisycl_test_TEST_REGEX} STREQUAL "")

endfunction(declare_trisycl_test)

# Recurse into tests dir to pick up unit tests
add_subdirectory(2014-04-21-HPC-GPU_Meetup)
add_subdirectory(accessor)
add_subdirectory(address_spaces)
add_subdirectory(buffer)
add_subdirectory(detail)
add_subdirectory(device)
add_subdirectory(device_selector)
add_subdirectory(examples)
add_subdirectory(group)
add_subdirectory(id)
add_subdirectory(item)
add_subdirectory(jacobi)
add_subdirectory(kernel)
add_subdirectory(math)
add_subdirectory(multiple_compilation_units)
add_subdirectory(nd_item)
add_subdirectory(nd_range)
add_subdirectory(parallel_for)
add_subdirectory(pipe)
add_subdirectory(platform)
add_subdirectory(queue)
add_subdirectory(range)
add_subdirectory(single_task)
#add_subdirectory(SDAccel)
add_subdirectory(vector)
