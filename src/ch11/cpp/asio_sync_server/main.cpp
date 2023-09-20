#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

using namespace boost::asio;
using socket_ptr = std::shared_ptr<ip::tcp::socket>;


void client_session(socket_ptr sock)
{
    while (true)
    {
        std::string data;
        data.resize(10);
        size_t len = sock->receive(buffer(data));
        if (len > 1)
        {
            boost::asio::write(*sock, buffer("ok\n", 3));
            data.resize(len - 1);
            std::cout << data << std::endl;
        }
        else
        {
            sock->write_some(buffer("bye\n", 4));
            break;
        }
    }
}


int main()
{
    io_context context;

    ip::tcp::endpoint server_endpoint(ip::tcp::v4(), 1234);
    ip::tcp::acceptor acc(context, server_endpoint);

    while (true)
    {
        try
        {
            auto sock{std::make_shared<ip::tcp::socket>(context)};
            acc.accept(*sock);
            std::thread(client_session, sock).detach();
        }
        catch (const boost::system::system_error &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}
