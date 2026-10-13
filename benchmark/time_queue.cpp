// SPDX-FileCopyrightText: Copyright (c) 2023 Cisco Systems
// SPDX-License-Identifier: BSD-2-Clause

#include "time_queue_bench.hpp"

#include <benchmark/benchmark.h>

#include <timeq/fast_time_queue.h>
#include <timeq/time_queue.h>

struct TrivialType
{};

static_assert(std::is_trivially_copyable_v<TrivialType>);

struct NonTrivialType
{
    NonTrivialType() {}
    ~NonTrivialType() {}
};

static_assert(!std::is_trivially_copyable_v<NonTrivialType>);

using namespace timeq;

BENCHMARK(Construct<time_queue<TrivialType>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(Construct<time_queue<NonTrivialType>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(Construct<fast_time_queue<TrivialType>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(Construct<fast_time_queue<NonTrivialType>>)->Arg(300)->Arg(1'000'000);

BENCHMARK(Push<time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Push<time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Push<fast_time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Push<fast_time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(FirstPushAfterIdle<time_queue<TrivialType>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(FirstPushAfterIdle<time_queue<NonTrivialType>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(FirstPushAfterIdle<fast_time_queue<TrivialType>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(FirstPushAfterIdle<fast_time_queue<NonTrivialType>>)->Arg(300)->Arg(1'000'000);

BENCHMARK(Pop<time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Pop<time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Pop<fast_time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Pop<fast_time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(Front<time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Front<time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Front<fast_time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Front<fast_time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(PopFront<time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(PopFront<time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(PopFront<fast_time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(PopFront<fast_time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(Size<time_queue<TrivialType>>)->Iterations(kIterations);
BENCHMARK(Size<time_queue<NonTrivialType>>)->Iterations(kIterations);
BENCHMARK(Size<fast_time_queue<TrivialType>>)->Iterations(kIterations);
BENCHMARK(Size<fast_time_queue<NonTrivialType>>)->Iterations(kIterations);

BENCHMARK(Empty<time_queue<TrivialType>>)->Iterations(kIterations);
BENCHMARK(Empty<time_queue<NonTrivialType>>)->Iterations(kIterations);
BENCHMARK(Empty<fast_time_queue<TrivialType>>)->Iterations(kIterations);
BENCHMARK(Empty<fast_time_queue<NonTrivialType>>)->Iterations(kIterations);

BENCHMARK(PushAndPopLoaded<time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPopLoaded<time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPopLoaded<fast_time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPopLoaded<fast_time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);

BENCHMARK(PushAndPop_Interval_1ms<time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_1ms<time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_1ms<fast_time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_1ms<fast_time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);

BENCHMARK(PushAndPop_Interval_125ms<time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_125ms<time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_125ms<fast_time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_125ms<fast_time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);
