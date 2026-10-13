#include "time_queue_bench.hpp"

#include <benchmark/benchmark.h>

#include <timeq/flat_time_queue.h>
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
BENCHMARK(Construct<flat_time_queue<MockPayloadData>>)->Arg(300)->Arg(1'000'000);

BENCHMARK(Push<time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Push<flat_time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(FirstPushAfterIdle<time_queue<MockPayloadData>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(FirstPushAfterIdle<flat_time_queue<MockPayloadData>>)->Arg(300)->Arg(1'000'000);

BENCHMARK(Pop<time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Pop<flat_time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(Front<time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Front<flat_time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(PopFront<time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(PopFront<flat_time_queue<MockPayloadData>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(Size<time_queue<MockPayloadData>>)->Iterations(kIterations);
BENCHMARK(Size<flat_time_queue<MockPayloadData>>)->Iterations(kIterations);

BENCHMARK(Empty<time_queue<MockPayloadData>>)->Iterations(kIterations);
BENCHMARK(Empty<flat_time_queue<MockPayloadData>>)->Iterations(kIterations);

BENCHMARK(PushAndPopLoaded<time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPopLoaded<flat_time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);

BENCHMARK(PushAndPop_Interval_1ms<time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_1ms<flat_time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);

BENCHMARK(PushAndPop_Interval_125ms<time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_125ms<flat_time_queue<MockPayloadData>>)->Arg(5000)->Arg(1'000'000);
