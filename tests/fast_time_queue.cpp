#include <gtest/gtest.h>

#include <timeq/fast_time_queue.h>
#include <timeq/tick_service.h>

using namespace timeq;

struct test_tick_service : public tick_service
{
    test_tick_service() = default;
    virtual ~test_tick_service() = default;

    std::chrono::microseconds get() const override { return std::chrono::microseconds(ticks); }

    std::chrono::microseconds ticks;
};

static auto tick_manager = std::make_shared<test_tick_service>();

template<typename T>
class inspectable_fast_time_queue : public fast_time_queue<T>
{
  public:
    using fast_time_queue<T>::fast_time_queue;

    std::size_t raw_queue_size() const noexcept { return this->_queue.size(); }

    std::size_t queue_index() const noexcept { return this->_queue_index; }
};

TEST(fast_time_queue, Construction)
{
    ASSERT_NO_THROW(fast_time_queue<int>(10, 1, tick_manager));
    ASSERT_THROW(fast_time_queue<int>(10, 1, nullptr), std::invalid_argument);
    ASSERT_THROW(fast_time_queue<int>(0, 1, tick_manager), std::invalid_argument);
}

TEST(fast_time_queue, PushAndExpire)
{
    fast_time_queue<int> tq(10, 1, tick_manager);

    tq.push(123, 2);
    ASSERT_EQ(tq.front().value.value(), 123);

    tick_manager->ticks += std::chrono::milliseconds(1);
    ASSERT_EQ(tq.front().value.value(), 123);

    tick_manager->ticks += std::chrono::milliseconds(1);
    ASSERT_FALSE(tq.front().value.has_value());
}

TEST(fast_time_queue, PushMidIntervalWithMaxTtlExpiresAtTtl)
{
    auto tick_manager = std::make_shared<test_tick_service>();
    fast_time_queue<int> tq(5000, 500, tick_manager);

    tick_manager->ticks = std::chrono::milliseconds(100);
    tq.push(123, 5000);

    for (int ticks = 200; ticks <= 5000; ticks += 100) {
        tick_manager->ticks = std::chrono::milliseconds(ticks);
        tq.update();
    }

    tick_manager->ticks = std::chrono::milliseconds(5099);
    ASSERT_EQ(tq.front().value.value(), 123);

    tick_manager->ticks = std::chrono::milliseconds(5100);
    ASSERT_FALSE(tq.front().value.has_value());
}

TEST(fast_time_queue, SubIntervalUpdatesStillAdvanceBuckets)
{
    auto tick_manager = std::make_shared<test_tick_service>();
    fast_time_queue<int> tq(5000, 500, tick_manager);

    tq.push(123, 500);

    for (int ticks = 100; ticks <= 500; ticks += 100) {
        tick_manager->ticks = std::chrono::milliseconds(ticks);
        tq.update();
    }

    ASSERT_FALSE(tq.front().value.has_value());
    ASSERT_TRUE(tq.empty());
}

TEST(fast_time_queue, DelayedPopBacklogCompactsQueue)
{
    auto tick_manager = std::make_shared<test_tick_service>();
    inspectable_fast_time_queue<int> tq(5000, 500, tick_manager);

    for (int i = 0; i < 80; ++i) {
        tick_manager->ticks = std::chrono::milliseconds(i * 100);
        tq.push(i, 5000);
    }

    ASSERT_EQ(tq.size(), std::size_t{ 80 });
    ASSERT_EQ(tq.raw_queue_size(), std::size_t{ 80 });

    tick_manager->ticks = std::chrono::milliseconds(8000);
    while (tq.size() > std::size_t{ 5 }) {
        auto elem = tq.pop_front();
        ASSERT_TRUE(elem.value.has_value());
    }

    EXPECT_EQ(tq.size(), std::size_t{ 5 });
    EXPECT_EQ(tq.raw_queue_size(), std::size_t{ 5 });
    EXPECT_EQ(tq.queue_index(), std::size_t{ 0 });

    tick_manager->ticks = std::chrono::milliseconds(11000);
    tq.update();

    EXPECT_EQ(tq.size(), std::size_t{ 5 });
    EXPECT_EQ(tq.raw_queue_size(), std::size_t{ 5 });
    EXPECT_EQ(tq.queue_index(), std::size_t{ 0 });
    EXPECT_EQ(tq.front().value.value(), 75);
}

TEST(fast_time_queue, PushAndPop)
{
    fast_time_queue<int> tq(10, 1, tick_manager);

    tq.push(123, 1);
    ASSERT_EQ(tq.pop_front().value.value(), 123);

    auto elem = tq.front();
    ASSERT_FALSE(elem.value.has_value());
}

TEST(fast_time_queue, PushAndExpireBeforePop)
{
    fast_time_queue<int> tq(10, 1, tick_manager);

    tq.push(123, 1);

    tick_manager->ticks += std::chrono::milliseconds(1);
    ASSERT_FALSE(tq.pop_front().value.has_value());
}

TEST(fast_time_queue, PushAndPopSequential)
{
    fast_time_queue<int> tq(10, 1, tick_manager);

    for (int i = 0; i < 10; ++i) {
        tq.push(i, 1);
    }

    size_t popped = 0;
    for (auto elem = tq.pop_front(); elem.value.has_value(); elem = tq.pop_front()) {
        ASSERT_EQ(elem.value.value(), popped++);
    }

    ASSERT_EQ(popped, 10);
}

TEST(fast_time_queue, PushAndPopSequentialButExpireSome)
{
    fast_time_queue<int> tq(10, 1, tick_manager);

    for (int i = 0; i < 10; ++i) {
        tq.push(i, i + 1);
    }

    size_t popped = 0;
    size_t expected_value = 0;
    size_t expected_expired = 0;

    for (auto elem = tq.pop_front(); elem.value.has_value(); elem = tq.pop_front()) {
        ++popped;
        EXPECT_EQ(elem.value.value(), expected_value);
        EXPECT_EQ(elem.expired, expected_expired);

        expected_value += 4;
        expected_expired = 3;

        tick_manager->ticks += std::chrono::milliseconds(4);
        tq.update();
    }

    ASSERT_EQ(popped, 3);
}

TEST(fast_time_queue, ExpireAllBeforePop)
{
    fast_time_queue<int> tq(10, 1, tick_manager);

    for (int i = 0; i < 10; ++i) {
        tq.push(i, i + 1);
    }

    tick_manager->ticks = std::chrono::milliseconds(10);

    // Even though all elements have expired, we haven't updated the queue, so it won't know it is empty
    ASSERT_FALSE(tq.empty());

    // Try to pop an element, updating the queue.
    for (auto elem = tq.pop_front(); elem.value.has_value();) {
        // If we successfully popped an item, then we failed to expire the whole queue after the duration of the queue
        // has passed.
        FAIL();
    }

    // Now we have updated the queue, we expect it to be empty.
    ASSERT_TRUE(tq.empty());
}
