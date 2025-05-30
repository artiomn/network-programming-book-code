//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, coroutine
//
//------------------------------------------------------------------------------

#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>


namespace beast = boost::beast;    // from <boost/beast.hpp>
namespace http = beast::http;      // from <boost/beast/http.hpp>
namespace net = boost::asio;       // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>
using namespace std::chrono_literals;


const auto timeout = 3s;

// Performs an HTTP GET and prints the response
void do_session(
    std::string const& host, std::string const& port, std::string const& target, int version, net::io_context& ioc,
    net::yield_context yield)
{
    beast::error_code ec;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);

    try
    {
        // Look up the domain name
        auto const results = resolver.async_resolve(host, port, yield[ec]);
        if (ec) throw std::system_error(ec, "resolve");

        beast::tcp_stream stream(ioc);
        // Set the timeout.
        stream.expires_after(timeout);

        // Make the connection on the IP address we get from a lookup
        stream.async_connect(results, yield[ec]);
        if (ec) throw std::system_error(ec, "connect");

        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Set the timeout.
        stream.expires_after(timeout);

        // Send the HTTP request to the remote host
        http::async_write(stream, req, yield[ec]);
        if (ec) throw std::system_error(ec, "write");

        // This buffer is used for reading and must be persisted
        beast::flat_buffer b;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::async_read(stream, b, res, yield[ec]);
        if (ec) throw std::system_error(ec, "read");

        // Write the message to standard out
        std::cout << res << std::endl;

        // Gracefully close the socket
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        if (ec && ec != beast::errc::not_connected) throw std::system_error(ec, "shutdown");
    }
    catch (std::system_error& e)
    {
        std::cerr << e.what() << ": " << e.code().message() << "\n";
    }

    // If we get here then the connection is closed gracefully
}


int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc < 2 || argc > 4)
    {
        std::cerr << "Usage: http-client-coro <host>[:port] [target] [<HTTP version: 1.0 or 1.1(default)>]\n"
                  << "Example:\n"
                  << "    http-client-coro www.example.com:80 /\n"
                  << "    http-client-coro www.example.com / 1.0\n";
        return EXIT_FAILURE;
    }

    auto const hp = std::string(argv[1]);
    auto const port_pos = hp.find(':');

    auto const host = (std::string::npos == port_pos) ? hp : hp.substr(0, port_pos);
    auto const port = (std::string::npos == port_pos) ? "80" : hp.substr(port_pos + 1);
    auto const target = (argc >= 3) ? argv[2] : "/";

    int version = argc == 4 && !std::strcmp("1.0", argv[3]) ? 10 : 11;

    // The io_context is required for all I/O
    net::io_context ioc;

    std::cout << host << ":" << port << target << std::endl;
    // Launch the asynchronous operation
    boost::asio::spawn(
        ioc, std::bind(&do_session, host, port, std::string(target), version, std::ref(ioc), std::placeholders::_1));

    // Run the I/O service. The call will return when
    // the get operation is complete.
    ioc.run();

    return EXIT_SUCCESS;
}
