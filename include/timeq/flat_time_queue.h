// SPDX-FileCopyrightText: Copyright (c) 2023 Cisco Systems
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include "tick_service.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace timeq {
#ifdef _MSC_VER
#define FORCE_INLINE inline __forceinline
#else
#define FORCE_INLINE inline __attribute__((always_inline))
#endif

    /**
     * @brief Aging element FIFO queue using contiguous inline storage.
     *
     * @details Time based queue that maintains the push/pop order, but expires
     *          older values given a specific ttl.
     *
     * @tparam T The element type to be stored.
     */
    template<typename T>
    class flat_time_queue
    {
      protected:
        using tick_type = tick_service::tick_type;
        using index_type = std::uint64_t;

        struct queue_value_type
        {
            T value;
            tick_type expiry_tick;
            tick_type wait_for_tick;
        };

        using queue_type = std::vector<queue_value_type>;

      public:
        template<typename U>
        struct element
        {
            using type = U;

            /// Value of front object
            std::optional<U> value;

            /// Number of items expired before on this front access
            std::uint32_t expired{ 0 };
        };

        template<typename U>
        struct element<U&>
        {
            using type = U;

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
         * @brief Construct a flat_time_queue with supplied parameters.
         *
         * @param duration Duration of the queue in milliseconds. Value must be > 0, and != interval.
         * @param interval Interval of ticks in milliseconds. Value must be > 0, < duration, duration % interval == 0.
         * @param tick_service Shared pointer to tick_service.
         * @param initial_queue_size Initial size of the queue to reserve.
         *
         * @throws std::invalid_argument If the duration or interval do not meet requirements or the tick_service is
         * null.
         */
        flat_time_queue(std::size_t duration,
                        std::size_t interval,
                        std::shared_ptr<tick_service> tick_service,
                        std::size_t initial_queue_size)
          : _duration{ duration }
          , _interval{ interval }
          , _tick_service(std::move(tick_service))
        {
            if (duration == 0 || interval == 0 || duration % interval != 0 || duration == interval) {
                throw std::invalid_argument("Invalid flat_time_queue constructor args");
            }

            if (!_tick_service) {
                throw std::invalid_argument("Tick service cannot be null");
            }

            _queue.reserve(initial_queue_size);
        }

        /**
         * @brief Construct a flat_time_queue with supplied parameters.
         *
         * @param duration Duration of the queue in milliseconds. Value must be > 0, and != interval.
         * @param interval Interval of ticks in milliseconds. Must be > 0, < duration, duration % interval == 0.
         * @param tick_service Shared pointer to tick_service service.
         *
         * @throws std::invalid_argument If the duration or interval do not meet requirements or If the tick_service is
         *         null.
         */
        flat_time_queue(std::size_t duration, std::size_t interval, std::shared_ptr<tick_service> tick_service)
          : flat_time_queue(duration, interval, std::move(tick_service), duration / interval)
        {
        }

        flat_time_queue() = delete;
        flat_time_queue(const flat_time_queue&) = default;
        flat_time_queue(flat_time_queue&&) noexcept = default;

        flat_time_queue& operator=(const flat_time_queue&) = default;
        flat_time_queue& operator=(flat_time_queue&&) noexcept = default;

        /**
         * @brief Pushes a new value onto the queue with a time-to-live.
         *
         * @param value The value to push onto the queue.
         * @param ttl Time to live for an object in milliseconds.
         * @param delay_ttl Pop wait time to live for an object in milliseconds.
         *
         * @throws std::invalid_argument If ttl is greater than duration.
         */
        FORCE_INLINE void push(const T& value, std::size_t ttl, std::size_t delay_ttl = 0)
        {
            internal_push(value, ttl, delay_ttl);
        }

        /**
         * @brief Pushes a new value onto the queue with a time-to-live.
         *
         * @param value The value to push onto the queue.
         * @param ttl Time to live for an object in milliseconds.
         * @param delay_ttl Pop wait time to live for an object in milliseconds.
         *
         * @throws std::invalid_argument If ttl is greater than duration.
         */
        FORCE_INLINE void push(T&& value, std::size_t ttl, std::size_t delay_ttl = 0)
        {
            internal_push(std::forward<T>(value), ttl, delay_ttl);
        }

        /**
         * @brief Pop (increment) front.
         *
         * @details This method should be called after front when the object is
         * processed. This will move the queue forward. If at the end of the queue,
         * it'll be cleared and reset.
         */
        FORCE_INLINE void pop() noexcept
        {
            if (_queue.empty()) {
                return;
            }

            if (++_queue_index >= _queue.size()) {
                clear();
                return;
            }

            if (_queue_index >= size()) {
                compact_consumed_queue();
            }
        }

        /**
         * @brief Returns the most valid front of the queue without popping.
         *
         * @returns Element of the front value.
         */
        FORCE_INLINE reference front()
        {
            const tick_type ticks = advance();

            if (_queue.empty()) {
                return { std::nullopt, 0 };
            }

            std::uint32_t expired = 0;

            while (_queue_index < _queue.size()) {
                auto& [value, expiry_tick, pop_wait_ttl] = _queue.at(_queue_index);

                if (ticks >= expiry_tick) {
                    expired++;
                    _queue_index++;
                    continue;
                }

                if (pop_wait_ttl > ticks) {
                    return { std::nullopt, expired };
                }

                return { value, expired };
            }

            clear();

            return { std::nullopt, expired };
        }

        /**
         * @brief Pops (removes) the front of the queue.
         *
         * @returns Element of the popped value.
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
         * @brief Clear/reset the queue to no objects.
         */
        FORCE_INLINE void clear() noexcept
        {
            if (!_queue.empty()) {
                _queue.clear();
            }

            _queue_index = 0;
            _last_tick_queue_cleared = _last_advance_tick = _current_ticks;
        }

      protected:
        /**
         * @brief Based on current time, adjust and move the queue with time.
         *
         * @returns Current tick value at time of advance.
         */
        [[nodiscard]] FORCE_INLINE tick_type advance()
        {
            const tick_type new_tick_count =
              std::chrono::duration_cast<std::chrono::milliseconds>(_tick_service->get()).count();
            const tick_type delta = new_tick_count - _last_advance_tick;
            _current_ticks = new_tick_count;

            if (delta < _interval) {
                return _current_ticks;
            }

            if (delta > _duration) {
                clear();
                return _current_ticks;
            }

            _last_advance_tick = _current_ticks;

            if (_current_ticks - _last_tick_queue_cleared > _duration && !_queue.empty()) {
                compact_consumed_queue();
                _last_tick_queue_cleared = _current_ticks;
            }

            return _current_ticks;
        }

        /**
         * @brief Pushes a new element onto the queue.
         *
         * @param value The value to push onto the queue.
         * @param ttl Time to live for an object in milliseconds.
         * @param delay_ttl Pop wait time to live for an object in milliseconds.
         *
         * @throws std::invalid_argument If ttl is greater than duration.
         */
        FORCE_INLINE void internal_push(auto&& value, std::size_t ttl, std::size_t delay_ttl)
        {
            if (ttl > _duration) {
                throw std::invalid_argument("TTL is greater than max duration");
            }

            if (ttl == 0) {
                ttl = _duration;
            }

            const tick_type ticks = advance();

            const tick_type expiry_tick = ticks + ttl;

            _queue.emplace_back(std::forward<decltype(value)>(value), expiry_tick, ticks + delay_ttl);
        }

        FORCE_INLINE void compact_consumed_queue() noexcept
        {
            if (_queue_index == 0) {
                return;
            }

            if (_queue_index >= _queue.size()) {
                clear();
                return;
            }

            const std::size_t logical_size = this->size();
            std::move(_queue.begin() + static_cast<std::ptrdiff_t>(_queue_index), _queue.end(), _queue.begin());

            _queue.resize(logical_size);
            _queue_index = 0;
        }

      protected:
        /// The duration in ticks of the entire queue.
        const std::size_t _duration;

        /// The interval at which buckets are cleared in ticks.
        const std::size_t _interval;

        /// The index of the first valid item in the queue.
        index_type _queue_index{ 0 };

        /// Last calculated tick value.
        tick_type _current_ticks{ 0 };

        /// Last calculated tick value when queue was cleared.
        tick_type _last_tick_queue_cleared{ 0 };

        /// Last tick when advance actually advanced.
        tick_type _last_advance_tick{ 0 };

        /// The FIFO ordered queue of values as they were inserted.
        queue_type _queue;

        /// Tick service for calculating new tick and jumps in time.
        std::shared_ptr<tick_service> _tick_service;
    };

#undef FORCE_INLINE
}; // namespace timeq
