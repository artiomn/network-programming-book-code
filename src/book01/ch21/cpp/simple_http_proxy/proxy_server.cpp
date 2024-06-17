#include "proxy_server.h"

#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <array>
#include <cassert>
#include <exception>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <thread>
#include <utility>


// https://stackoverflow.com/questions/27745/getting-parts-of-a-url-regex .
const std::regex uri_regexp(
    R"_((?:([^\:]*)\://)?(?:([^\:\@]*)(?:\:([^\@]*))?\@)?(?:([^/\:]*)\.(?=[^\./\:]*\.[^\./\:]*))?([^\./\:]*)(?:\.([^/\.\:]*))?(?:\:([0-9]*))?(/[^\?#]*(?=.*?/)/)?([^\?#]*)?(?:\?([^#]*))?(?:#(.*))?)_",
    std::regex_constants::ECMAScript | std::regex_constants::icase);

constexpr size_t back_log = 10;
constexpr std::uint16_t default_target_port = 80;

// Necessary headers.
constexpr char user_agent_hdr[] =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:93.0) Gecko/20100101 Firefox/93.0\r\n";
constexpr char accept_hdr[] = "Accept: text/html,application/xhtml+xml,application/xml;\r\n";
constexpr char accept_encoding_hdr[] = "Accept-Encoding: identity\r\n";
constexpr char connection_hdr[] = "Connection: close\r\n";
constexpr char proxy_conn_hdr[] = "Proxy-Connection: close\r\n";


inline std::string &rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());

    return s;
}


ProxyServer::ProxyServer(std::uint16_t port) : port_(port), sock_(AF_INET, SOCK_STREAM, IPPROTO_TCP)
{
    if (!sock_)
    {
        throw std::logic_error(sock_wrap_.get_last_error_string());
    }
}


std::string ProxyServer::read_line(const socket_wrapper::Socket &s) const
{
    std::string result;

    for (;;)
    {
        char ch = 0;
        const ssize_t read_bytes = ::recv(s, &ch, 1, 0);

        if (-1 == read_bytes)
        {
#if !defined(_WIN32)
            // Interrupted (by signal, for example), restart read().
            if (EINTR == errno)
                continue;
            else
#endif  // _WIN32
                throw std::system_error(errno, std::system_category(), "recv");
        }
        // EOF.
        else if (0 == read_bytes)
        {
            break;
        }
        else
        {
            result += ch;
            if ('\n' == ch) break;
        }
    }

    return result;
}


void ProxyServer::client_error(
    const socket_wrapper::Socket &sock, const std::string &cause, int err_num, const std::string &short_message,
    const std::string &long_message) const
{
    std::string err_headers = "HTTP/1.0 " + std::to_string(err_num) + " " + short_message + "\r\n";

    // Print the HTTP response.
    if (send(sock, &err_headers.at(0), err_headers.size(), 0) == -1)
    {
        throw std::system_error(errno, std::system_category(), "send");
    }

    err_headers = "Content-type: text/html\r\n";
    if (send(sock, &err_headers.at(0), err_headers.size(), 0) == -1)
    {
        throw std::system_error(errno, std::system_category(), "send");
    }

    std::stringstream err_body_s;
    err_body_s << "<html><title>Proxy Error</title>"
               << "<body bgcolor=0xffffff>\r\n"
               << err_num << ": " << short_message << "\r\n"
               << "<p>" << long_message << ": " << cause << "\r\n"
               << "<hr><em>Example Proxy Server</em>\r\n"
               << "</body></html>\r\n";

    auto err_body = err_body_s.str();

    err_headers = "Content-length: " + std::to_string(err_body.size()) + "\r\n\r\n";
    if (send(sock, &err_headers.at(0), err_headers.size(), 0) == -1)
    {
        throw std::system_error(errno, std::system_category(), "send");
    }
    if (send(sock, &err_body.at(0), err_body.size(), 0) == -1)
    {
        throw std::system_error(errno, std::system_category(), "send");
    }
}


ProxyServer::uri_data ProxyServer::parse_uri(const std::string &uri) const
{
    std::smatch uri_match;
    if (!std::regex_match(uri, uri_match, uri_regexp)) return std::make_tuple("", "./", 0);

    std::stringstream host_name_s;

    if (uri_match[4].matched) host_name_s << uri_match[4] << ".";
    host_name_s << uri_match[5];
    if (uri_match[6].matched) host_name_s << "." << uri_match[6];

    const std::string path = uri_match[9].str();
    std::string host_name = host_name_s.str();
    rtrim(host_name);

    return std::make_tuple(
        host_name, path.size() ? path : "/", uri_match[7].matched ? std::stoi(uri_match[7]) : default_target_port);
}


std::pair<std::string, std::string> ProxyServer::parse_request_headers(const socket_wrapper::Socket &s) const
{
    std::string line;
    std::stringstream result;
    std::string host_name;

    static constexpr char host_header_name[] = "Host: ";

    do
    {
        line = read_line(s);

        if (("\r\n" == line) || ("\n" == line)) continue;

        if (line.find("User-Agent:") != std::string::npos) continue;

        if (line.find("Accept:") != std::string::npos) continue;

        if (line.find("Accept-Encoding:") != std::string::npos) continue;

        if (line.find("Connection:") != std::string::npos) continue;

        if (line.find("Proxy-Connection:") != std::string::npos) continue;

        if (const auto host_pos = line.find(host_header_name); host_pos != std::string::npos)
        {
            host_name = line.substr(host_pos + std::size(host_header_name) - 1);
            continue;
        }

        result << line;
    } while ((line != "\r\n") && (line != "\n"));

    return make_pair(result.str(), host_name);
}


bool ProxyServer::try_to_connect(const socket_wrapper::Socket &s, const sockaddr *sa, size_t sa_size)
{
    if (connect(s, sa, sa_size) == -1)
    {
        std::cerr << "Connection failed: " << sock_wrap_.get_last_error_string() << std::endl;
        return false;
    }

    return true;
}


socket_wrapper::Socket ProxyServer::connect_to_target_server(const std::string &host_name, std::uint16_t port)
{
    const auto servinfo = socket_wrapper::get_client_info(host_name, port);

    for (auto const *s = servinfo.get(); s != nullptr; s = s->ai_next)
    {
        assert(s->ai_family == s->ai_addr->sa_family);
        if (AF_INET == s->ai_family)
        {
            sockaddr_in *const sin = reinterpret_cast<sockaddr_in *const>(s->ai_addr);
            in_addr addr;
            addr.s_addr = *reinterpret_cast<const in_addr_t *>(&sin->sin_addr);

            sin->sin_family = AF_INET;
            sin->sin_port = htons(port);

            std::array<char, INET_ADDRSTRLEN> ip;
            std::cout << "Trying IP Address: " << inet_ntop(AF_INET, &addr, ip.data(), ip.size()) << std::endl;
            socket_wrapper::Socket sock = {AF_INET, SOCK_STREAM, IPPROTO_TCP};

            if (try_to_connect(sock, reinterpret_cast<const sockaddr *>(sin), sizeof(sockaddr_in)))
            {
                return sock;
            }
        }
        else if (AF_INET6 == s->ai_family)
        {
            sockaddr_in6 *const sin = reinterpret_cast<sockaddr_in6 *const>(s->ai_addr);

            sin->sin6_family = AF_INET6;
            sin->sin6_port = htons(port);

            std::array<char, INET6_ADDRSTRLEN> ip6;
            std::cout << "Trying IPv6 Address: " << inet_ntop(AF_INET6, &(sin->sin6_addr), ip6.data(), ip6.size())
                      << std::endl;
            socket_wrapper::Socket sock = {AF_INET6, SOCK_STREAM, IPPROTO_TCP};

            if (try_to_connect(sock, reinterpret_cast<const sockaddr *>(sin), sizeof(sockaddr_in6)))
            {
                return sock;
            }
        }
    }  // for

    throw std::system_error(errno, std::system_category(), "Connection error");
}


void ProxyServer::proxify(socket_wrapper::Socket client_socket)
{
    std::cout << "Waiting for the client request..." << std::endl;

    try
    {
        std::string method;
        std::string uri;
        std::string version;

        auto line = read_line(client_socket);
        std::stringstream ss(rtrim(line));
        ss >> method >> uri >> version;

        std::cout << "Client request: \"" << line << "\" parsed.\n"
                  << "Method = " << method << "\n"
                  << "URI = " << uri << "\n"
                  << "Version = " << version << std::endl;

        // This proxy currently only supports the GET call.
        if (method != "GET")
        {
            client_error(client_socket, method, 501, "Not implemented", "This proxy does not implement this method");
            throw std::invalid_argument("Unknown method \"" + method + "\"");
        }

        // Parse the HTTP request to extract the hostname, path and port.
        auto [host_name, path, port] = parse_uri(uri);

        std::cout << "Host name from the request = " << host_name << "\n"
                  << "Path = " << path << "\n"
                  << "Port from the request = " << port << std::endl;

        auto [new_headers, host_name_from_header] = parse_request_headers(client_socket);

        if (host_name_from_header.size())
        {
            std::tie(host_name, std::ignore, port) = parse_uri(host_name_from_header);
            std::cout << "Host from the header = " << host_name << "\n"
                      << "Port from the header = " << port << std::endl;
        }

        // Generate a new modified HTTP request to forward to the server
        std::stringstream new_request_s;

        new_request_s << "GET " << path << " HTTP/1.1\r\n";

        new_request_s << "Host: " << host_name << "\r\n"
                      << user_agent_hdr << accept_hdr << accept_encoding_hdr << connection_hdr << proxy_conn_hdr
                      << new_headers << "\r\n";

        const std::string &new_request = new_request_s.str();

        std::cout << "\n"
                  << "============\n"
                  << "New request:\n"
                  << "------------\n"
                  << new_request << "============\n"
                  << std::endl;

        std::cout << "Connecting to host: \"" << host_name << ":" << port << "\"..." << std::endl;

        // Create new connection with server.
        auto &&proxy_to_server_socket = connect_to_target_server(host_name, port);

        std::cout << "Connected.\n\n"
                  << "Writing HTTP request to the target server..." << std::endl;

        if (send(proxy_to_server_socket, &new_request.at(0), new_request.size(), 0) < 0)
        {
            client_error(client_socket, method, 503, "Internal error", sock_wrap_.get_last_error_string());
            throw std::system_error(sock_wrap_.get_last_error_code(), std::system_category(), "send");
        }

        std::cout << "Request was written.\n\n"
                  << "Reading response from the target server..." << std::endl;

        std::string response;

        do
        {
            // HTTP 1.x is a text protocol.
            line = read_line(proxy_to_server_socket);
            response += line;
        } while (line.size());

        std::cout << "Response was read.\n"
                  << "=======================\n"
                  << "Target server response:\n"
                  << "-----------------------\n"
                  << response << "\n=======================\n"
                  << std::endl;

        std::cout << "Forwarding response to the client..." << std::endl;

        if (send(client_socket, &response.at(0), response.size(), 0) < 0)
        {
            throw std::system_error(errno, std::system_category(), "send");
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception in the client!" << std::endl;
    }
}


void ProxyServer::start()
{
    sockaddr_in addr = {.sin_family = PF_INET, .sin_port = htons(port_)};

    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) != 0)
    {
        throw std::system_error(errno, std::system_category(), "bind()");
    }

    // Start listening on port 'port' for incoming requests and serve them.
    if (listen(sock_, back_log) < 0)
    {
        throw std::logic_error("Can't listen on the socket!");
    }

    std::cout << "Listening on port " << port_ << "..." << std::endl;

    // For now the only way to shutdown the server is interreption signal.
    // For proper asynchronous server shutdown flag could be handled gracefully.
    while (true)
    {
        struct sockaddr_storage client_addr = {0};

        socklen_t client_len = sizeof(client_addr);

        socket_wrapper::Socket client_sock(accept(sock_, reinterpret_cast<sockaddr *>(&client_addr), &client_len));

        std::cout << "Accepted connection" << std::endl;

        if (!client_sock)
        {
            throw std::system_error(errno, std::system_category(), "Accept connection");
        }

        // Start a new thread for every new request.
        // Threads are not necessary here, because "proxify" call is synchronous anyway.
        // For proper multithreading examples see further examples.
        std::thread client_thread(&ProxyServer::proxify, this, std::move(client_sock));
        std::cout << "Creating client thread..." << std::endl;
        client_thread.join();
    }
}

void ProxyServer::stop() noexcept
{
    // TODO: implement
    // For now it's impossible to shutdown server gracefully.
    // Asynchronous/multithreading server will be implemented in further examples.
}
