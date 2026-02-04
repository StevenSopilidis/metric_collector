#include "shard.hpp"

namespace metric_collector::aggregation
{

std::shared_ptr<MetricValue> Shard::get_metric(uint64_t key) const
{
    std::lock_guard lock(mutex_);

    auto it = metrics_.find(key);
    if (it == metrics_.end())
    {
        return nullptr;
    }
    return it->second;
}

} // namespace metric_collector::aggregation
