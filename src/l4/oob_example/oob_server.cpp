#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

extern "C"
{
#include <fcntl.h>
}

#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


namespace
{

const size_t clients_count = 10;

std::function<void(int)> sig_handler;

void signal_handler_wrapper(int signal)
{
    sig_handler(signal);
}

} // namespace


int main(int argc, const char * const argv[])
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

        if (bind(server_sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            throw std::logic_error("bind");
        }

        if (listen(server_sock, clients_count) < 0)
        {
            throw std::logic_error("listen");
        }

		socket_wrapper::Socket client_sock(accept(server_sock, nullptr, nullptr));

        if (!client_sock)
        {
            throw std::logic_error("accept");
        }

        sig_handler = [&client_sock](int sig_num)
        {
            char buff[100];

            std::cout << "SIGURG received" << std::endl;
            auto n = recv(client_sock, buff, sizeof(buff) - 1, MSG_OOB);
            buff[n] = 0;
            std::cout
                << n << " OOB byte: " << buff
                << std::endl;
        };

        std::signal(SIGURG, signal_handler_wrapper);

        fcntl(client_sock, F_SETOWN, getpid());

        std::vector<char> buff;
        buff.resize(100);

        for (ssize_t n = 1; n; n = recv(client_sock, &buff[0], buff.size() - 1, 0))
        {
            buff[n] = 0;
            std::cout
                << "read " << n
                << " bytes: " << std::string(buff.begin(), buff.begin() + n)
                << std::endl;
        }

    }
    catch (const std::logic_error &e)
    {
        std::cerr
            << e.what()
            << ": " << sock_wrap.get_last_error_string() << "!"
            << std::endl;
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

