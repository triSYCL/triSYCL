include(CMakeParseArguments) # Helper macro to define new functions

function(declare_trisycl_test)

  # Setup argument parsing
  set(options USES_OPENCL FORCE_SYNCRONOUS_EXEC XILINX_SPECIFIC)
  set(oneValueArgs TARGET)
  set(multiValueArgs HEADERS SOURCES KERNELS)

  # Parse function arguments
  cmake_parse_arguments(declare_trisycl_test "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

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

  # Specify executable sources
  add_executable (${TARGET_NAME} ${Files_BUILD})

  # Add include directories to the "#include <>" paths
  target_include_directories (${TARGET_NAME} PUBLIC $<$<BOOL:USES_OPENCL>:${OpenCL_INCLUDE_DIRS}>
                                                    $<$<BOOL:UNIX>:${BOOST_COMPUTE_INCPATH}>)

  # Link dependencies
  target_link_libraries (${TARGET_NAME} PUBLIC $<$<BOOL:USES_OPENCL>:${OpenCL_LIBRARIES}> # Link to OpenCL if example requires
                                               $<$<NOT:$<BOOL:WIN32>>:Threads::Threads> # If not on Windows, use some threading model
                                               #$<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:USE_OPENMP>>:gomp> # g++ requires libgomp to use OpenMP
                                               $<$<BOOL:WIN32>:Boost::disable_autolinking> # Boost can auto-link when used with MSVC, but relies on relative paths which CMake does not really like
                                               Boost::chrono)

  # Compile definitions
  target_compile_definitions(${TARGET_NAME} PUBLIC TRISYCL_ASYNC $<$<BOOL:USES_OPENCL>:TRISYCL_OPENCL>)

  # Add unit test
  add_test(NAME "${PROJECT_NAME}/${declare_trisycl_test_TARGET}"
           COMMAND ${TARGET_NAME}
           WORKING_DIRECTORY ${PROJECT_NAME})

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