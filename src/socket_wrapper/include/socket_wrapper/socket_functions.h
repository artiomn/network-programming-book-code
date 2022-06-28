#pragma once

#include <memory>

#include "socket_class.h"


namespace socket_wrapper
{

// Return server address information. Need to get correct bind address.
std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> get_serv_info(const char *port);

// Set SO_REUSE option.
void set_reuse_addr(Socket &sock);

// Accept client, return new socket and write console message.
Socket accept_client(Socket &server_sock);
}
