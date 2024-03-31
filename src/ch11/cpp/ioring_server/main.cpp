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


enum event_type
{
    et_accept,
    et_recv,
    et_send
};


int accept_client(
    socket_wrapper::Socket &server_socket, io_uring &ring, sockaddr *client_addr, socklen_t &client_addr_len)
{
    // Get an SQE and fill in a READ operation.
    io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_accept(sqe, server_socket, client_addr, &client_addr_len, 0);

    sqe->user_data = et_accept;
    // io_uring_sqe_set_data(sqe, reinterpret_cast<void*>(et_accept));

    // Tell the kernel we have an SQE ready for consumption.
    return io_uring_submit(&ring);
}


int async_read(int sock, io_uring &ring, msghdr &msg)
{
    io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_recvmsg(sqe, sock, &msg, 0);
    sqe->user_data = et_recv;

    return io_uring_submit(&ring);
}


int async_send(int sock, io_uring &ring, msghdr &msg)
{
    io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_sendmsg(sqe, sock, &msg, 0);
    sqe->user_data = et_send;

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
    socket_wrapper::Socket server_sock = std::move(socket_wrapper::create_tcp_server(argv[1]));

    try
    {
        sockaddr_in cli_addr;
        __socklen_t cli_addr_len = sizeof(cli_addr);
        int client_sock;

        io_uring_queue_init(16, &ring, 0);

        accept_client(server_sock, ring, reinterpret_cast<sockaddr *>(&cli_addr), cli_addr_len);

        while (true)
        {
            io_uring_cqe *cqe;
            __kernel_timespec ts = {5, 0};
            int res = io_uring_wait_cqes(&ring, &cqe, 1, &ts, nullptr);

            if (res < 0)
            {
                throw std::system_error(-res, std::system_category(), "io_uring_wait_cqe");
            }

            event_type type = (event_type)(cqe->user_data);

            if (et_accept == type)
            {
                std::cout << "Accept client." << std::endl;
                client_sock = cqe->res;

                if (client_sock < 0)
                {
                    throw std::system_error(errno, std::system_category(), "accept");
                }

                if (1 != async_read(client_sock, ring, msg))
                {
                    throw std::system_error(errno, std::system_category(), "async_read");
                }
            }
            else if (et_recv == type)
            {
                // Receive completed.
                // If it is the end of file we are done
                if (!cqe->res)
                {
                    std::cout << "Empty request." << std::endl;
                    close(client_sock);
                    client_sock = -1;

                    accept_client(server_sock, ring, reinterpret_cast<sockaddr *>(&cli_addr), cli_addr_len);
                }
                else if (cqe->res < 0)
                {
                    throw std::system_error(errno, std::system_category(), "cqe res < 0");
                }
                else
                {
                    std::cout << "Res = " << cqe->res << ", Data = " << data << std::endl;

                    if (1 != async_send(client_sock, ring, msg))
                    {
                        throw std::system_error(errno, std::system_category(), "async_read");
                    }

                    if (1 != async_read(client_sock, ring, msg))
                    {
                        throw std::system_error(errno, std::system_category(), "async_read");
                    }
                }
            }
            else if (et_send == type)
            {
                std::cout << "Data was sent" << std::endl;
                if (1 != async_read(client_sock, ring, msg))
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
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
