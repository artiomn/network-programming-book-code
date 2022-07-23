#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


namespace
{

const size_t buffer_size = 255;

} // namespace


int main(int argc, const char * const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        auto servinfo = socket_wrapper::get_serv_info(argv[1], SOCK_DGRAM);

        socket_wrapper::Socket sock = {servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol};

        if (bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            throw std::logic_error("bind");
        }

        std::vector<char> data_buff(buffer_size);
        // For the TTL.
        std::vector<char> ancil_data_buff(CMSG_SPACE(sizeof(uint32_t)));

        iovec iov[1] = { { &data_buff[0], data_buff.size() }  };

        msghdr msgh =
        {
            .msg_name = nullptr, .msg_namelen = 0,
            .msg_iov = iov, .msg_iovlen = 1,
            .msg_control = &ancil_data_buff[0],
            .msg_controllen = ancil_data_buff.size(),
            .msg_flags = 0
        };

        int recv_ttl = 1;

        // Enable IP_RECVTTL option.
        if (setsockopt(sock, IPPROTO_IP, IP_RECVTTL, &recv_ttl, sizeof(recv_ttl)) != 0)
        {
            throw std::logic_error("Set IP_RECVTTL");
        }

        // Receive auxiliary data in msgh.
        for (ssize_t n = recvmsg(sock, &msgh, 0); n; n = recvmsg(sock, &msgh, 0))
        {
            if (n < 0)
            {
                throw std::logic_error("recvmsg");
            }

            std::cout
                << n
                << " bytes was read: " << std::string(data_buff.begin(), data_buff.begin() + n)
                << std::endl;

            cmsghdr *cmsg;

            // Get ancillary data.
            for (cmsg = CMSG_FIRSTHDR(&msgh); cmsg != nullptr;
                 cmsg = CMSG_NXTHDR(&msgh, cmsg))
            {
                if (IPPROTO_IP == cmsg->cmsg_level && IP_TTL == cmsg->cmsg_type)
                {
                    uint32_t received_ttl = 0;
                    std::copy(CMSG_DATA(cmsg), CMSG_DATA(cmsg) + sizeof(received_ttl), &received_ttl);

                    std::cout
                        << "IP_RECVTTL was received: " << received_ttl
                        << std::endl;
                    break;
                }
            }

            if (nullptr == cmsg)
            {
                // Error: IP_TTL was not enabled or small buffer or I/O error.
                throw std::logic_error("IP_RECVTTL receiving");
            }

            break;
        }

    }
    catch (const std::logic_error &e)
    {
        std::cerr
            << e.what()
            << ": " << sock_wrap.get_last_error_string() << "!"
            << std::endl;
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

