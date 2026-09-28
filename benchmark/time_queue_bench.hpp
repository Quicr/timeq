#pragma once

#include <benchmark/benchmark.h>
#include <timeq/tick_service.h>

#include <type_traits>

struct mock_tick_service : timeq::tick_service
{
    explicit mock_tick_service(std::chrono::milliseconds ticks)
      : ticks(ticks)
    {
    }

    std::chrono::microseconds get() const override { return ticks; }

    std::chrono::microseconds ticks;
};

static auto service = std::make_shared<timeq::threaded_tick_service>();

constexpr size_t kIterations = 1'000'000;

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
FirstPushAfterIdle(benchmark::State& state)
{
    const std::size_t duration = state.range(0);
    auto aged_service = std::make_shared<mock_tick_service>(std::chrono::milliseconds(duration - 1));
    TimeQueue tq(duration, 1, aged_service, 1);

    for (auto _ : state) {
        tq.push({}, duration);

        std::size_t size = tq.size();
        benchmark::DoNotOptimize(size);
        benchmark::ClobberMemory();
    }
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

    using T = TimeQueue::value_type::type;
    const T data{};

    for (auto _ : state) {
        ++items_count;
        tq.push(data, 1000);

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

    using T = TimeQueue::value_type::type;
    const T data{};

    for (auto _ : state) {
        ++items_count;
        tq.push(data, 1000);

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

    using T = TimeQueue::value_type::type;
    const T data{};

    for (auto _ : state) {
        ++items_count;
        tq.push(data, 1000);

        auto elem = tq.front();
        tq.pop();

        benchmark::DoNotOptimize(elem);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(items_count);
}
