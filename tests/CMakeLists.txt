# SPDX-FileCopyrightText: Copyright (c) 2026 Cisco Systems
# SPDX-License-Identifier: BSD-2-Clause

include(CPM)
include(GoogleTest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
CPMAddPackage("gh:google/googletest@1.17.0")

add_executable(timeq_test
    time_queue.cpp
)

set_target_properties(timeq_test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS ON
)

target_link_libraries(timeq_test PRIVATE timeq GTest::gtest_main)

target_compile_options(timeq_test PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
)

gtest_discover_tests(timeq_test)
