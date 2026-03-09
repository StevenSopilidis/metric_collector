#include "bucket_ring.hpp"

#include <iostream>
#include <metrics.hpp>
#include <udp_server.hpp>

using namespace metric_collector::aggregation;

int main() { auto server = new metric_collector::ingestion::UdpServer(8080, "localhost", 10); }