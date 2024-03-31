#pragma once

#include <memory>
#include <string>

#include "socket_class.h"
#include "socket_headers.h"


namespace socket_wrapper
{

typedef std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> AddrInfoResult;

// Return server address information. Need to get correct bind address.
AddrInfoResult get_serv_info(const char *port, int sock_type = SOCK_STREAM);

// Return client address information.
AddrInfoResult get_client_info(
    const std::string &host, const std::string &port, int sock_type = SOCK_STREAM, int sock_family = AF_UNSPEC);
AddrInfoResult get_client_info(
    const std::string &host, unsigned short port, int sock_type = SOCK_STREAM, int sock_family = AF_UNSPEC);

// Set SO_REUSE option.
void set_reuse_addr(Socket &sock);

// Accept client, return new socket and write console message.
Socket accept_client(Socket &server_sock);

// Create listening TCP server and return server socket.
Socket create_tcp_server(const char *port);

}  // namespace socket_wrapper
