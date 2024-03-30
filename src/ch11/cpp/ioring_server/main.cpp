/*
This is a simple server that users io_uring in blocking mode.
It demonstrates the bare minimum needed to use io_uring.
It uses liburing for simplicity.
*/

extern "C"
{
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
}

#include <liburing.h>
#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <cassert>
#include <iostream>
#include <string>


int async_read(int sock, io_uring &ring, msghdr &msg)
{
    // Get an SQE and fill in a READ operation.
    io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_recvmsg(sqe, sock, &msg, 0);
    sqe->user_data = 0;

    // Tell the kernel we have an SQE ready for consumption.
    return io_uring_submit(&ring);
}


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << argv[0] << " <port>" << std::endl;
        exit(EXIT_FAILURE);
    }

    io_uring ring;

    std::string data;

    data.resize(256);

    iovec iov = {data.data(), data.size()};
    msghdr msg = {nullptr, 0, &iov, 1, nullptr, 0, 0};

    socket_wrapper::SocketWrapper sock_wrap;

    auto servinfo = socket_wrapper::get_serv_info(argv[1]);
    socket_wrapper::Socket sock = {servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol};

    try
    {
        if (sock < 0)
        {
            throw std::system_error(errno, std::system_category(), "socket");
        }

        if (bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            throw std::system_error(errno, std::system_category(), "bind");
        }

        if (listen(sock, 1) < 0)
        {
            throw std::system_error(errno, std::system_category(), "listen");
        }

        sockaddr_in cli_addr;
        __socklen_t clilen = sizeof(cli_addr);
        socket_wrapper::Socket newsock{accept(sock, reinterpret_cast<sockaddr *>(&cli_addr), &clilen)};

        if (newsock < 0)
        {
            throw std::system_error(errno, std::system_category(), "accept");
        }

        io_uring_queue_init(16, &ring, 0);

        if (1 != async_read(newsock, ring, msg))
        {
            throw std::system_error(errno, std::system_category(), "async_read");
        }

        while (true)
        {
            io_uring_cqe *cqe;
            __kernel_timespec ts = {2, 0};
            int res = io_uring_wait_cqes(&ring, &cqe, 1, &ts, nullptr);

            if (res < 0)
            {
                throw std::system_error(-res, std::system_category(), "io_uring_wait_cqe");
            }

            if (0 == cqe->user_data)
            {
                // Read completed
                // If it is the end of file we are done
                if (!cqe->res)
                {
                    break;
                }

                if (cqe->res < 0)
                {
                    if (sock > 0) close(sock);
                    throw std::system_error(errno, std::system_category(), "cqe res < 0");
                }

                std::cout << "Res = " << cqe->res << ", Data = " << data << std::endl;
                if (1 != async_read(newsock, ring, msg))
                {
                    throw std::system_error(errno, std::system_category(), "async_read");
                }
            }
            else if (LIBURING_UDATA_TIMEOUT == cqe->user_data)
            {
                std::cout << ".";
            }
            else
            {
                // Problem
                throw std::system_error(errno, std::system_category(), "Incorrect user data");
            }

            io_uring_cqe_seen(&ring, cqe);
        }

        io_uring_queue_exit(&ring);

        if (close(newsock) < 0)
        {
            throw std::system_error(errno, std::system_category(), "close(newsock)");
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
