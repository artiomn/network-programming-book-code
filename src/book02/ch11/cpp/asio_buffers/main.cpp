#include <iostream>

#include <boost/asio.hpp>


int main()
{
    boost::asio::io_context io_context;

    boost::asio::ip::tcp::socket socket(io_context);

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), 1234);

    socket.connect(endpoint);

    std::string message = "Hello!\n";
    boost::asio::write(socket, boost::asio::buffer(message));

    std::array<char, 128> buffer;
    boost::system::error_code error;

    size_t length = socket.read_some(boost::asio::buffer(buffer), error);
    if (boost::asio::error::eof == error)
    {
        std::cout << "Connection closed by peer." << std::endl;
    }
    else if (error)
    {
        std::cout << "Error: " << error.message() << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "Received: " << std::string(buffer.data(), length) << std::endl;
    }

    return EXIT_SUCCESS;
}
