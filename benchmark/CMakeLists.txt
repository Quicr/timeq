# SPDX-FileCopyrightText: Copyright (c) 2026 Cisco Systems
# SPDX-License-Identifier: BSD-2-Clause

include(CPM)

set(BENCHMARK_USE_BUNDLED_GTEST OFF)
set(BENCHMARK_ENABLE_TESTING OFF)
CPMAddPackage("gh:google/benchmark@1.9.5")

add_executable(timeq_benchmark
    time_queue.cpp
)

set_target_properties(timeq_benchmark PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS ON
)

target_compile_options(timeq_benchmark PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall -Wno-error=unused-function>
)

target_link_libraries(timeq_benchmark PRIVATE timeq benchmark::benchmark_main)

add_test(NAME timeq_benchmark COMMAND timeq_benchmark)
