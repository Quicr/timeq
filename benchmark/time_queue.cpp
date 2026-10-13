#include "time_queue_bench.hpp"

#include <benchmark/benchmark.h>

#include <timeq/flat_time_queue.h>
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
BENCHMARK(Construct<flat_time_queue<TrivialType>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(Construct<flat_time_queue<NonTrivialType>>)->Arg(300)->Arg(1'000'000);

BENCHMARK(Push<time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Push<time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Push<flat_time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Push<flat_time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(FirstPushAfterIdle<time_queue<TrivialType>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(FirstPushAfterIdle<time_queue<NonTrivialType>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(FirstPushAfterIdle<flat_time_queue<TrivialType>>)->Arg(300)->Arg(1'000'000);
BENCHMARK(FirstPushAfterIdle<flat_time_queue<NonTrivialType>>)->Arg(300)->Arg(1'000'000);

BENCHMARK(Pop<time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Pop<time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Pop<flat_time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Pop<flat_time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(Front<time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Front<time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Front<flat_time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(Front<flat_time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(PopFront<time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(PopFront<time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(PopFront<flat_time_queue<TrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(PopFront<flat_time_queue<NonTrivialType>>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);

BENCHMARK(Size<time_queue<TrivialType>>)->Iterations(kIterations);
BENCHMARK(Size<time_queue<NonTrivialType>>)->Iterations(kIterations);
BENCHMARK(Size<flat_time_queue<TrivialType>>)->Iterations(kIterations);
BENCHMARK(Size<flat_time_queue<NonTrivialType>>)->Iterations(kIterations);

BENCHMARK(Empty<time_queue<TrivialType>>)->Iterations(kIterations);
BENCHMARK(Empty<time_queue<NonTrivialType>>)->Iterations(kIterations);
BENCHMARK(Empty<flat_time_queue<TrivialType>>)->Iterations(kIterations);
BENCHMARK(Empty<flat_time_queue<NonTrivialType>>)->Iterations(kIterations);

BENCHMARK(PushAndPopLoaded<time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPopLoaded<time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPopLoaded<flat_time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPopLoaded<flat_time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);

BENCHMARK(PushAndPop_Interval_1ms<time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_1ms<time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_1ms<flat_time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_1ms<flat_time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);

BENCHMARK(PushAndPop_Interval_125ms<time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_125ms<time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_125ms<flat_time_queue<TrivialType>>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(PushAndPop_Interval_125ms<flat_time_queue<NonTrivialType>>)->Arg(5000)->Arg(1'000'000);
