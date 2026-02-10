#ifndef METRIC_COLLECTOR_INGESTION_WORKER
#define METRIC_COLLECTOR_INGESTION_WORKER

#include "spsc_queue.hpp"

#include <cstddef>
#include <span>
#include <thread>

namespace metric_collector::ingestion
{

constexpr std::size_t WORKER_QUEUE_CAPACITY = 8192;

class Worker
{
  public:
    using Packet = std::span<std::byte>;
    using Queue  = SpscQueue<Packet, WORKER_QUEUE_CAPACITY>;

    Worker() = default;

    Worker(const Worker&)            = default;
    Worker(Worker&&)                 = default;
    Worker& operator=(const Worker&) = default;
    Worker& operator=(Worker&&)      = default;

    [[nodiscard]] Queue& queue() noexcept { return queue_; }

    void start()
    {
        running_.store(true, std::memory_order_release);
        thread_ = std::thread([this]() { run(); });
    }

    void stop()
    {
        if (!running_.exchange(false, std::memory_order_acq_rel))
        {
            return; // already stopped
        }

        if (thread_.joinable())
        {
            thread_.join();
        }
    }

  private:
    static constexpr std::size_t SPIN_ITERATIONS  = 256;
    static constexpr std::size_t YIELD_ITERATIONS = 256;

    void run()
    {
        std::size_t idle = 0;

        while (running_.load(std::memory_order_acquire))
        {
            auto pkt = queue_.pop();

            if (pkt != std::nullopt)
            {
                // TODO process packet
                idle = 0;
            }
            else
            {
                adaptive_wait(idle);
            }
        }

        // drain any remaning packets
        drain();
    }

    void drain()
    {
        auto packet = queue_.pop();
        while (packet != std::nullopt)
        {
            // process packet
        }
    }

    void adaptive_wait(std::size_t& idle)
    {
        ++idle;

        if (idle <= SPIN_ITERATIONS)
        {
#if defined(__x86_64) || defined(_M_X64)
            __builtin_ia32_pause(); // x86_64 system
#elif defined(__aarch64__)
            asm volatile("yield"); // arm system
#endif
        }
        else if (idle <= YIELD_ITERATIONS)
        {
            std::this_thread::yield();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }

    Queue             queue_;
    std::thread       thread_;
    std::atomic<bool> running_{false};
};
} // namespace metric_collector::ingestion

#endif
