#include "bucket_ring.hpp"

#include <iostream>
#include <metrics.hpp>
#include <udp_server.hpp>

using namespace metric_collector::aggregation;

int main()
{
    BucketRing<10, 8> ring;
    ring.store<Counter>("stefanos", 10);
    ring.rotate();
    ring.store<Counter>("stefanos", 12);
    auto ptr = ring.get_metric<Counter>("stefanos");
    if (ptr != nullptr)
    {
        std::cout << std::get<Counter>(ptr->metric).get() << "\n";
    }
}