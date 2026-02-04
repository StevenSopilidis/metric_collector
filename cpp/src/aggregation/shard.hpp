#ifndef METRIC_COLLECTOR_AGGREGATION_SHARD_HPP
#define METRIC_COLLECTOR_AGGREGATION_SHARD_HPP

#include "metrics.hpp"

#include <memory>
#include <mutex>
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

    [[nodiscard]] std::shared_ptr<MetricValue> get_metric(uint64_t key) const;

    template <MetricTypeConcept T> std::shared_ptr<MetricValue> store(uint64_t key)
    {
        std::lock_guard lock(mutex_);

        auto it = metrics_.find(key);
        if (it != metrics_.end())
        {
            if (!std::holds_alternative<T>(it->second->metric))
            {
                return nullptr; // type mismatch
            }
            return it->second;
        }

        auto ptr = std::make_shared<MetricValue>(std::in_place_type<T>);
        metrics_.emplace(key, ptr);
        return ptr;
    }

  private:
    std::unordered_map<uint64_t, std::shared_ptr<MetricValue>> metrics_;
    mutable std::mutex                                         mutex_;
};
} // namespace metric_collector::aggregation

#endif