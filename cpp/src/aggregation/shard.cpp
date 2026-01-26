#include "shard.hpp"

namespace metric_collector::aggregation
{

std::optional<std::reference_wrapper<const MetricValue>> Shard::getMetric(uint64_t key) const
{
    std::lock_guard lock(mutex_);

    auto it = metrics_.find(key);
    if (it == metrics_.end())
    {
        return std::nullopt;
    }
    return std::make_optional(std::cref(*it->second));
}

} // namespace metric_collector::aggregation
