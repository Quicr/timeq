# SPDX-FileCopyrightText: Copyright (c) 2026 Cisco Systems
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.13)

project(timeq
    VERSION 1.0.0
    DESCRIPTION "Header-only C++20 library for time-bucketed aging FIFO queues with TTL expiration"
    LANGUAGES C CXX
)

configure_file(cmake/version.h.in ${CMAKE_BINARY_DIR}/include/timeq/version.h)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#=============================================================================#
# Options
#=============================================================================#

include(CMakeDependentOption)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(timeq_IS_TOP_LEVEL ON)
endif()

cmake_dependent_option(timeq_BUILD_TESTS "Build tests for timeq" ON timeq_IS_TOP_LEVEL OFF)
cmake_dependent_option(timeq_BUILD_BENCHMARKS "Build benchmarks for timeq" ON timeq_IS_TOP_LEVEL OFF)

#=============================================================================#
# Build the library
#=============================================================================#

add_library(timeq INTERFACE)

set_target_properties(timeq PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF
)

target_include_directories(timeq INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

#=============================================================================#
# Testing
#=============================================================================#

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(BUILD_TESTING)
    if(timeq_BUILD_TESTS)
        add_subdirectory(tests)
    endif()

    if(timeq_BUILD_BENCHMARKS)
        add_subdirectory(benchmark)
    endif()
endif()

#=============================================================================#
# Install
#=============================================================================#

install(TARGETS timeq EXPORT timeq_targets)

install(DIRECTORY include/ DESTINATION include)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/timeq/version.h" DESTINATION include/timeq)
