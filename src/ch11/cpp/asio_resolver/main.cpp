#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#include <boost/asio.hpp>


using boost::asio::ip::tcp;


int main()
{
    try
    {
        std::string url = "www.artiomsoft.ru";

        boost::asio::io_context context;
        tcp::resolver resolver(context);
        tcp::resolver::query query(url, "443");

        resolver.async_resolve(
            query,
            [&context](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator ep_iter)
            {
                if (ec)
                {
                    return;
                }

                tcp::resolver::iterator end;
                tcp::socket socket(context);
                boost::system::error_code error = boost::asio::error::host_not_found;
                while (error && ep_iter != end)
                {
                    socket.close();
                    std::cout << "Connecting to " << ep_iter->endpoint().address() << ":" << ep_iter->endpoint().port()
                              << std::endl;
                    socket.connect(*ep_iter++, error);
                }
            });

        context.run();
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what();
        std::getchar();
    }
    return 0;
}
