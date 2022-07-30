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
const size_t buffer_size = 255;

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

        if (!socket)
        {
            throw std::logic_error("socket");
        }

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

        std::vector<char> data_buff;
        data_buff.resize(buffer_size);
        auto data_buff_iterator = data_buff.begin();

        sig_handler = [&client_sock, &data_buff_iterator](int sig_num)
        {
            std::vector<char> oob_buff;
            oob_buff.resize(buffer_size);

            std::cout << "SIGURG received" << std::endl;

            while (!socatmark(client_sock))
            {
                buf_free_size = data_buff.size() -
                                (data_buff_iterator - data_buff.cbegin()) - 1;

                ssize_t n = recv(client_sock, &(*data_buff_iterator), buff_free_size, 0);
            }

            auto n = recv(client_sock, &oob_buff[0], oob_buff.size() - 1, MSG_OOB);
            oob_buff[n] = '\0';
            std::cout
                << n << " OOB bytes was read: "
                << std::string(oob_buff.begin(), oob_buff.begin() + n)
                << std::endl;
        };

        std::signal(SIGURG, signal_handler_wrapper);

        fcntl(client_sock, F_SETOWN, getpid());

        for (ssize_t n = recv(client_sock, &(*data_buff_iterator),
                              data_buff.size() - (data_buff_iterator - data_buff.cbegin()) - 1, 0);
             n;
             n = recv(client_sock, &data_buff[0], data_buff.size() - 1, 0))
        {
            if (n < 0)
            {
                throw std::logic_error("recvmsg");
            }

			data_buff_iterator += n;
            *data_buff_iterator = '\0';
            std::cout
                << n
                << " bytes was read: " << std::string(data_buff.begin(), data_buff.begin() + n)
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

