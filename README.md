# timeq

[![CMake](https://github.com/Quicr/timeq/actions/workflows/cmake.yml/badge.svg)](https://github.com/Quicr/timeq/actions/workflows/cmake.yml)

A time-based queue where the length of the queue is a duration, divided into buckets
based on a given time interval. As time progresses, buckets in the past are cleared,
and the main queue is updated so that the front only returns a valid object that has
not expired. To improve performance, buckets are only cleared on push or pop
operations. Buckets in the past can therefore be cleared in bulk based on how many
intervals have elapsed since the last update.

`timeq` is a header-only C++20 library. The public API lives under `include/timeq/`.

## Requirements

- CMake 3.13 or newer
- A C++20 compiler

When built as the top-level project, test and benchmark dependencies (GoogleTest and
Google Benchmark) are fetched automatically via [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake).

## Build

Configure a build directory:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

Compile:

```bash
cmake --build build -j
```

### CMake options

| Option | Default (top-level) | Description |
| --- | --- | --- |
| `BUILD_TESTING` | `ON` | Enable CTest integration |
| `timeq_BUILD_TESTS` | `ON` | Build the GoogleTest suite |
| `timeq_BUILD_BENCHMARKS` | `ON` | Build the Google Benchmark executable |

To configure without tests or benchmarks:

```bash
cmake -B build -DBUILD_TESTING=OFF
```

Or disable them individually:

```bash
cmake -B build -Dtimeq_BUILD_TESTS=OFF -Dtimeq_BUILD_BENCHMARKS=OFF
```

> [!NOTE]
> When `timeq` is added as a subdirectory of another CMake project, tests and
> benchmarks are disabled by default.

### Install

```bash
cmake --install build --prefix /path/to/install
```

This installs the headers and CMake target for use from other projects.

## Testing

Tests are built when `BUILD_TESTING` and `timeq_BUILD_TESTS` are enabled (the default
when configuring this repository directly).

Run the full suite with CTest:

```bash
ctest --test-dir build --output-on-failure
```

Or run the test binary directly:

```bash
./build/tests/timeq_test
```

For a single test case:

```bash
./build/tests/timeq_test --gtest_filter=time_queue.PushAndExpire
```

## Benchmarks

Benchmarks are built when `BUILD_TESTING` and `timeq_BUILD_BENCHMARKS` are enabled.
Use a `Release` build for meaningful results.

Run all benchmarks:

```bash
./build/benchmark/timeq_benchmark
```

Run a subset by name filter:

```bash
./build/benchmark/timeq_benchmark --benchmark_filter=BM_TimeQueue_Push
```

Useful Google Benchmark flags:

```bash
# Repeat each benchmark and report statistics
./build/benchmark/timeq_benchmark --benchmark_repetitions=5

# Set a minimum run time (seconds) per benchmark
./build/benchmark/timeq_benchmark --benchmark_min_time=1
```

The benchmark suite covers construction, push, pop, pop-front, and mixed
push/pop workloads at different queue sizes and bucket intervals.
