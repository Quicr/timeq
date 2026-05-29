#include <gtest/gtest.h>

#include <timeq/tick_service.h>
#include <timeq/time_queue.h>

using namespace timeq;

struct test_tick_service : public tick_service
{
    test_tick_service() = default;
    virtual ~test_tick_service() = default;

    std::chrono::microseconds get() const override { return std::chrono::microseconds(ticks); }

    std::chrono::microseconds ticks;
};

static auto tick_manager = std::make_shared<test_tick_service>();

TEST(time_queue, Construction)
{
    ASSERT_NO_THROW(time_queue<int>(10, 1, tick_manager));
    ASSERT_THROW(time_queue<int>(10, 1, nullptr), std::invalid_argument);
    ASSERT_THROW(time_queue<int>(0, 1, tick_manager), std::invalid_argument);
}

TEST(time_queue, PushAndExpire)
{
    time_queue<int> tq(10, 1, tick_manager);

    tq.push(123, 2);
    ASSERT_EQ(tq.front().value.value(), 123);

    tick_manager->ticks += std::chrono::milliseconds(1);
    ASSERT_EQ(tq.front().value.value(), 123);

    tick_manager->ticks += std::chrono::milliseconds(1);
    ASSERT_FALSE(tq.front().value.has_value());
}

TEST(time_queue, PushAndPop)
{
    time_queue<int> tq(10, 1, tick_manager);

    tq.push(123, 1);
    ASSERT_EQ(tq.pop_front().value.value(), 123);

    auto elem = tq.front();
    ASSERT_FALSE(elem.value.has_value());
}

TEST(time_queue, PushAndExpireBeforePop)
{
    time_queue<int> tq(10, 1, tick_manager);

    tq.push(123, 1);

    tick_manager->ticks += std::chrono::milliseconds(1);
    ASSERT_FALSE(tq.pop_front().value.has_value());
}

TEST(time_queue, PushAndPopSequential)
{
    time_queue<int> tq(10, 1, tick_manager);

    for (int i = 0; i < 10; ++i)
    {
        tq.push(i, 1);
    }

    size_t popped = 0;
    for (auto elem = tq.pop_front(); elem.value.has_value(); elem = tq.pop_front())
    {
        ASSERT_EQ(elem.value.value(), popped++);
    }

    ASSERT_EQ(popped, 10);
}

TEST(time_queue, PushAndPopSequentialButExpireSome)
{
    time_queue<int> tq(10, 1, tick_manager);

    for (int i = 0; i < 10; ++i)
    {
        tq.push(i, i + 1);
    }

    size_t popped = 0;
    size_t expected_value = 0;
    size_t expected_expired = 0;

    for (auto elem = tq.pop_front(); elem.value.has_value(); elem = tq.pop_front())
    {
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

TEST(time_queue, ExpireAllBeforePop)
{
    time_queue<int> tq(10, 1, tick_manager);

    for (int i = 0; i < 10; ++i)
    {
        tq.push(i, i + 1);
    }

    tick_manager->ticks = std::chrono::milliseconds(10);

    // Even though all elements have expired, we haven't updated the queue, so it won't know it is empty
    ASSERT_FALSE(tq.empty());

    // Try to pop an element, updating the queue.
    for (auto elem = tq.pop_front(); elem.value.has_value();)
    {
        // If we successfully popped an item, then we failed to expire the whole queue after the duration of the queue
        // has passed.
        FAIL();
    }

    // Now we have updated the queue, we expect it to be empty.
    ASSERT_TRUE(tq.empty());
}
