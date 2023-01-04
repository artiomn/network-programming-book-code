#pragma once

#include <regex>
#include <tuple>
#include <thread>
#include <iostream>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_class.h>


class ProxyServer final
{
public:
    static const std::regex uri_regexp;

    const size_t back_log = 10;
    const unsigned short default_target_port = 80;

    typedef std::tuple<std::string, std::string, unsigned short> uri_data;

public:
    ProxyServer(unsigned short port = 8080);
    ~ProxyServer();

public:
    void start();
    void stop();

public:
    std::string read_line(socket_wrapper::Socket &sock) const;

    // Send HTML error message to the client.
    void client_error(socket_wrapper::Socket &sock, const std::string &cause, int err_num, const std::string &short_message,
                      const std::string &long_message) const;
    // Extract the host name, path to the resource and the port number from the URL.
    uri_data parse_uri(const std::string &uri) const;

    /*
     * Reads the request headers from the client and generates a new request header
     * header to send to the host that the client is trying to access.
     * Ensures that the headers follow the writup specification.
     * If the host header is present in the reqest headers frm the client it copies
     * it directly. Else it generates it using the hostname and adds it. 
     */
    std::tuple<std::string, std::string> parse_request_headers(socket_wrapper::Socket &s) const;

    /*     Core part of the proxy server that parses the request from the client, 
            forwards it to the server and sends the response (cached or real-time)  
            back to the client.
        The proxy is LRU cache enabled with synchronization.
    */
    void proxify(socket_wrapper::Socket client_socket);

    unsigned short get_port() const { return port_; }

private:
    bool try_to_connect(socket_wrapper::Socket &s, const sockaddr* sa, size_t sa_size);
    socket_wrapper::Socket connect_to_target_server(const std::string &host_name, unsigned short port);

private:
    unsigned short port_;
    bool started_ = false;
    socket_wrapper::SocketWrapper sock_wrap_;
    socket_wrapper::Socket sock_;
};

