#pragma once

#include <memory>
#include <string>

#include "socket_class.h"
#include "socket_headers.h"


namespace socket_wrapper
{

using AddrInfoResult = std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>;

// Return server address information. Need to get correct bind address.
AddrInfoResult get_serv_info(const std::string &port, int sock_type = SOCK_STREAM);

// Return client address information.
AddrInfoResult get_client_info(
    const std::string &host, const std::string &port, int sock_type = SOCK_STREAM, int sock_family = AF_INET);
AddrInfoResult get_client_info(
    const std::string &host, unsigned short port, int sock_type = SOCK_STREAM, int sock_family = AF_INET);

// Set SO_REUSE option.
void set_reuse_addr(Socket &sock);

// Accept client, return new socket and write console message.
Socket accept_client(Socket &server_sock);

// Create listening TCP server and return server socket.
Socket create_tcp_server(const std::string &port);

}  // namespace socket_wrapper
