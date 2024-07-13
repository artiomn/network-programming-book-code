extern "C"
{
#include <netinet/ip.h>
// sock_extended_err.
#include <linux/errqueue.h>
#include <linux/tcp.h>
#include <poll.h>
}

#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <array>
#include <cinttypes>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>


std::pair<uint32_t, uint32_t> read_notification(const msghdr *msg)
{
    const cmsghdr *cm = CMSG_FIRSTHDR(msg);

    if (cm->cmsg_level != SOL_IP && cm->cmsg_type != IP_RECVERR)
        throw std::system_error(errno, std::system_category(), "cmsg");

    const sock_extended_err *serr = reinterpret_cast<const sock_extended_err *>(CMSG_DATA(cm));

    if (serr->ee_errno != 0 || serr->ee_origin != SO_EE_ORIGIN_ZEROCOPY)
        throw std::system_error(errno, std::system_category(), "recvmsg");

    return std::make_pair(serr->ee_data, serr->ee_info);
}


int main()
{
    const socket_wrapper::SocketWrapper sock_wrap;
    const int sock_fd{socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)};

    if (!sock_fd) return EXIT_FAILURE;

    try
    {
        const int one = 1;

        if (setsockopt(sock_fd, SOL_SOCKET, SO_ZEROCOPY, &one, sizeof(one)))
            throw std::system_error(errno, std::system_category(), "setsockopt");

        std::string buf{"test data"};

        auto c_i = socket_wrapper::get_client_info("localhost", "12345");

        if (-1 == connect(sock_fd, c_i->ai_addr, c_i->ai_addrlen))
        {
            throw std::system_error(errno, std::system_category(), "connect");
        }

        if (send(sock_fd, buf.data(), buf.size(), MSG_ZEROCOPY) != buf.size())
            throw std::system_error(errno, std::system_category(), "send");

        pollfd pfd = {.fd = sock_fd, .events = 0};

        if (poll(&pfd, 1, -1) != 1 || !(pfd.revents & POLLERR))
            throw std::system_error(errno, std::system_category(), "poll");

        std::array<char, CMSG_SPACE(sizeof(uint32_t))> ancil_data_buff;
        iovec iov[1] = {{buf.data(), buf.size()}};

        msghdr msg = {
            .msg_name = nullptr,
            .msg_namelen = 0,
            .msg_iov = iov,
            .msg_iovlen = 1,
            .msg_control = &ancil_data_buff[0],
            .msg_controllen = ancil_data_buff.size(),
            .msg_flags = 0};

        if (-1 == recvmsg(sock_fd, &msg, MSG_ERRQUEUE))
            throw std::system_error(errno, std::system_category(), "recvmsg");

        auto [data, info] = read_notification(&msg);
        std::cout << "Data: " << data << ", Info: " << info << std::endl;
        close(sock_fd);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        close(sock_fd);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
