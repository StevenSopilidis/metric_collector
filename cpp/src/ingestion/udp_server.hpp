#ifndef METRIC_COLLECTOR_INGESTION_UDP_SERVER_HPP
#define METRIC_COLLECTOR_INGESTION_UDP_SERVER_HPP

#include <arpa/inet.h>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

constexpr std::size_t MAX_EVENTS = 64;
constexpr std::size_t BATCH_SIZE = 64;
constexpr std::size_t MAX_PACKET = 512;

namespace metric_collector::aggregation
{
class UdpServer
{
  public:
    explicit UdpServer(uint16_t port, std::string addr);

    ~UdpServer();

    void run();
    void stop();

  private:
    void init_buffers();

    inline void       init_listen_socket();
    inline void       init_epoll_socket();
    static inline int set_non_blocking(int fd);

    int               listen_fd_;
    int               epoll_fd_;
    uint16_t          port_;
    std::string       addr_;
    std::atomic<bool> running_;

    std::array<std::array<std::byte, MAX_PACKET>, BATCH_SIZE> buffers_;
    std::array<iovec, BATCH_SIZE>                             iovecs_;
    std::array<mmsghdr, BATCH_SIZE>                           msgs_;
    std::array<sockaddr_storage, BATCH_SIZE>                  peers_;
    std::array<epoll_event, MAX_EVENTS>                       events_;
};
}; // namespace metric_collector::aggregation

#endif