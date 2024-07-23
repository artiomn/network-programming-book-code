extern "C"
{
#include <net/if.h>
#include <sys/ioctl.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>


int main()
{
    ifconf ifc = {0};

    const socket_wrapper::SocketWrapper sock_wrap;

    const auto sock = socket_wrapper::Socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    if (ioctl(sock, SIOCGIFCONF, &ifc) < 0)
    {
        throw std::system_error(errno, std::system_category(), "SIOCGIFCONF with 0 buffer");
    }

    std::cout << "Buffer size = " << ifc.ifc_len << std::endl;
    assert(ifc.ifc_len);

    std::vector<char> buffer(ifc.ifc_len);

    ifc.ifc_buf = buffer.data();

    if (ioctl(sock, SIOCGIFCONF, &ifc) < 0)
    {
        throw std::system_error(errno, std::system_category(), "SIOCGIFCONF");
    }

    for (auto i = ifc.ifc_req; i != ifc.ifc_req + ifc.ifc_len / sizeof(ifreq); ++i)
    {
        std::cout << i->ifr_name << ": ";
        if (AF_INET == i->ifr_addr.sa_family)
        {
            std::array<char, INET_ADDRSTRLEN> buf;
            std::cout << inet_ntop(
                AF_INET, &(reinterpret_cast<sockaddr_in*>(&i->ifr_addr)->sin_addr), buf.data(), buf.size());
        }
        else if (AF_INET6 == i->ifr_addr.sa_family)
        {
            std::array<char, INET6_ADDRSTRLEN> buf;
            std::cout << inet_ntop(
                AF_INET6, &(reinterpret_cast<sockaddr_in6*>(&i->ifr_addr)->sin6_addr), buf.data(), buf.size());
        }

        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}
