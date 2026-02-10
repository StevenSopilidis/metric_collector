#ifndef METRIC_COLLECTOR_INGESTION_SPSC_QUEUE
#define METRIC_COLLECTOR_INGESTION_SPSC_QUEUE

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>
#include <type_traits>

namespace metric_collector::ingestion
{

template <typename T, std::size_t Capacity> class SpscQueue
{
    static_assert(Capacity > 0, "Capacity must be positive number");
    static_assert(((Capacity & (Capacity - 1)) == 0), "Capacity must be power of two number");
    static_assert((std::is_nothrow_move_assignable_v<T> || std::is_nothrow_copy_assignable_v<T>),
                  "T must be nothrow move assignable or copy-assignable");

    static constexpr std::size_t MASK = Capacity - 1;

  public:
    SpscQueue() = default;

    SpscQueue(const SpscQueue&)            = delete;
    SpscQueue(SpscQueue&&)                 = delete;
    SpscQueue& operator=(const SpscQueue&) = delete;
    SpscQueue& operator=(SpscQueue&&)      = delete;

    void push(const T& item) noexcept
    {
        const auto wp   = write_pos_.load(std::memory_order_relaxed);
        auto       rp   = read_pos_.load(std::memory_order_acquire);
        auto       next = wp + 1;

        // buffer full drop older element
        if (next - rp > Capacity)
        {
            read_pos_.store(rp + 1, std::memory_order_release);
        }

        buffer_[wp & MASK] = item;
        write_pos_.store(next, std::memory_order_release);
    }

    [[nodiscard]] std::optional<T> pop() noexcept
    {
        auto rp = read_pos_.load(std::memory_order_acquire);
        auto wp = write_pos_.load(std::memory_order_acquire);

        if (rp == wp)
        {
            return std::nullopt;
        }

        auto item = std::move(buffer_[rp & MASK]);
        read_pos_.store(rp + 1, std::memory_order_release);
        return item;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return read_pos_.load(std::memory_order_acquire) ==
               write_pos_.load(std::memory_order_acquire);
    }

  private:
    std::array<T, Capacity>  buffer_;
    std::atomic<std::size_t> write_pos_{0};
    std::atomic<std::size_t> read_pos_{0};
};
} // namespace metric_collector::ingestion

#endif