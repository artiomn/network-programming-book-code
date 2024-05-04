extern "C"
{
#include <fcntl.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


namespace
{

const size_t clients_count = 10;
const size_t buffer_size = 255;

}  // namespace


int main(int argc, const char *const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        auto server_sock = socket_wrapper::create_tcp_server(argv[1]);
        auto client_sock = socket_wrapper::accept_client(server_sock);

        std::vector<char> data_buff(buffer_size);

        char oob_data;
        bool oob_printed = false;

        while (true)
        {
            int at_mark = sockatmark(client_sock);

            switch (at_mark)
            {
                case -1:
                    throw std::system_error(errno, std::system_category(), "sockatmark");
                    break;
                case 1:
                    if (oob_printed) continue;
                    std::cout << "OOB data received..." << std::endl;
                    if (-1 == recv(client_sock, &oob_data, 1, MSG_OOB))
                    {
                        throw std::system_error(errno, std::system_category(), "recv oob");
                    }
                    std::cout << "OOB data = " << oob_data << std::endl;
                    oob_printed = true;
                    break;
                case 0:
                {
                    ssize_t n = recv(client_sock, data_buff.data(), data_buff.size(), 0);

                    if (n < 0)
                    {
                        throw std::system_error(errno, std::system_category(), "recv data");
                    }
                    else if (!n)
                    {
                        std::cout << "No data, exiting..." << std::endl;
                        exit(EXIT_SUCCESS);
                    }

                    std::cout << "Ordinary data received...\n"
                              << n << " bytes was read: " << std::string(data_buff.begin(), data_buff.begin() + n)
                              << std::endl;
                    oob_printed = false;
                    break;
                }
                default:
                {
                }
            }
        }
    }
    catch (const std::system_error &e)
    {
        std::cerr << e.what() << ": " << sock_wrap.get_last_error_string() << "!" << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
