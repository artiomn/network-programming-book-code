#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>


using boost::asio::ip::tcp;
const int echo_port = 1300;

std::string make_daytime_string()
{
    using namespace std;  // time_t, time и ctime;
    time_t now = time(0);
    return ctime(&now);
}


class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    typedef std::shared_ptr<TcpConnection> pointer;

    static pointer create(boost::asio::io_context& io_context) { return pointer(new TcpConnection(io_context)); }

    tcp::socket& socket() { return socket_; }

    void start()
    {
        // The data to be sent is stored in the class member message_ as we need to keep the data valid until the
        // asynchronous operation is complete.
        message_ = make_daytime_string();
        auto s = shared_from_this();

        boost::asio::async_write(
            socket_, boost::asio::buffer(message_),
            [s](const boost::system::error_code& error, size_t bytes_transferred)
            { s->handle_write(error, bytes_transferred); });
    }

private:
    explicit TcpConnection(boost::asio::io_context& io_context) : socket_(io_context) {}

    void handle_write(const boost::system::error_code& /*error*/, size_t bytes_transferred)
    {
        std::cout << "Bytes transferred: " << bytes_transferred << std::endl;
    }

private:
    tcp::socket socket_;
    std::string message_;
};


class TcpServer
{
public:
    explicit TcpServer(boost::asio::io_context& io_context)
        : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), echo_port))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        TcpConnection::pointer new_connection = TcpConnection::create(io_context_);

        acceptor_.async_accept(
            new_connection->socket(), [this, new_connection](const boost::system::error_code& error)
            { this->handle_accept(new_connection, error); });
    }

    void handle_accept(TcpConnection::pointer new_connection, const boost::system::error_code& error)
    {
        if (!error)
        {
            new_connection->start();
        }

        start_accept();
    }

private:
    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};


int main()
{
    try
    {
        boost::asio::io_context io_context;
        TcpServer server(io_context);

        io_context.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
