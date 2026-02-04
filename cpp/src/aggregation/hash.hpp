#ifndef METRIC_COLLECTOR_AGGREGATION_HASH_HPP
#define METRIC_COLLECTOR_AGGREGATION_HASH_HPP

#include <cstdint>

namespace metric_collector::aggregation
{

constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
constexpr uint64_t FNV_PRIME  = 1099511628211ULL;

[[nodiscard]] inline constexpr uint64_t hash_fnv1a(const char* data, std::size_t len)
{
    uint64_t hash = FNV_OFFSET;
    for (std::size_t i = 0; i < len; i++)
    {
        hash ^= static_cast<uint64_t>(static_cast<unsigned char>(data[i]));
        hash *= FNV_PRIME;
    }
    return hash;
}

} // namespace metric_collector::aggregation

#endif