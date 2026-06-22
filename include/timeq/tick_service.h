// SPDX-FileCopyrightText: Copyright (c) 2023 Cisco Systems
// SPDX-License-Identifier: BSD-2-Clause

/**
 *  tick_service.h
 *
 *  Description:
 *
 *  Portability Issues:
 *      None.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>
#include <type_traits>

namespace timeq {
    /**
     * Interface for services that calculate ticks.
     */
    struct tick_service
    {
        using tick_type = size_t;

        virtual ~tick_service() = default;

        virtual std::chrono::microseconds get() const = 0;
    };

    /**
     * @brief Calculates elapsed time in ticks.
     *
     * @details Calculates time that's elapsed between update calls. Keeps
     *          track of time using ticks as a counter of elapsed time. The
     *          precision 500us or greater, which results in the tick interval
     *          being >= 500us.
     */
    class threaded_tick_service : public tick_service
    {
        using clock_type = std::chrono::steady_clock;

      public:
        threaded_tick_service(std::uint64_t sleep_delay_us = 333)
          : _sleep_delay_us{ sleep_delay_us }
        {
            _tick_thread = std::thread(&threaded_tick_service::tick_loop, this);
        }

        threaded_tick_service(const threaded_tick_service& other)
          : _ticks{ other._ticks.load(std::memory_order_relaxed) }
          , _sleep_delay_us{ other._sleep_delay_us }
          , _stop{ other._stop.load() }
        {
            _tick_thread = std::thread(&threaded_tick_service::tick_loop, this);
        }

        virtual ~threaded_tick_service()
        {
            _stop = true;
            _tick_thread.join();
        }

        threaded_tick_service& operator=(const threaded_tick_service& other)
        {
            _ticks.store(other._ticks.load(std::memory_order_relaxed), std::memory_order_relaxed);

            _sleep_delay_us = other._sleep_delay_us;
            _stop = other._stop.load();

            _tick_thread = std::thread(&threaded_tick_service::tick_loop, this);

            return *this;
        }

        [[nodiscard]] std::chrono::microseconds get() const override
        {
            return std::chrono::microseconds(_ticks.load(std::memory_order_relaxed));
        }

      private:
        void tick_loop()
        {
            clock_type::time_point prev_time = clock_type::now();

            while (!_stop) {
                const clock_type::time_point now = clock_type::now();
                const std::uint64_t delta =
                  std::chrono::duration_cast<std::chrono::microseconds>(now - prev_time).count();
                std::this_thread::sleep_for(std::chrono::microseconds(_sleep_delay_us));

                if (delta >= _sleep_delay_us) {
                    _ticks.fetch_add(delta, std::memory_order_relaxed);
                    prev_time = now;
                }
            }
        }

      private:
        /// The current ticks since the tick_service began.
        std::atomic<std::uint64_t> _ticks{ 0 };

        /// Sleep delay in duration_type.
        std::uint64_t _sleep_delay_us;

        /// Flag to stop tick_service thread.
        std::atomic<bool> _stop{ false };

        /// The thread to update ticks on.
        std::thread _tick_thread;
    };

}; // namespace timeq
