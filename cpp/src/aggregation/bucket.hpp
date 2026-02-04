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
    static_assert((NUM_SHARDS & (NUM_SHARDS - 1)) == 0, "Num of shards must be power of two");

    Bucket() = default;

    template <MetricTypeConcept T> void add_metric(uint64_t key, uint64_t delta)
    {
        std::size_t idx = key & (NUM_SHARDS - 1);
        auto        ptr = shards_[idx].template store<T>(key);

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
    [[nodiscard]] std::shared_ptr<MetricValue> get_metric(uint64_t key) const
    {
        std::size_t idx   = key & (NUM_SHARDS - 1);
        auto&       shard = shards_[idx];

        auto ptr = shard.get_metric(key);
        if (ptr == nullptr)
        {
            return nullptr;
        }

        return ptr;
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
