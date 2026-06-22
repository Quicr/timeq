// SPDX-FileCopyrightText: Copyright (c) 2023 Cisco Systems
// SPDX-License-Identifier: BSD-2-Clause

/**
 *  time_queue.h
 *
 *  Description:
 *      A time based queue, where the length of the queue is a duration,
 *      divided into buckets based on a given time interval. As time
 *      progresses, buckets in the past are cleared, and the main queue
 *      is updated so that the front only returns a valid object that
 *      has not expired. To improve performance, buckets are only cleared
 *      on push or pop operations. Thus, buckets in the past can be
 *      cleared in bulk based on how many we should have advanced since
 *      the last time we updated.
 *
 *  Portability Issues:
 *      None.
 */

#pragma once

#include "tick_service.h"

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace timeq {
#define FORCE_INLINE __attribute__((always_inline))

    /**
     * @brief Aging element FIFO queue.
     *
     * @details Time based queue that maintains the push/pop order, but expires
     *          older values given a specific ttl.
     *
     * @tparam T The element type to be stored.
     */
    template<typename T>
    class time_queue
    {
      protected:
        using tick_type = tick_service::tick_type;
        using index_type = std::uint64_t;
        using bucket_type = std::vector<T>;

        struct queue_value_type
        {
            queue_value_type(bucket_type& bucket, index_type value_index, tick_type expiry_tick)
              : bucket(std::addressof(bucket))
              , value_index(value_index)
              , expiry_tick(expiry_tick)
            {
            }

            queue_value_type(const queue_value_type&) = default;
            queue_value_type& operator=(const queue_value_type&) = default;

            bucket_type* bucket;
            index_type value_index;
            tick_type expiry_tick;
        };

        using queue_type = std::vector<queue_value_type>;

      public:
        template<typename U>
        struct element
        {
            /// Value of front object
            std::optional<U> value;

            /// Number of items expired before on this front access
            std::uint32_t expired{ 0 };
        };

        template<typename U>
        struct element<U&>
        {
            /// Value of front object
            std::optional<std::reference_wrapper<U>> value;

            /// Number of items expired before on this front access
            std::uint32_t expired{ 0 };
        };

#if defined(__clang__) || (defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 12)
        template<typename U>
        element(U) -> element<U>;
#endif

        using value_type = element<T>;
        using reference = element<T&>;

        /**
         * @brief Construct a time_queue with defaults or supplied parameters
         *
         * @param duration Duration of the queue in milliseconds. Value must be > 0, and != interval.
         * @param interval Interval of ticks in milliseconds. Value must be > 0, < duration, duration % interval == 0.
         * @param tick_service Shared pointer to tick_service.
         * @param initial_queue_size Initial size of the queue to reserve.
         *
         * @throws std::invalid_argument If the duration or interval do not meet requirements or the tick_service is
         * null.
         */
        time_queue(std::size_t duration,
                   std::size_t interval,
                   std::shared_ptr<tick_service> tick_service,
                   std::size_t initial_queue_size)
          : _duration{ duration }
          , _interval{ interval }
          , _tick_service(std::move(tick_service))
        {
            if (duration == 0 || duration % interval != 0 || duration == interval) {
                throw std::invalid_argument("Invalid time_queue constructor args");
            }

            if (!_tick_service) {
                throw std::invalid_argument("Tick service cannot be null");
            }

            _queue.reserve(initial_queue_size);
        }

        /**
         * @brief Construct a time_queue with defaults or supplied parameters
         *
         * @param duration Duration of the queue in milliseconds. Value must be > 0, and != interval.
         * @param interval Interval of ticks in milliseconds. Must be > 0, < duration, duration % interval == 0.
         * @param tick_service Shared pointer to tick_service service.
         *
         * @throws std::invalid_argument If the duration or interval do not meet requirements or If the tick_service is
         *         null.
         */
        time_queue(std::size_t duration, std::size_t interval, std::shared_ptr<tick_service> tick_service)
          : time_queue(duration, interval, std::move(tick_service), duration / interval)
        {
        }

        time_queue() = delete;
        time_queue(const time_queue&) = default;
        time_queue(time_queue&&) noexcept = default;

        time_queue& operator=(const time_queue&) = default;
        time_queue& operator=(time_queue&&) noexcept = default;

        /**
         * @brief pushes a new value onto the queue with a time-to-live.
         *
         * @param value         The value to push onto the queue.
         * @param ttl           Time to live for an object in milliseconds
         *
         * @throws std::invalid_argument If ttl is greater than duration.
         */
        FORCE_INLINE void push(const T& value, std::size_t ttl) { internal_push(value, ttl); }

        /**
         * @brief pushes a new value onto the queue with a time-to-live.
         *
         * @param value      The value to push onto the queue.
         * @param ttl        Time to live for an object in milliseconds
         *
         * @throws std::invalid_argument If ttl is greater than duration.
         */
        FORCE_INLINE void push(T&& value, std::size_t ttl) { internal_push(std::forward<T>(value), ttl); }

        /**
         * @brief Pop (increment) front
         *
         * @details This method should be called after front when the object is
         * processed. This will move the queue forward. If at the end of the queue,
         * it'll be cleared and reset.
         */
        FORCE_INLINE void pop() noexcept
        {
            if (_queue.empty() || ++_queue_index < _queue.size()) {
                return;
            }

            clear();
        }

        /**
         * @brief Returns the most valid front of the queue without popping.
         *
         * @returns Element of the front value
         */
        FORCE_INLINE reference front()
        {
            const tick_type ticks = advance();

            if (_queue.empty()) {
                return { std::nullopt, 0 };
            }

            std::uint32_t expired = 0;

            while (_queue_index < _queue.size()) {
                auto& [bucket, value_index, expiry_tick] = _queue.at(_queue_index);

                if (ticks > expiry_tick || value_index >= bucket->size()) {
                    expired++;
                    _queue_index++;
                    continue;
                }

                return { bucket->at(value_index), expired };
            }

            clear();

            return { std::nullopt, expired };
        }

        /**
         * @brief Pops (removes) the front of the queue.
         *
         * @returns element of the popped value
         */
        [[nodiscard]] FORCE_INLINE value_type pop_front()
        {
            auto&& [value, expired] = front();
            value_type elem{ value.has_value() ? std::make_optional(std::move(value->get())) : std::nullopt, expired };

            if (elem.value.has_value()) {
                pop();
            }

            return elem;
        }

        FORCE_INLINE constexpr std::size_t size() const noexcept { return _queue.size() - _queue_index; }

        FORCE_INLINE constexpr bool empty() const noexcept { return _queue.empty() || _queue_index >= _queue.size(); }

        FORCE_INLINE void update() { [[maybe_unused]] auto _ = advance(); }

        /**
         * @brief Clear/reset the queue to no objects
         */
        FORCE_INLINE void clear() noexcept
        {
            if (_queue.empty()) {
                return;
            }

            _queue.clear();

            _buckets.clear();

            _queue_index = _bucket_index = 0;
            _last_tick_queue_cleared = _current_ticks;
        }

      protected:
        [[nodiscard]] FORCE_INLINE constexpr index_type get_future_bucket_index(index_type delta)
        {
            return (_bucket_index + delta) % (_duration / _interval);
        }

        /**
         * @brief Based on current time, adjust and move the bucket index with time
         *        (sliding window)
         *
         * @returns Current tick value at time of advance
         */
        [[nodiscard]] FORCE_INLINE tick_type advance()
        {
            const tick_type new_tick_count =
              std::chrono::duration_cast<std::chrono::milliseconds>(_tick_service->get()).count();
            const tick_type delta = new_tick_count - _current_ticks;
            _current_ticks = new_tick_count;

            if (delta == 0) {
                return new_tick_count;
            }

            if (delta >= _duration) {
                clear();
                return new_tick_count;
            }

            for (std::size_t i = 0; i < delta / _interval; ++i) {
                bucket_type& bucket = _buckets[get_future_bucket_index(i)];
                bucket.clear();
                bucket.shrink_to_fit();
            }

            _bucket_index = get_future_bucket_index(delta / _interval);

            if (_current_ticks - _last_tick_queue_cleared >= _duration && !_queue.empty()) {
                _queue.erase(_queue.begin(), std::next(_queue.begin(), _queue_index));
                _queue_index = 0;
                _last_tick_queue_cleared = _current_ticks;
            }

            return _current_ticks;
        }

        /**
         * @brief pushes new element onto the queue and adds it to future bucket.
         *
         * @details Internal definition of push. pushes value into specified
         *          bucket, and then emplaces the location info into the queue.
         *
         * @param value         The value to push onto the queue.
         * @param ttl           Time to live for an object in milliseconds
         * @param delay_ttl     pop wait Time to live for an object in milliseconds
         *                      This will cause pop to be delayed by this TTL value
         *
         * @throws std::invalid_argument If ttl is greater than duration.
         */
        FORCE_INLINE void internal_push(auto&& value, std::size_t ttl)
        {
            if (ttl > _duration) {
                throw std::invalid_argument("TTL is greater than max duration");
            }

            if (ttl == 0) {
                ttl = _duration;
            }

            auto relative_ttl = ttl / _interval;

            const tick_type ticks = advance();

            const tick_type expiry_tick = ticks + ttl;

            const index_type future_index = get_future_bucket_index(relative_ttl - 1);

            bucket_type& bucket = _buckets[future_index];

            bucket.emplace_back(value);
            _queue.emplace_back(bucket, bucket.size() - 1, expiry_tick);
        }

      protected:
        /// The duration in ticks of the entire queue.
        const std::size_t _duration;

        /// The interval at which buckets are cleared in ticks.
        const std::size_t _interval;

        /// The index in time of the current bucket.
        index_type _bucket_index{ 0 };

        /// The index of the first valid item in the queue.
        index_type _queue_index{ 0 };

        /// Last calculated tick value.
        tick_type _current_ticks{ 0 };

        /// Last calculated tick value when queue was cleared.
        tick_type _last_tick_queue_cleared{ 0 };

        /// The memory storage for all elements to be managed.
        std::map<std::uint64_t, bucket_type> _buckets;

        /// The FIFO ordered queue of values as they were inserted.
        queue_type _queue;

        /// Tick service for calculating new tick and jumps in time.
        std::shared_ptr<tick_service> _tick_service;
    };

#undef FORCE_INLINE
}; // namespace timeq
