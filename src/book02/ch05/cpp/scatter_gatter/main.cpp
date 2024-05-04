extern "C"
{
#include <signal.h>
#include <sys/types.h>
#include <sys/uio.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <cerrno>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>


const auto clients_count = 3;


int main(int argc, const char *const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    signal(SIGPIPE, SIG_IGN);

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

        auto client_sock = socket_wrapper::accept_client(server_sock);
        std::cout << "Accepted client..." << std::endl;

        std::vector<char> data_buffer(256);

        struct iovec iov[1];

        iov[0].iov_base = &data_buffer[0];
        iov[0].iov_len = data_buffer.size();
        // iov[1].iov_base = buf1;
        // iov[1].iov_len = sizeof(buf1);
        // iov[2].iov_base = buf2;
        // iov[2].iov_len = sizeof(buf2);
        // ...
        const size_t iovcnt = sizeof(iov) / sizeof(iovec);

        ssize_t bytes_count = -1;

        while (bytes_count)
        {
            bytes_count = readv(client_sock, iov, iovcnt);

            if (bytes_count < 0)
            {
                if (EINTR == errno) continue;
                throw std::system_error(errno, std::system_category(), "readv");
            }

            bytes_count = writev(client_sock, iov, iovcnt);

            if (bytes_count < 0)
            {
                if (EINTR == errno) continue;
                throw std::system_error(errno, std::system_category(), "writev");
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << ": " << sock_wrap.get_last_error_string() << "!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
