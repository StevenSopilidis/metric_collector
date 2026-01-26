#ifndef METRIC_COLLECTOR_AGGREGATION_HASH_HPP
#define METRIC_COLLECTOR_AGGREGATION_HASH_HPP

#include <cstdint>

namespace metric_collector::aggregation
{

uint64_t hash_fnv1a(const char* data, std::size_t len)
{
    uint64_t hash = 14695981039346656037ULL; // FNV offset
    for (std::size_t i = 0; i < len; i++)
    {
        hash ^= static_cast<uint64_t>(data[i]);
        hash *= 1099511628211ULL; // FNV prime
    }
    return hash;
}

} // namespace metric_collector::aggregation

#endif