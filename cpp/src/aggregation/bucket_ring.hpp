#ifndef METRIC_COLLECTOR_AGGREGATION_BUCKET_RING_HPP
#define METRIC_COLLECTOR_AGGREGATION_BUCKET_RING_HPP

#include "hash.hpp"

#include <array>
#include <bucket.hpp>
#include <cstddef>

namespace metric_collector::aggregation
{
template <std::size_t RING_SIZE, std::size_t SHARDS_PER_BUCKET> class BucketRing
{
  public:
    static_assert(RING_SIZE > 0, "ring size must be positive number");
    static_assert(SHARDS_PER_BUCKET > 0, "Num of shards must be positive number");
    static_assert(SHARDS_PER_BUCKET % 2 == 0, "Num of shards must be power of two");

    BucketRing() = default;

    void rotate()
    {
        current_bucket_ = (current_bucket_ + 1) % RING_SIZE;
        buckets_[current_bucket_].clear();
    }

    template <MetricTypeConcept T> void store(std::string_view name, uint64_t delta)
    {
        uint64_t key = hash_fnv1a(name.data(), name.length());
        buckets_[current_bucket_].template addMetric<T>(key, delta);
    }

    template <MetricTypeConcept T>
    [[nodiscard]] std::optional<std::reference_wrapper<const MetricValue>>
    getMetric(std::string_view name) const
    {
        uint64_t key = hash_fnv1a(name.data(), name.length());

        for (std::size_t offset = 0; offset < RING_SIZE; ++offset)
        {
            // Walk backwards from current_bucket_
            std::size_t idx = (current_bucket_ + RING_SIZE - offset) % RING_SIZE;

            auto val = buckets_[idx].template getMetric<T>(key);
            if (val.has_value())
            {
                return val;
            }
        }

        return std::nullopt;
    }

  private:
    std::size_t                                      current_bucket_{0};
    std::array<Bucket<SHARDS_PER_BUCKET>, RING_SIZE> buckets_;
};
} // namespace metric_collector::aggregation

#endif