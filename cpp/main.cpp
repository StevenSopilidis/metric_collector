#include <iostream>
#include <udp_server.hpp>


int main()
{
    auto server = new metric_collector::aggregation::UdpServer(8080, "0.0.0.0");
    server->run();
}