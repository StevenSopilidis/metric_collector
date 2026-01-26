#include "bucket_ring.hpp"

#include <iostream>
#include <metrics.hpp>
#include <udp_server.hpp>

using namespace metric_collector::aggregation;

int main()
{
    BucketRing<10, 10> ring;
    ring.store<Counter>("stefanos", 10);
    ring.rotate();
    ring.store<Counter>("stefanos", 12);
    auto value = ring.getMetric<Counter>("stefanos");
    if (value.has_value())
    {
        std::cout << std::get<Counter>(value->get().metric).get() << "\n";
    }
}