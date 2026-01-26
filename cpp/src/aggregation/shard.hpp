#ifndef METRIC_COLLECTOR_AGGREGATION_SHARD_HPP
#define METRIC_COLLECTOR_AGGREGATION_SHARD_HPP

#include "metrics.hpp"

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace metric_collector::aggregation
{
class Shard
{
  public:
    Shard() = default;

    void clear()
    {
        std::lock_guard lock(mutex_);
        metrics_.clear();
    }

    std::optional<std::reference_wrapper<const MetricValue>> getMetric(uint64_t key) const;

    template <MetricTypeConcept T> MetricValue* store(uint64_t key)
    {
        std::lock_guard lock(mutex_);

        auto it = metrics_.find(key);
        if (it != metrics_.end())
        {
            if (!std::holds_alternative<T>(it->second->metric))
            {
                return nullptr; // type mismatch
            }
            return it->second.get();
        }

        auto ptr        = std::make_unique<MetricValue>(std::in_place_type<T>);
        auto return_ptr = ptr.get();
        metrics_.emplace(key, std::move(ptr));
        return return_ptr;
    }

  private:
    std::unordered_map<uint64_t, std::unique_ptr<MetricValue>> metrics_;

    mutable std::mutex mutex_;
};
} // namespace metric_collector::aggregation

#endif