#include <cerrno>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#if !defined(_WIN32)
extern "C"
{
#    include <signal.h>
}
#else
#    include <cwctype>
#endif

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <boost/asio.hpp>


const auto clients_count = 3;


int main(int argc, const char *const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

#if !defined(_WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif

    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        auto servinfo = socket_wrapper::get_serv_info(argv[1]);

        socket_wrapper::Socket server_sock = {servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol};

        if (!server_sock)
        {
            throw std::system_error(errno, std::system_category(), "socket");
        }

        socket_wrapper::set_reuse_addr(server_sock);

        if (bind(server_sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            throw std::system_error(errno, std::system_category(), "bind");
        }

        if (listen(server_sock, clients_count) < 0)
        {
            throw std::system_error(errno, std::system_category(), "listen");
        }

        std::cout << "Listening on port " << argv[1] << "...\n" << std::endl;

        boost::asio::thread_pool pool(clients_count);
        std::mutex m;

        for (int i = 0; i < clients_count; ++i)
        {
            boost::asio::post(
                pool,
                [&server_sock, &m, i]()
                {
                    {
                        std::lock_guard<std::mutex> lg(m);
                        std::cout << "Thread " << i << " started..." << std::endl;
                    }
                    auto client_sock = socket_wrapper::accept_client(server_sock);
                    std::cout << "Thread " << i << " accepted client..." << std::endl;
                    char data;
                    auto result = recv(client_sock, &data, sizeof(data), 0);
                    std::cout << "Thread " << i << " received data and exited." << std::endl;
                });
        }

        pool.join();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << ": " << sock_wrap.get_last_error_string() << "!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
