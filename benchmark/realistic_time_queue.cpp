// SPDX-FileCopyrightText: Copyright (c) 2023 Cisco Systems
// SPDX-License-Identifier: BSD-2-Clause

#include "time_queue_bench.hpp"

#include <benchmark/benchmark.h>

#include <timeq/fast_time_queue.h>
#include <timeq/time_queue.h>

struct MockPayloadData
{
    std::uint64_t id;
    std::uint8_t priority;
    std::uint8_t action;
    std::uint64_t tick_microseconds;

    std::shared_ptr<const std::vector<uint8_t>> data = std::make_shared<const std::vector<std::uint8_t>>(1000);
};

using namespace timeq;

BENCHMARK(Construct<time_queue<MockPayloadData>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(Construct<fast_time_queue<MockPayloadData>>)->Arg(300)->Arg(1'000'000);

BENCHMARK(Push<time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Push<fast_time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(FirstPushAfterIdle<time_queue<MockPayloadData>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(FirstPushAfterIdle<fast_time_queue<MockPayloadData>>)->Arg(300)->Arg(1'000'000);

BENCHMARK(Pop<time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Pop<fast_time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(Front<time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Front<fast_time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(PopFront<time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(PopFront<fast_time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(Size<time_queue<MockPayloadData>>)->Iterations(kIterations);
BENCHMARK(Size<fast_time_queue<MockPayloadData>>)->Iterations(kIterations);

BENCHMARK(Empty<time_queue<MockPayloadData>>)->Iterations(kIterations);
BENCHMARK(Empty<fast_time_queue<MockPayloadData>>)->Iterations(kIterations);

BENCHMARK(PushAndPopLoaded<time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPopLoaded<fast_time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);

BENCHMARK(PushAndPop_Interval_1ms<time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_1ms<fast_time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);

BENCHMARK(PushAndPop_Interval_125ms<time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_125ms<fast_time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);
