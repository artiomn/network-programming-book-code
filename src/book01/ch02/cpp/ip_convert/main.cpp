#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>


int main()
{
    socket_wrapper::SocketWrapper sock_wrap;
    // IPv4.
    struct sockaddr_in sa;
    // IPv6.
    struct sockaddr_in6 sa6;
    // IPv4.
    inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr));
    // IPv6.
    inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr));

    // Буфер для строкового представления IPv4.
    char ip4[INET_ADDRSTRLEN];

    // Сконвертировать адрес в строку типа char*.
    inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);

    // IPv6:
    // Буфер для строкового представления IPv6.
    char ip6[INET6_ADDRSTRLEN];

    inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);

    std::cout << "IP6 Address: " << ip6 << ", IP4 Address " << ip4 << std::endl;

    return EXIT_SUCCESS;
}
