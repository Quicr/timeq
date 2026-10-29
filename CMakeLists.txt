# SPDX-FileCopyrightText: Copyright (c) 2026 Cisco Systems
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.13)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" timeq_VERSION)
project(timeq
    VERSION "${timeq_VERSION}"
    DESCRIPTION "Header-only C++20 library for time-bucketed aging FIFO queues with TTL expiration"
    LANGUAGES C CXX
)

include(GNUInstallDirs)

configure_file(cmake/version.h.in ${PROJECT_BINARY_DIR}/include/timeq/version.h)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#=============================================================================#
# Options
#=============================================================================#

include(CMakeDependentOption)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(timeq_STANDALONE ON)
endif()

cmake_dependent_option(TIMEQ_BUILD_TESTS "Build tests for timeq" ON timeq_STANDALONE OFF)
cmake_dependent_option(TIMEQ_BUILD_BENCHMARKS "Build benchmarks for timeq" ON timeq_STANDALONE OFF)
option(TIMEQ_INSTALL "Enable install rules for timeq" ${timeq_STANDALONE})

#=============================================================================#
# Build the library
#=============================================================================#

add_library(timeq INTERFACE)
add_library(timeq::timeq ALIAS timeq)

set_target_properties(timeq PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF
)

target_compile_features(timeq INTERFACE cxx_std_20)

target_include_directories(timeq INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

#=============================================================================#
# Testing
#=============================================================================#

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(BUILD_TESTING)
    if(TIMEQ_BUILD_TESTS)
        add_subdirectory(tests)
    endif()

    if(TIMEQ_BUILD_BENCHMARKS)
        add_subdirectory(benchmark)
    endif()
endif()

#=============================================================================#
# Install
#=============================================================================#

if (TIMEQ_INSTALL)
    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/timeqConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/timeqConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/timeqConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/timeq"
    )

    install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/timeq/version.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/timeq")

    install(TARGETS timeq
        EXPORT timeqTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(EXPORT timeqTargets
        FILE timeqTargets.cmake
        NAMESPACE timeq::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/timeq
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/timeqConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/timeqConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/timeq
    )
endif()
