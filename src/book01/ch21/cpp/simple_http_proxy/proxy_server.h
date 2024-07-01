#pragma once

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <string>
#include <thread>
#include <tuple>
#include <utility>


class ProxyServer final
{
public:
    explicit ProxyServer(std::uint16_t port = 8080);

public:
    void start();
    void stop() noexcept;

private:
    using uri_data = std::tuple<std::string, std::string, std::uint16_t>;

    std::string read_line(const socket_wrapper::Socket &sock) const;

    // Send HTML error message to the client.
    void client_error(
        const socket_wrapper::Socket &sock, const std::string &cause, int err_num, const std::string &short_message,
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
    std::pair<std::string, std::string> parse_request_headers(const socket_wrapper::Socket &s) const;

    /*     Core part of the proxy server that parses the request from the client,
            forwards it to the server and sends the response (cached or real-time)
            back to the client.
        The proxy is LRU cache enabled with synchronization.
    */
    void proxify(socket_wrapper::Socket client_socket);

private:
    bool try_to_connect(const socket_wrapper::Socket &s, const sockaddr *sa, size_t sa_size);
    socket_wrapper::Socket connect_to_target_server(const std::string &host_name, std::uint16_t port);

private:
    const std::uint16_t port_ = 0;
    socket_wrapper::SocketWrapper sock_wrap_;
    socket_wrapper::Socket sock_;
};
