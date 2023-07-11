# Create a function to declare a triSYCL unit test using Catch2
# \todo Describe the API
function(declare_trisycl_test)
  # Setup argument parsing for this function.
  # Declare possible option keywords (not taking any argument).
  # Not clear whether the following options are actually used except
  # CATCH2_WITH_MAIN which asks Catch2 to provide the main()
  set(options CATCH2_WITH_MAIN FORCE_SYNCHRONOUS_EXEC GUI
      USES_OPENCL XILINX_SPECIFIC)
  # Declare keywords taking 1 argument
  set(oneValueArgs TARGET TEST_REGEX)
  # Declare keywords taking 1 or more arguments
  set(multiValueArgs HEADERS SOURCES KERNELS)

  # Parse the function arguments by populating variables with prefix
  # "declare_trisycl_test_"
  cmake_parse_arguments(declare_trisycl_test
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN})

  # Handle cmake_parse_arguments() issues
  if(DEFINED declare_trisycl_test_UNPARSED_ARGUMENTS)
    message(SEND_ERROR
      "declare_trisycl_test is used with the following spurious arguments:"
      "${declare_trisycl_test_UNPARSED_ARGUMENTS}"
      )
  endif(DEFINED declare_trisycl_test_UNPARSED_ARGUMENTS)

  if(DEFINED declare_trisycl_test_KEYWORDS_MISSING_VALUES)
    message(SEND_ERROR
      "declare_trisycl_test is used with missing arguments for:"
      "${declare_trisycl_test_KEYWORDS_MISSING_VALUES}"
      )
  endif(DEFINED declare_trisycl_test_KEYWORDS_MISSING_VALUES)

  # Declare source code files
  set(Files_HDRS ${declare_trisycl_test_HEADERS})

  # If sources are defined, use them; otherwise use TARGET.cpp as default
  if(DEFINED declare_trisycl_test_SOURCES)
    set(Files_SRCS ${declare_trisycl_test_SOURCES})
  else(DEFINED declare_trisycl_test_SOURCES)
    set(Files_SRCS "${PROJECT_SOURCE_DIR}/${declare_trisycl_test_TARGET}.cpp")
  endif(DEFINED declare_trisycl_test_SOURCES)

  # Needed because of sloppy file naming in multiple_compilation_units test
  set_source_files_properties(${Files_SRCS} PROPERTIES LANGUAGE CXX )

  # Variable to hold ALL files to build and be visible in IDE
  set(Files_BUILD ${Files_HDRS} ${Files_SRCS})

  # Create filters for IDEs
  source_group("Headers" FILES ${Files_HDRS})
  source_group("Sources" FILES ${Files_SRCS})

  # Disambiguates sources with identical names from different directories
  set(TARGET_NAME "${PROJECT_NAME}_${declare_trisycl_test_TARGET}")
  # Save for multiple uses later
  set(TEST_NAME "${PROJECT_NAME}/${declare_trisycl_test_TARGET}")

  # Specify executable sources
  add_executable(${TARGET_NAME} ${Files_BUILD})

  add_sycl_to_target(${TARGET_NAME})

  # Use Catch2
  if(declare_trisycl_test_CATCH2_WITH_MAIN)
    # A test defined with option CATCH2_WITH_MAIN requires Catch2 to
    # provide its own main() function
    target_link_libraries(${TARGET_NAME} PRIVATE Catch2::Catch2WithMain)
  else(declare_trisycl_test_CATCH2_WITH_MAIN)
    # Otherwise just provide Catch2 and the user needs to declare the
    # main() function in the test
    target_link_libraries(${TARGET_NAME} PRIVATE Catch2::Catch2)
  endif(declare_trisycl_test_CATCH2_WITH_MAIN)

  # Add the unit test
  add_test(NAME ${TEST_NAME}
           COMMAND ${TARGET_NAME}
           WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

  # If test uses regular expression
  if(DEFINED declare_trisycl_test_TEST_REGEX)
    set_tests_properties(${TEST_NAME}
      PROPERTIES PASS_REGULAR_EXPRESSION ${declare_trisycl_test_TEST_REGEX})
  endif(DEFINED declare_trisycl_test_TEST_REGEX)

  # If the GUI keyword is used, passed it to ctest so we can enable or
  # skip tests using the graphical user interface or not
  if(declare_trisycl_test_GUI)
    set_tests_properties(${TEST_NAME} PROPERTIES LABELS GUI)
  endif(declare_trisycl_test_GUI)

endfunction(declare_trisycl_test)

# Recurse into tests dir to pick up unit tests
add_subdirectory(2014-04-21-HPC-GPU_Meetup)
add_subdirectory(acap)
add_subdirectory(accessor)
add_subdirectory(address_spaces)
add_subdirectory(aie-axi-stream)
add_subdirectory(array_partition)
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
add_subdirectory(marray)
add_subdirectory(math)
add_subdirectory(multiple_compilation_units)
add_subdirectory(nd_item)
add_subdirectory(nd_range)
add_subdirectory(parallel_for)
add_subdirectory(pipe)
add_subdirectory(platform)
add_subdirectory(queue)
add_subdirectory(random)
add_subdirectory(range)
add_subdirectory(scope)
#add_subdirectory(SDAccel)
add_subdirectory(single_task)
add_subdirectory(sycl_2_2_pipe)
add_subdirectory(sycl_namespace)
add_subdirectory(vector)
