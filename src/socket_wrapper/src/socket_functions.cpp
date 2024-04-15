#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>

#include <array>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>


namespace socket_wrapper
{

AddrInfoResult get_serv_info(const char *port, int sock_type)
{
    struct addrinfo hints = {
        .ai_flags = AI_PASSIVE | AI_NUMERICSERV,
        .ai_family = AF_INET,
        .ai_socktype = sock_type,
        .ai_protocol = (sock_type == SOCK_STREAM ? IPPROTO_TCP : IPPROTO_UDP)};

    struct addrinfo *s_i;
    int ai_status;

    if ((ai_status = getaddrinfo(nullptr, port, &hints, &s_i)) != 0)
    {
        throw std::logic_error(gai_strerror(ai_status));
    }

    return std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>(s_i, freeaddrinfo);
}


AddrInfoResult get_client_info(const std::string &host, const std::string &port, int sock_type, int sock_family)
{
    struct addrinfo hints = {
        .ai_flags = AI_CANONNAME | AI_NUMERICSERV,
        .ai_family = sock_family,
        .ai_socktype = sock_type,
        .ai_protocol = (sock_type == SOCK_STREAM ? IPPROTO_TCP : IPPROTO_UDP)};
    struct addrinfo *c_i;
    int ai_status;

    if ((ai_status = getaddrinfo(host.c_str(), port.c_str(), &hints, &c_i)) != 0)
    {
        throw std::logic_error(gai_strerror(ai_status));
    }

    return std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>(c_i, freeaddrinfo);
}


AddrInfoResult get_client_info(const std::string &host, unsigned short port, int sock_type, int sock_family)
{
    return get_client_info(host, std::to_string(port).c_str(), sock_type, sock_family);
}


void set_reuse_addr(Socket &sock)
{
    const int flag = 1;

    // Allow reuse of port.
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&flag), sizeof(flag)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Set SO_REUSEADDR error");
    }
}


Socket accept_client(socket_wrapper::Socket &server_sock)
{
    struct sockaddr_storage client_addr;
    socklen_t client_addr_length = sizeof(client_addr);
    std::array<char, INET_ADDRSTRLEN> addr;

    Socket client_sock(accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr), &client_addr_length));

    if (!client_sock)
    {
        throw std::system_error(errno, std::system_category(), "accept");
    }

    assert(sizeof(sockaddr_in) == client_addr_length);

    std::cout << "Client from "
              << inet_ntop(
                     AF_INET, &(reinterpret_cast<const sockaddr_in *const>(&client_addr)->sin_addr), &addr[0],
                     addr.size())
              << "..." << std::endl;

    return client_sock;
}


Socket create_tcp_server(const char *port)
{
    auto servinfo = get_serv_info(port);
    Socket server_sock = {servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol};

    if (!server_sock)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    set_reuse_addr(server_sock);

    if (-1 == bind(server_sock, servinfo->ai_addr, servinfo->ai_addrlen))
    {
        throw std::system_error(errno, std::system_category(), "bind");
    }

    if (-1 == listen(server_sock, SOMAXCONN))
    {
        throw std::system_error(errno, std::system_category(), "listen");
    }

    return std::move(server_sock);
}

}  // namespace socket_wrapper
