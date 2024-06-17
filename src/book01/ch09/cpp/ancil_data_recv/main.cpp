#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


int main(int argc, const char *const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    const socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        assert(argv[1]);
        auto servinfo = socket_wrapper::get_serv_info(argv[1], SOCK_DGRAM);

        const socket_wrapper::Socket sock = {servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol};

        if (bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            throw std::system_error(sock_wrap.get_last_error_code(), std::system_category(), "bind");
        }

        int recv_ttl = 1;

        // Enable IP_RECVTTL option.
        if (setsockopt(sock, IPPROTO_IP, IP_RECVTTL, &recv_ttl, sizeof(recv_ttl)) != 0)
        {
            throw std::system_error(sock_wrap.get_last_error_code(), std::system_category(), "Set IP_RECVTTL");
        }

        std::array<char, 255> data_buff;
        // For the TTL.
        std::array<char, CMSG_SPACE(sizeof(uint32_t))> ancil_data_buff;

        iovec iov[1] = {{&data_buff[0], data_buff.size()}};

        msghdr msgh = {
            .msg_name = nullptr,
            .msg_namelen = 0,
            .msg_iov = iov,
            .msg_iovlen = 1,
            .msg_control = &ancil_data_buff[0],
            .msg_controllen = ancil_data_buff.size(),
            .msg_flags = 0};

        // Receive auxiliary data in msgh.
        for (ssize_t n = recvmsg(sock, &msgh, 0); n; n = recvmsg(sock, &msgh, 0))
        {
            if (n < 0)
            {
                throw std::system_error(sock_wrap.get_last_error_code(), std::system_category(), "recvmsg");
            }

            std::cout << n << " bytes was read: " << std::string(data_buff.begin(), data_buff.begin() + n) << std::endl;

            cmsghdr *cmsg = nullptr;

            // Get ancillary data.
            for (cmsg = CMSG_FIRSTHDR(&msgh); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msgh, cmsg))
            {
                if (IPPROTO_IP == cmsg->cmsg_level && IP_TTL == cmsg->cmsg_type)
                {
                    uint32_t received_ttl = 0;
                    std::copy(CMSG_DATA(cmsg), CMSG_DATA(cmsg) + sizeof(received_ttl), &received_ttl);

                    std::cout << "IP_RECVTTL was received: " << received_ttl << std::endl;
                    break;
                }
            }

            if (nullptr == cmsg)
            {
                // Error: IP_TTL was not enabled or small buffer or I/O error.
                throw std::system_error(
                    sock_wrap.get_last_error_code(), std::system_category(), "IP_RECVTTL receiving");
            }

            break;
        }
    }
    catch (const std::system_error &e)
    {
        std::cerr << e.what() << ": " << sock_wrap.get_last_error_string() << "!" << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
