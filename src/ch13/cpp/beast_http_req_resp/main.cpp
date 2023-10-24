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


int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << "<host>[:port]\n"
                  << "Example:\n"
                  << "    " << argv[0] << " www.example.com:80 /" << std::endl;
        return EXIT_FAILURE;
    }

    auto const hp = std::string(argv[1]);
    auto const port_pos = hp.find(':');

    auto const host = (std::string::npos == port_pos) ? hp : hp.substr(0, port_pos);
    auto const port = (std::string::npos == port_pos) ? "80" : hp.substr(port_pos + 1);

    // The io_context is required for all I/O
    net::io_context ioc;

    std::cout << host << ":" << port << std::endl;

    beast::error_code ec;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);

    try
    {
        beast::tcp_stream stream(net::make_strand(ioc));
        // Set the timeout.
        stream.expires_after(timeout);

        // Look up the domain name.
        auto results = resolver.resolve(host, port);
        if (ec) throw std::system_error(ec, "resolve");

        stream.expires_after(timeout);
        stream.connect(results, ec);
        if (ec) throw std::system_error(ec, "connect");

        stream.expires_never();

        // Set up an HTTP GET request message.
        http::request<http::empty_body> req;
        req.version(11);  // HTTP/1.1
        req.method(http::verb::get);
        req.target("/index.html");
        req.set(http::field::host, host);
        req.set(http::field::accept, "text/html");
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        std::cout << "->\n" << req << std::endl;

        // Send the HTTP request to the remote host
        http::write(stream, req, ec);
        if (ec) throw std::system_error(ec, "write");

        beast::flat_buffer b;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(stream, b, res, ec);
        if (ec) throw std::system_error(ec, "read");
        // Write the message to standard out
        std::cout << "<-\n" << res << std::endl;
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

    // Run the I/O service. The call will return when
    // the get operation is complete.
    ioc.run();

    return EXIT_SUCCESS;
}
