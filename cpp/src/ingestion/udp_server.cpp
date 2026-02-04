#include "udp_server.hpp"

#include <cstring>
#include <iostream>
#include <span>
#include <utility>

namespace metric_collector::ingestion
{
UdpServer::UdpServer(uint16_t port, std::string addr) : port_(port), addr_(std::move(addr))
{
    init_buffers();
    init_listen_socket();
    init_epoll_socket();
}

UdpServer::~UdpServer()
{
    if (listen_fd_ >= 0)
    {
        close(listen_fd_);
    }

    if (epoll_fd_ >= 0)
    {
        close(epoll_fd_);
    }
}

void UdpServer::init_epoll_socket()
{
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1)
    {
        std::cerr << "---> epoll_create1() failed: " << strerror(errno) << "\n";
        throw std::runtime_error("epoll_create1() failed");
    }

    struct epoll_event event;
    event.events  = EPOLLIN | EPOLLET;
    event.data.fd = listen_fd_;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &event) == -1)
    {
        std::cerr << "---> epoll_ctl() failed: " << strerror(errno) << "\n";
        throw std::runtime_error("epoll_ctl() failed");
    }
}

void UdpServer::init_listen_socket()
{
    listen_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_fd_ < 0)
    {
        std::cerr << "---> socket() failed: " << strerror(errno) << "\n";
        throw std::runtime_error("socket() failed");
    }

    // allow reuse of port and addr
    int yes = 1;
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        std::cerr << "---> setsockopt(SO_REUSEADDR) failed: " << strerror(errno) << "\n";
        throw std::runtime_error("socket() failed");
    }
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) < 0)
    {
        std::cerr << "---> setsockopt(SO_REUSEPORT) failed: " << strerror(errno) << "\n";
        throw std::runtime_error("socket() failed");
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port_);
    inet_pton(AF_INET, addr_.data(), &addr.sin_addr);

    if (bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "---> socket() failed: " << strerror(errno) << "\n";
        throw std::runtime_error("socket() failed");
    }

    if (set_non_blocking(listen_fd_) < 0)
    {
        std::cerr << "---> set_non_blocking() failed: " << strerror(errno) << "\n";
        throw std::runtime_error("set_non_blocking() failed");
    }
}

int UdpServer::set_non_blocking(int fd)
{
    auto flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void UdpServer::stop()
{
    std::cout << "---> Shutting down server\n";
    running_.store(false);
}

void UdpServer::run()
{
    std::cout << "---> Server starting at: " << addr_ << "\n";
    running_.store(true);

    while (running_.load(std::memory_order_acquire))
    {
        int n = epoll_wait(epoll_fd_, events_.data(), static_cast<int>(events_.size()), 100);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            std::cerr << "---> epoll_wait() failed with error: " << strerror(errno) << "\n";
            break;
        }

        if (n == 0)
        {
            continue; // no packet received
        }

        for (std::size_t i = 0; i < n; i++)
        {
            if (events_[i].data.fd != listen_fd_)
            {
                continue;
            }

            drain_socket();
        }
    }
}

void UdpServer::drain_socket()
{
    while (true)
    {
        int r = recvmmsg(listen_fd_, msgs_.data(), BATCH_SIZE, MSG_DONTWAIT, nullptr);
        if (r < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break; // no more data
            }

            std::cerr << "---> recvmmsg() failed" << strerror(errno) << "\n";
        }

        if (r == 0)
        {
            break;
        }

        process_packets(r);
    }
}

void UdpServer::process_packets(size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        std::span<std::byte> packet{buffers_[i].data(), msgs_[i].msg_len}; // received data

        // TODO process packet

        (void)packet;
    }
}

void UdpServer::init_buffers()
{
    for (size_t i = 0; i < BATCH_SIZE; i++)
    {
        iovecs_[i].iov_base = buffers_[i].data();
        iovecs_[i].iov_len  = buffers_[i].size();

        msgs_[i].msg_hdr.msg_iov    = &iovecs_[i];
        msgs_[i].msg_hdr.msg_iovlen = 1;

        msgs_[i].msg_hdr.msg_name    = &peers_[i];
        msgs_[i].msg_hdr.msg_namelen = sizeof(sockaddr_storage);
    }
}
} // namespace metric_collector::ingestion
