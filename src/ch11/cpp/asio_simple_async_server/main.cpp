#include <iostream>
#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

using namespace boost::asio;
using socket_ptr = std::shared_ptr<ip::tcp::socket>;


void handle_accept(io_context &context, ip::tcp::acceptor &acc, socket_ptr sock)
{
    std::cout << "Client accepted" << std::endl;
    sock->async_send(
        buffer(std::string("Acc\n")), [](const boost::system::error_code &error, std::size_t bytes_transferred)
        { std::cout << bytes_transferred << " bytes were sent" << std::endl; });
}


void start_accept(io_context &context, ip::tcp::acceptor &acc)
{
    socket_ptr sock{std::make_shared<ip::tcp::socket>(context)};
    // Pointer MUST be passed by the value!
    acc.async_accept(
        *sock,
        [&, sock](const boost::system::error_code &err)
        {
            start_accept(context, acc);
            if (err) return;
            handle_accept(context, acc, sock);
        });
}


int main()
{
    io_context context;
    ip::tcp::endpoint server_endpoint(ip::tcp::v4(), 12345);
    ip::tcp::acceptor acc(context, server_endpoint);

    start_accept(context, acc);
    context.run();
}
