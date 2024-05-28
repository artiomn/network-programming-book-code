#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <array>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>


int print_ips_with_getaddrinfo(const std::string& host_name)
{
    // Need for Windows initialization.
    socket_wrapper::SocketWrapper sock_wrap;

    std::cout << "Getting name for \"" << host_name << "\"...\n"
              << "Using getaddrinfo() function." << std::endl;

    addrinfo hints = {
        .ai_flags = AI_CANONNAME,
        // Неважно, IPv4 или IPv6.
        .ai_family = AF_UNSPEC,
        // TCP stream-sockets.
        .ai_socktype = SOCK_STREAM,
        // Any protocol.
        .ai_protocol = 0};

    // Results.

    addrinfo* s_i = nullptr;

    if (int status = getaddrinfo(host_name.c_str(), nullptr, &hints, &s_i); status != 0)
    {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return EXIT_FAILURE;
    }

    const socket_wrapper::AddrInfoResult servinfo(s_i, freeaddrinfo);

    for (auto const* s = servinfo.get(); s != nullptr; s = s->ai_next)
    {
        std::cout << "Canonical name: ";
        if (s->ai_canonname) std::cout << s->ai_canonname;
        std::cout << "\n";

        assert(s->ai_family == s->ai_addr->sa_family);
        std::cout << "Address type: ";

        if (AF_INET == s->ai_family)
        {
            std::array<char, INET_ADDRSTRLEN> ip;
            std::cout << "AF_INET\n";
            sockaddr_in const* const sin = reinterpret_cast<const sockaddr_in* const>(s->ai_addr);
            std::cout << "Address length: " << sizeof(sin->sin_addr) << "\n";
            std::cout << "IP Address: " << inet_ntop(AF_INET, &(sin->sin_addr), ip.data(), ip.size()) << "\n";
        }
        else if (AF_INET6 == s->ai_family)
        {
            std::array<char, INET6_ADDRSTRLEN> ip6;
            std::cout << "AF_INET6\n";
            const sockaddr_in6* const sin = reinterpret_cast<const sockaddr_in6* const>(s->ai_addr);
            std::cout << "Address length: " << sizeof(sin->sin6_addr) << "\n";
            std::cout << "IP Address: " << inet_ntop(AF_INET6, &(sin->sin6_addr), ip6.data(), ip6.size()) << "\n";
        }
        else
        {
            std::cout << s->ai_family << "\n";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    return EXIT_SUCCESS;
}


int print_ips_with_gethostbyname(const std::string& host_name)
{
    std::cout << "Getting name for \"" << host_name << "\"...\n"
              << "Using gethostbyname() function." << std::endl;

    socket_wrapper::SocketWrapper sock_wrap;
    const hostent* remote_host{gethostbyname(host_name.c_str())};

    if (nullptr == remote_host)
    {
        if (sock_wrap.get_last_error_code())
        {
            std::cerr << sock_wrap.get_last_error_string() << std::endl;
        }

        return EXIT_FAILURE;
    }

    std::cout << "Official name: " << remote_host->h_name << "\n";

    for (const char* const* p_alias = const_cast<const char* const*>(remote_host->h_aliases); *p_alias; ++p_alias)
    {
        std::cout << "# Alternate name: \"" << *p_alias << "\"\n";
    }

    std::cout << "Address type: ";
    if (AF_INET == remote_host->h_addrtype)
    {
        std::cout << "AF_INET\n";
        std::cout << "\nAddress length: " << remote_host->h_length << "\n";

        in_addr addr = {0};

        for (int i = 0; remote_host->h_addr_list[i]; ++i)
        {
            addr.s_addr = *reinterpret_cast<const u_long* const>(remote_host->h_addr_list[i]);
            std::cout << "IP Address: " << inet_ntoa(addr) << "\n";
        }
    }
    else if (AF_INET6 == remote_host->h_addrtype)
    {
        std::cout << "AF_INET6\n";
    }
    else
    {
        std::cout << remote_host->h_addrtype << "\n";
    }

    std::cout << std::endl;

    return EXIT_SUCCESS;
}


int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <hostname>" << std::endl;
        return EXIT_FAILURE;
    }

    const std::string host_name = {argv[1]};

    print_ips_with_getaddrinfo(host_name);
    // This slows down frequency of requests to DNS
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    print_ips_with_gethostbyname(host_name);

    return EXIT_SUCCESS;
}
