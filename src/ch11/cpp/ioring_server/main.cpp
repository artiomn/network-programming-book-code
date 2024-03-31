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
#include <list>
#include <string>
#include <vector>


class Request
{
public:
    enum class EventType
    {
        none,
        accept,
        recv,
        send
    };
    using ContainerType = std::vector<char>;

public:
    Request() : data_(256), iov_{data_.data(), data_.size()}, msg_{nullptr, 0, &iov_, 1, nullptr, 0, 0} {}

    ~Request()
    {
        if (client_sock_ != -1) close(client_sock_);
    }

public:
    ContainerType::value_type *data() { return data_.data(); }
    const ContainerType::value_type *data() const { return data_.data(); }
    msghdr *msg() { return &msg_; }
    void set_length(size_t len) { iov_.iov_len = len; }
    void reset_length() { iov_.iov_len = data_.size(); }
    const void *addr() const { return this; }

public:
    int client_sock_ = -1;
    EventType event_type_ = EventType::accept;

    sockaddr_in client_addr;
    __socklen_t client_addr_len = sizeof(client_addr);

private:
    ContainerType data_;
    iovec iov_;
    msghdr msg_;
};


int accept_client(socket_wrapper::Socket &server_socket, io_uring &ring, std::list<Request> &requests)
{
    // Get an SQE and fill in a ACCEPT operation.
    io_uring_sqe *sqe = io_uring_get_sqe(&ring);

    requests.emplace_front();

    auto req = &requests.front();

    io_uring_sqe_set_data(sqe, req);
    io_uring_prep_accept(sqe, server_socket, reinterpret_cast<sockaddr *>(&req->client_addr), &req->client_addr_len, 0);

    // sqe->user_data = &requests[requests.size() - 1];

    // Tell the kernel we have an SQE ready for consumption.
    return io_uring_submit(&ring);
}


int async_recv(io_uring &ring, Request *req)
{
    io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_recvmsg(sqe, req->client_sock_, req->msg(), 0);
    req->event_type_ = Request::EventType::recv;
    io_uring_sqe_set_data(sqe, req);

    return io_uring_submit(&ring);
}


int async_send(io_uring &ring, Request *req)
{
    io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_sendmsg(sqe, req->client_sock_, req->msg(), 0);
    req->event_type_ = Request::EventType::send;
    io_uring_sqe_set_data(sqe, req);

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

    socket_wrapper::SocketWrapper sock_wrap;
    socket_wrapper::Socket server_sock = std::move(socket_wrapper::create_tcp_server(argv[1]));

    try
    {
        std::list<Request> requests;
        io_uring_queue_init(16, &ring, 0);

        if (1 != accept_client(server_sock, ring, requests))
        {
            throw std::system_error(errno, std::system_category(), "accept_client");
        }

        while (true)
        {
            io_uring_cqe *cqe;
            __kernel_timespec ts = {5, 0};
            int res = io_uring_wait_cqes(&ring, &cqe, 1, &ts, nullptr);

            if (res < 0)
            {
                throw std::system_error(-res, std::system_category(), "io_uring_wait_cqe");
            }

            Request *request = reinterpret_cast<Request *>(cqe->user_data);

            if (Request::EventType::accept == request->event_type_)
            {
                std::cout << "Accept client." << std::endl;
                request->client_sock_ = cqe->res;

                if (request->client_sock_ < 0)
                {
                    throw std::system_error(errno, std::system_category(), "accept");
                }

                // Request to accept new client.
                if (1 != accept_client(server_sock, ring, requests))
                {
                    throw std::system_error(errno, std::system_category(), "accept_client");
                }
                if (1 != async_recv(ring, request))
                {
                    throw std::system_error(errno, std::system_category(), "async_recv");
                }
            }
            else if (Request::EventType::recv == request->event_type_)
            {
                // Receive completed.
                // If it is the end of file we are done
                std::cout << "Data received." << std::endl;
                if (!cqe->res)
                {
                    std::cout << "Empty request." << std::endl;
                    requests.remove_if([&request](const Request &req) { return req.addr() == request; });
                    accept_client(server_sock, ring, requests);
                }
                else if (cqe->res < 0)
                {
                    throw std::system_error(errno, std::system_category(), "cqe->res < 0");
                }
                else
                {
                    std::cout << "Res = " << cqe->res
                              << ", Data = " << std::string(request->data(), request->data() + cqe->res) << std::endl;
                    request->set_length(cqe->res);

                    if (1 != async_send(ring, request))
                    {
                        throw std::system_error(errno, std::system_category(), "async_send");
                    }
                }
            }
            else if (Request::EventType::send == request->event_type_)
            {
                std::cout << "Data was sent." << std::endl;
                request->reset_length();

                if (1 != async_recv(ring, request))
                {
                    throw std::system_error(errno, std::system_category(), "async_recv");
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
