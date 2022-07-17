#include <cassert>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>


namespace socket_wrapper
{

std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>
get_serv_info(const char *port, int sock_type)
{
    struct addrinfo hints =
    {
        .ai_flags = AI_PASSIVE,
        .ai_family = AF_INET,
        .ai_socktype = sock_type,
        .ai_protocol = (sock_type == SOCK_STREAM ? IPPROTO_TCP : IPPROTO_UDP)
    };
    struct addrinfo *s_i;
    int ai_status;

    if ((ai_status = getaddrinfo(nullptr, port, &hints, &s_i)) != 0)
    {
        throw std::logic_error(gai_strerror(ai_status));
    }

    return std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>(s_i, freeaddrinfo);
}


void set_reuse_addr(Socket &sock)
{
    const int flag = 1;

    // Allow reuse of port.
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&flag), sizeof(flag)) < 0)
    {
        throw std::logic_error("Set SO_REUSEADDR error");
    }
}


Socket accept_client(socket_wrapper::Socket &server_sock)
{
    struct sockaddr_storage client_addr;
    socklen_t client_addr_length = sizeof(client_addr);
    std::array<char, INET_ADDRSTRLEN> addr;

    Socket client_sock(accept(server_sock, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_length));

    if (!client_sock)
    {
        throw std::logic_error("accept");
    }

    assert(sizeof(sockaddr_in) == client_addr_length);

    std::cout
        << "Client from "
        << inet_ntop(AF_INET, &(reinterpret_cast<const sockaddr_in * const>(&client_addr)->sin_addr), &addr[0], addr.size())
        << "..."
        << std::endl;

    return client_sock;
}

}
