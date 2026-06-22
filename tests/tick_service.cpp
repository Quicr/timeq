#include <gtest/gtest.h>

#include <timeq/tick_service.h>

#include <chrono>
#include <memory>
#include <thread>

using namespace timeq;
using namespace std::chrono_literals;

namespace {

/// Lower bound as a fraction of wall-clock elapsed time (accounts for scheduler jitter).
constexpr double k_min_elapsed_fraction = 0.4;

/// Upper bound slack beyond wall-clock elapsed time (accounts for overshoot and startup lag).
constexpr auto k_max_elapsed_slack = 30ms;

void expect_ticks_within_wall_clock(
    std::chrono::microseconds tick_delta,
    std::chrono::microseconds wall_elapsed)
{
    const auto min_expected = std::chrono::microseconds(
      static_cast<std::int64_t>(wall_elapsed.count() * k_min_elapsed_fraction));
    const auto max_expected = wall_elapsed + k_max_elapsed_slack;

    EXPECT_GE(tick_delta, min_expected)
      << "ticks (" << tick_delta.count() << "us) fell below "
      << (k_min_elapsed_fraction * 100) << "% of wall time (" << wall_elapsed.count() << "us)";
    EXPECT_LE(tick_delta, max_expected)
      << "ticks (" << tick_delta.count() << "us) exceeded wall time (" << wall_elapsed.count()
      << "us) plus slack";
}

std::chrono::microseconds measure_tick_growth(threaded_tick_service& service,
                                              std::chrono::milliseconds wait_duration)
{
    const auto initial_ticks = service.get();
    const auto wall_start = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(wait_duration);
    const auto wall_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now() - wall_start);

    const auto tick_delta = service.get() - initial_ticks;
    expect_ticks_within_wall_clock(tick_delta, wall_elapsed);
    return tick_delta;
}

} // namespace

TEST(tick_service, InterfaceThroughPointer)
{
    std::unique_ptr<tick_service> service = std::make_unique<threaded_tick_service>(500);

    const auto initial = service->get();
    EXPECT_GE(initial, 0us);

    std::this_thread::sleep_for(25ms);
    EXPECT_GT(service->get(), initial);
}

TEST(threaded_tick_service, ConstructionAndDestruction)
{
    ASSERT_NO_THROW(threaded_tick_service service(500));
}

TEST(threaded_tick_service, InitialTicksNearZero)
{
    threaded_tick_service service(500);
    EXPECT_GE(service.get(), 0us);
    EXPECT_LT(service.get(), 5ms);
}

TEST(threaded_tick_service, TicksIncreaseWithWallClock)
{
    threaded_tick_service service(500);
    measure_tick_growth(service, 50ms);
}

TEST(threaded_tick_service, TicksIncreaseWithCustomSleepDelay)
{
    threaded_tick_service service(1000);
    measure_tick_growth(service, 75ms);
}

TEST(threaded_tick_service, TicksAreMonotonic)
{
    threaded_tick_service service(500);

    auto previous = service.get();
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(10ms);
        const auto current = service.get();
        EXPECT_GE(current, previous);
        previous = current;
    }
}

TEST(threaded_tick_service, CopyConstructorPreservesTicksAndContinues)
{
    threaded_tick_service original(500);
    std::this_thread::sleep_for(30ms);
    const auto original_ticks = original.get();

    threaded_tick_service copy(original);
    const auto copy_ticks = copy.get();

    EXPECT_GE(copy_ticks, original_ticks);

    const auto copy_before = copy.get();
    std::this_thread::sleep_for(20ms);
    EXPECT_GT(copy.get(), copy_before);
}

// Copy assignment is not tested: operator= replaces _tick_thread without stopping or
// joining the existing background thread, which terminates the process.

TEST(threaded_tick_service, DestructorStopsBackgroundThread)
{
    auto service = std::make_unique<threaded_tick_service>(500);
    std::this_thread::sleep_for(15ms);
    ASSERT_NO_THROW(service.reset());
}
