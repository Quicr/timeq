#include <benchmark/benchmark.h>

#include <timeq/time_queue.h>

static auto service = std::make_shared<timeq::threaded_tick_service>();

constexpr size_t kIterations = 100'000'000;

template<class T>
static void
BM_TimeQueue_Construct(benchmark::State& state)
{
    for (auto _ : state) {
        auto tq = timeq::time_queue<T>(300, 1, service, kIterations);

        std::size_t size = tq.size();
        benchmark::DoNotOptimize(size);
        benchmark::ClobberMemory();
    }
}

template<class T>
static void
BM_TimeQueue_Push(benchmark::State& state)
{
    timeq::time_queue<T> tq(state.range(0), 1, service, kIterations);
    int64_t items_count = 0;

    for (auto _ : state) {
        ++items_count;
        tq.push({}, 20);
    }

    state.SetItemsProcessed(items_count);
}

template<class T>
static void
BM_TimeQueue_Pop(benchmark::State& state)
{
    timeq::time_queue<T> tq(state.range(0), 1, service, kIterations);
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

template<class T>
static void
BM_TimeQueue_Front(benchmark::State& state)
{
    timeq::time_queue<T> tq(state.range(0), 1, service, kIterations);
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

template<class T>
static void
BM_TimeQueue_PopFront(benchmark::State& state)
{
    timeq::time_queue<T> tq(state.range(0), 1, service, kIterations);
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

template<class T>
static void
BM_TimeQueue_Size(benchmark::State& state)
{
    timeq::time_queue<T> tq(300, 1, service, kIterations);
    for (size_t i = 0; i < kIterations; ++i) {
        tq.push({}, 10);
    }

    for (auto _ : state) {
        std::size_t size = tq.size();
        benchmark::DoNotOptimize(size);
        benchmark::ClobberMemory();
    }
}

template<class T>
static void
BM_TimeQueue_Empty(benchmark::State& state)
{
    timeq::time_queue<T> tq(300, 1, service, kIterations);
    for (size_t i = 0; i < kIterations; ++i) {
        tq.push({}, 10);
    }

    for (auto _ : state) {
        std::size_t size = tq.empty();
        benchmark::DoNotOptimize(size);
        benchmark::ClobberMemory();
    }
}

template<class T>
static void
BM_TimeQueue_PushAndPopLoaded(benchmark::State& state)
{
    timeq::time_queue<T> tq(state.range(0), 1, service, kIterations);
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

template<class T>
static void
BM_TimeQueue_PushAndPop_Interval_1ms(benchmark::State& state)
{
    timeq::time_queue<T> tq(state.range(0), 1, service, kIterations);
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

template<class T>
static void
BM_TimeQueue_PushAndPop_Interval_125ms(benchmark::State& state)
{
    timeq::time_queue<T> tq(state.range(0), 125, service, kIterations);
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

BENCHMARK(BM_TimeQueue_Construct<int>)->Arg(300);
BENCHMARK(BM_TimeQueue_Push<int>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_Pop<int>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_Front<int>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_PopFront<int>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_Size<int>)->Iterations(kIterations);
BENCHMARK(BM_TimeQueue_Empty<int>)->Iterations(kIterations);
BENCHMARK(BM_TimeQueue_PushAndPopLoaded<int>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_PushAndPop_Interval_1ms<int>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_PushAndPop_Interval_125ms<int>)->Arg(5000)->Arg(1'000'000);

struct NonTrivialType
{
    std::shared_ptr<int> ptr;
};

static_assert(!std::is_trivially_copyable_v<NonTrivialType>);

BENCHMARK(BM_TimeQueue_Construct<NonTrivialType>)->Arg(300);
BENCHMARK(BM_TimeQueue_Push<NonTrivialType>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_Pop<NonTrivialType>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_Front<NonTrivialType>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_PopFront<NonTrivialType>)->Iterations(kIterations)->Arg(300)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_Size<NonTrivialType>)->Iterations(kIterations);
BENCHMARK(BM_TimeQueue_Empty<NonTrivialType>)->Iterations(kIterations);
BENCHMARK(BM_TimeQueue_PushAndPopLoaded<NonTrivialType>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_PushAndPop_Interval_1ms<NonTrivialType>)->Arg(5000)->Arg(1'000'000);
BENCHMARK(BM_TimeQueue_PushAndPop_Interval_125ms<NonTrivialType>)->Arg(5000)->Arg(1'000'000);
