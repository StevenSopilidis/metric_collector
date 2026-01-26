#ifndef METRIC_COLLECTOR_AGGREGATION_BUCKET_HPP
#define METRIC_COLLECTOR_AGGREGATION_BUCKET_HPP

#include "shard.hpp"

#include <array>
#include <cstdint>

namespace metric_collector::aggregation
{
template <std::size_t NUM_SHARDS> class Bucket
{
  public:
    static_assert(NUM_SHARDS > 0, "Num of shards must be positive number");
    static_assert(NUM_SHARDS % 2 == 0, "Num of shards must be power of two");

    Bucket() = default;

    template <MetricTypeConcept T> void addMetric(uint64_t key, uint64_t delta)
    {
        std::size_t  idx = key & (NUM_SHARDS - 1);
        MetricValue* ptr = shards_[idx].template store<T>(key);

        if (ptr == nullptr)
        {
            return; // type mismatch ignore
        }

        if constexpr (std::same_as<T, Counter>)
        {
            std::get<Counter>(ptr->metric).increment(delta);
        }
        else if constexpr (std::same_as<T, Gauge>)
        {
            std::get<Gauge>(ptr->metric).set(delta);
        }
        else if constexpr (std::same_as<T, Timer>)
        {
            std::get<Timer>(ptr->metric).record(delta);
        }
    }

    template <MetricTypeConcept T>
    [[nodiscard]] std::optional<std::reference_wrapper<const MetricValue>>
    getMetric(uint64_t key) const
    {
        std::size_t idx   = key & (NUM_SHARDS - 1);
        auto&       shard = shards_[idx];

        auto opt = shard.getMetric(key);
        if (!opt.has_value())
        {
            return std::nullopt;
        }

        const MetricValue& val = opt->get();
        if (!std::holds_alternative<T>(val.metric))
        {
            return std::nullopt;
        }

        return val;
    }

    void clear()
    {
        for (auto& shard : shards_)
        {
            shard.clear();
        }
    }

  private:
    std::array<Shard, NUM_SHARDS> shards_;
};
} // namespace metric_collector::aggregation

#endif
