extern "C"
{
#include <fcntl.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <atomic>
#include <csignal>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>


const size_t buffer_size = 255;

std::function<void(int)> sig_handler;

void signal_handler_wrapper(int signal)
{
    sig_handler(signal);
}


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
        volatile std::atomic<bool> oob_flag = false;

        // cppcheck-suppress danglingLifetime
        sig_handler = [&oob_flag](int sig_num)
        {
            std::cout << "SIGURG [" << sig_num << "] received" << std::endl;
            oob_flag = true;
        };

        if (SIG_ERR == std::signal(SIGURG, signal_handler_wrapper))
        {
            throw std::system_error(errno, std::system_category(), "signal");
        }

        std::vector<char> data_buff(buffer_size);

        char oob_data;

        auto server_sock = socket_wrapper::create_tcp_server(argv[1]);
        auto client_sock = socket_wrapper::accept_client(server_sock);

        if (-1 == fcntl(client_sock, F_SETOWN, getpid()))
        {
            throw std::system_error(errno, std::system_category(), "fcntl");
        }

        while (true)
        {
            int at_mark = sockatmark(client_sock);

            switch (at_mark)
            {
                case -1:
                    throw std::system_error(errno, std::system_category(), "sockatmark");
                    break;
                case 1:
                {
                    if (!oob_flag) continue;
                    std::cout << "OOB data received..." << std::endl;

                    if (-1 == recv(client_sock, &oob_data, 1, MSG_OOB))
                    {
                        throw std::system_error(errno, std::system_category(), "recv oob");
                    }

                    std::cout << "OOB data = " << oob_data << std::endl;
                    oob_flag = false;
                }
                break;
                case 0:
                {
                    ssize_t n = recv(client_sock, data_buff.data(), data_buff.size(), 0);

                    if (n < 0)
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

                    std::cout << "Ordinary data received...\n"
                              << n << " bytes was read: " << std::string(data_buff.begin(), data_buff.begin() + n)
                              << std::endl;
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
