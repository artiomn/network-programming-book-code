extern "C"
{
#include <fcntl.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <chrono>
#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <vector>


namespace
{

const size_t clients_count = 10;
const size_t buffer_size = 255;

std::function<void(int)> sig_handler;

void signal_handler_wrapper(int signal)
{
    sig_handler(signal);
}

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
        auto servinfo = socket_wrapper::get_serv_info(argv[1]);

        socket_wrapper::Socket server_sock = {servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol};

        if (!socket)
        {
            throw std::system_error(errno, std::system_category(), "socket");
        }

        if (bind(server_sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            throw std::system_error(errno, std::system_category(), "bind");
        }

        if (listen(server_sock, clients_count) < 0)
        {
            throw std::system_error(errno, std::system_category(), "listen");
        }

        socket_wrapper::Socket client_sock(accept(server_sock, nullptr, nullptr));

        if (!client_sock)
        {
            throw std::system_error(errno, std::system_category(), "accept");
        }

        std::vector<char> data_buff;
        data_buff.reserve(buffer_size);

        // cppcheck-suppress danglingLifetime
        sig_handler = [&client_sock, &data_buff](int sig_num)
        {
            std::cout << "SIGURG received" << std::endl;

            char data;

            for (int sa_result = sockatmark(client_sock); sa_result; sa_result = sockatmark(client_sock))
            {
                if (-1 == sa_result)
                {
                    perror("sockatmark");
                    return;
                }

                auto res = recv(client_sock, &data, sizeof(data), 0);
                if (-1 == res)
                {
                    perror("recv ordinary in handler");
                    return;
                }

                data_buff.push_back(data);
                std::cout << data << " byte received in handler" << std::endl;
            }

            auto res = recv(client_sock, &data, sizeof(data), MSG_OOB);

            if (-1 == res)
            {
                perror("recv ordinary in handler");
                return;
            }

            std::cout << data << " OOB byte was read" << std::endl;
        };

        std::signal(SIGURG, signal_handler_wrapper);

        fcntl(client_sock, F_SETOWN, getpid());

        char data;

        for (ssize_t n = recv(client_sock, &data, sizeof(data), 0); n; n = recv(client_sock, &data, sizeof(data), 0))
        {
            if (n < 0)
            {
                throw std::system_error(errno, std::system_category(), "recv");
            }

            data_buff.push_back(data);
            std::cout << data << " byte was read in the working cycle" << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "Result: " << std::string(data_buff.begin(), data_buff.end()) << std::endl;
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
