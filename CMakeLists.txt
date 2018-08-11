cmake_minimum_required (VERSION 3.5) # The minimum version of CMake necessary to build this project

set( TRISYCL_VERSION_MAJOR 0)
set( TRISYCL_VERSION_MINOR 1)
set( TRISYCL_VERSION_PATCH 0)
set( TRISYCL_VERSION "${TRISYCL_VERSION_MAJOR}.${TRISYCL_VERSION_MINOR}.${TRISYCL_VERSION_PATCH}")
project(triSYCL VERSION ${TRISYCL_VERSION} LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(FindTriSYCL)

# Enable CTest
include(CTest)
enable_testing()

include_directories(${PROJECT_SOURCE_DIR}/include) # All targets inherit SYCL include dir
include_directories(${PROJECT_SOURCE_DIR}/tests/common) # All targets inherit SYCL include dir


# Recurse into tests dir to pick up unit tests
add_subdirectory(tests)


add_library (${PROJECT_NAME} INTERFACE)

# Package definition

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)


set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")
set(CMAKEPACKAGE_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake")
set(PACKAGE_TRISYCL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include" )

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties (${PROJECT_NAME} PROPERTIES EXPORT_NAME ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKEPACKAGE_INSTALL_DIR}
)

write_basic_package_version_file(
    "${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${TRISYCL_VERSION}
    COMPATIBILITY SameMajorVersion
)


export (TARGETS ${PROJECT_NAME} NAMESPACE ${namespace} FILE ${PROJECT_NAME}Targets.cmake)
export (PACKAGE ${PROJECT_NAME})


install(EXPORT ${TARGETS_EXPORT_NAME} NAMESPACE ${namespace} DESTINATION ${CMAKEPACKAGE_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
          DESTINATION ${CMAKEPACKAGE_INSTALL_DIR} )

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)


# CPACK

set( CPACK_GENERATOR "TGZ;DEB" )
set( CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${TRISYCL_VERSION}-${CMAKE_HOST_SYSTEM_NAME}" )
set( CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-Source" )

set( CPACK_PACKAGE_CONTACT "${PROJECT_NAME} Maintainers")
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "SyCL reference implementation" )
set( CPACK_PACKAGE_NAME ${PROJECT_NAME})
set( CPACK_PACKAGE_VENDOR "Neutral" )
set( CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.rst")

set( CPACK_PACKAGE_VERSION_MAJOR ${TRISYCL_VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${TRISYCL_VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${TRISYCL_VERSION_PATCH} )

set( CPACK_DEBIAN_PACKAGE_DEPENDS "opencl-c-headers (>=1.2)")

include( CPack )
