#ifndef METRIC_COLLECTOR_AGGREGATION_METRICS_HPP
#define METRIC_COLLECTOR_AGGREGATION_METRICS_HPP

#include <atomic>
#include <cstdint>
#include <limits>
#include <variant>

namespace metric_collector::aggregation
{
enum class MetricType : uint8_t
{
    Counter,
    Gauge,
    Timer,
    Invalid
};

inline constexpr MetricType StringToMetricType(std::string_view value)
{
    if (value == "c")
    {
        return MetricType::Counter;
    }
    if (value == "g")
    {
        return MetricType::Gauge;
    }
    if (value == "t")
    {
        return MetricType::Timer;
    }

    return MetricType::Invalid;
}

class Counter
{
  public:
    void increment(uint64_t value = 1) noexcept
    {
        value_.fetch_add(value, std::memory_order_relaxed);
    }
    [[nodiscard]] auto get() const noexcept { return value_.load(std::memory_order_relaxed); }

  private:
    std::atomic<uint64_t> value_{0};
};

class Gauge
{
  public:
    Gauge() = default;

    void set(uint64_t value = 1) noexcept { value_.store(value, std::memory_order_relaxed); }
    [[nodiscard]] auto get() const noexcept { return value_.load(std::memory_order_relaxed); }

  private:
    std::atomic<uint64_t> value_{0};
};

class Timer
{
  public:
    Timer() = default;

    void record(uint64_t value) noexcept
    {
        count_.fetch_add(1, std::memory_order_relaxed);
        sum_.fetch_add(value, std::memory_order_relaxed);

        update_min(value);
        update_max(value);
    }

    [[nodiscard]] auto count() const noexcept { return count_.load(std::memory_order_relaxed); }
    [[nodiscard]] auto sum() const noexcept { return sum_.load(std::memory_order_relaxed); }
    [[nodiscard]] auto min() const noexcept { return min_.load(std::memory_order_relaxed); }
    [[nodiscard]] auto max() const noexcept { return max_.load(std::memory_order_relaxed); }

  private:
    void update_atomic_min(uint64_t new_val) noexcept
    {
        uint64_t curr = min_.load(std::memory_order_relaxed);
        while (new_val < curr &&
               !min_.compare_exchange_weak(curr, new_val, std::memory_order_relaxed,
                                           std::memory_order_relaxed))
        {
        }
    }

    void update_atomic_max(uint64_t new_val) noexcept
    {
        uint64_t curr = max_.load(std::memory_order_relaxed);
        while (new_val > curr &&
               !max_.compare_exchange_weak(curr, new_val, std::memory_order_relaxed,
                                           std::memory_order_relaxed))
        {
        }
    }

    void update_min(uint64_t value) noexcept { update_atomic_min(value); }
    void update_max(uint64_t value) noexcept { update_atomic_max(value); }

    std::atomic<uint64_t> count_{0};
    std::atomic<uint64_t> sum_{0};
    std::atomic<uint64_t> min_{std::numeric_limits<uint64_t>::max()};
    std::atomic<uint64_t> max_{std::numeric_limits<uint64_t>::min()};
};

using MetricVariant = std::variant<Counter, Gauge, Timer>;

template <typename T>
concept MetricTypeConcept =
    std::same_as<T, Counter> || std::same_as<T, Gauge> || std::same_as<T, Timer>;

struct MetricValue
{
    MetricValue() = default;

    template <MetricTypeConcept T>
    explicit MetricValue(std::in_place_type_t<T> /*unused*/) : metric(std::in_place_type<T>)
    {
    }

    MetricVariant metric;
};

template <MetricType> struct MetricSelector;

template <> struct MetricSelector<MetricType::Counter>
{
    using type = Counter;
};

template <> struct MetricSelector<MetricType::Gauge>
{
    using type = Gauge;
};

template <> struct MetricSelector<MetricType::Timer>
{
    using type = Timer;
};

template <MetricType T> MetricValue create_metric()
{
    using type = typename MetricSelector<T>::type;
    return MetricValue(std::in_place_type<type>);
};

} // namespace metric_collector::aggregation

#endif