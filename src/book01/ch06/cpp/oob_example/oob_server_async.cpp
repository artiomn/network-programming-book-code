extern "C"
{
#include <fcntl.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <array>
#include <atomic>
#include <cassert>
#include <csignal>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>


constexpr size_t buffer_size = 255;

std::atomic<bool> oob_flag = false;

void signal_handler_wrapper(int signal)
{
    std::cout << "SIGURG [" << signal << "] received" << std::endl;
    oob_flag = true;
}


int main(int argc, const char *const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    const socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        if (SIG_ERR == std::signal(SIGURG, signal_handler_wrapper))
        {
            throw std::system_error(errno, std::system_category(), "signal");
        }

        std::array<char, buffer_size> data_buff;

        assert(argv[1]);
        auto server_sock = socket_wrapper::create_tcp_server(argv[1]);
        auto client_sock = socket_wrapper::accept_client(server_sock);

        if (-1 == fcntl(client_sock, F_SETOWN, getpid()))
        {
            throw std::system_error(errno, std::system_category(), "fcntl");
        }

        while (true)
        {
            switch (sockatmark(client_sock))
            {
                case -1:
                    throw std::system_error(errno, std::system_category(), "sockatmark");
                    break;
                case 1:
                    if (!oob_flag) continue;
                    std::cout << "OOB data received..." << std::endl;

                    if (char oob_data = recv(client_sock, &oob_data, 1, MSG_OOB); -1 == oob_data)
                    {
                        throw std::system_error(errno, std::system_category(), "recv oob");
                    }
                    else
                    {
                        std::cout << "OOB data = " << oob_data << std::endl;
                    }

                    oob_flag = false;
                    break;
                case 0:
                    if (ssize_t n = recv(client_sock, data_buff.data(), data_buff.size(), 0); n < 0)
                    {
                        if (EINTR == errno || EAGAIN == errno)
                        {
                            std::cout << "recv was broken by signal!" << std::endl;
                            continue;
                        }
                        throw std::system_error(errno, std::system_category(), "recv data");
                    }
                    else if (!n)
                    {
                        std::cout << "No data, exiting..." << std::endl;
                        exit(EXIT_SUCCESS);
                    }
                    else
                    {
                        std::cout << "Ordinary data received...\n"
                                  << n << " bytes was read: " << std::string(data_buff.begin(), data_buff.begin() + n)
                                  << std::endl;
                    }
                    break;
                default:
                    throw std::runtime_error("unexpected sockatmark");
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
