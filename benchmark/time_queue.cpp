#include <benchmark/benchmark.h>

#include <timeq/fast_time_queue.h>
#include <timeq/time_queue.h>

#include <type_traits>

static auto service = std::make_shared<timeq::threaded_tick_service>();

constexpr size_t kIterations = 100'000'000;

template<class TimeQueue>
static void
Construct(benchmark::State& state)
{
    const std::size_t duration = state.range(0);
    for (auto _ : state) {
        auto tq = TimeQueue(duration, 1, service);

        std::size_t size = tq.size();
        benchmark::DoNotOptimize(size);
        benchmark::ClobberMemory();
    }
}

template<class TimeQueue>
static void
Push(benchmark::State& state)
{
    TimeQueue tq(state.range(0), 1, service, kIterations);
    int64_t items_count = 0;

    for (auto _ : state) {
        ++items_count;
        tq.push({}, 20);
    }

    state.SetItemsProcessed(items_count);
}

template<class TimeQueue>
static void
Pop(benchmark::State& state)
{
    TimeQueue tq(state.range(0), 1, service, kIterations);
    for (size_t i = 0; i < kIterations; ++i) {
        tq.push({}, 10);
    }

    int64_t items_count = 0;
    for (auto _ : state) {
        ++items_count;
        tq.pop();
    }

    state.SetItemsProcessed(items_count);
}

template<class TimeQueue>
static void
Front(benchmark::State& state)
{
    TimeQueue tq(state.range(0), 1, service, kIterations);
    for (size_t i = 0; i < kIterations; ++i) {
        tq.push({}, 15);
    }

    int64_t items_count = 0;
    for (auto _ : state) {
        ++items_count;
        auto value = tq.front();
        benchmark::DoNotOptimize(value);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(items_count);
}

template<class TimeQueue>
static void
PopFront(benchmark::State& state)
{
    TimeQueue tq(state.range(0), 1, service, kIterations);
    for (size_t i = 0; i < kIterations; ++i) {
        tq.push({}, 15);
    }

    int64_t items_count = 0;
    for (auto _ : state) {
        ++items_count;
        auto value = tq.pop_front();
        benchmark::DoNotOptimize(value);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(items_count);
}

template<class TimeQueue>
static void
Size(benchmark::State& state)
{
    TimeQueue tq(300, 1, service, kIterations);
    for (size_t i = 0; i < kIterations; ++i) {
        tq.push({}, 10);
    }

    for (auto _ : state) {
        std::size_t size = tq.size();
        benchmark::DoNotOptimize(size);
        benchmark::ClobberMemory();
    }
}

template<class TimeQueue>
static void
Empty(benchmark::State& state)
{
    TimeQueue tq(300, 1, service, kIterations);
    for (size_t i = 0; i < kIterations; ++i) {
        tq.push({}, 10);
    }

    for (auto _ : state) {
        std::size_t size = tq.empty();
        benchmark::DoNotOptimize(size);
        benchmark::ClobberMemory();
    }
}

template<class TimeQueue>
static void
PushAndPopLoaded(benchmark::State& state)
{
    TimeQueue tq(state.range(0), 1, service, kIterations);
    int64_t items_count = 0;

    for (auto _ : state) {
        ++items_count;
        tq.push({}, 1000);

        // Simulate load by not popping all items
        if (items_count % 100 == 0) {
            auto elem = tq.front();
            tq.pop();
            benchmark::DoNotOptimize(elem);
            benchmark::ClobberMemory();
        }
    }

    state.SetItemsProcessed(items_count);
}

template<class TimeQueue>
static void
PushAndPop_Interval_1ms(benchmark::State& state)
{
    TimeQueue tq(state.range(0), 1, service, kIterations);
    int64_t items_count = 0;

    for (auto _ : state) {
        ++items_count;
        tq.push({}, 1000);

        auto elem = tq.front();
        tq.pop();

        benchmark::DoNotOptimize(elem);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(items_count);
}

template<class TimeQueue>
static void
PushAndPop_Interval_125ms(benchmark::State& state)
{
    TimeQueue tq(state.range(0), 125, service, kIterations);
    int64_t items_count = 0;

    for (auto _ : state) {
        ++items_count;
        tq.push({}, 1000);

        auto elem = tq.front();
        tq.pop();

        benchmark::DoNotOptimize(elem);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(items_count);
}

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
