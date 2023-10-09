extern "C"
{
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <array>
#include <iostream>
#include <string>


const size_t max_events = 10;


int main(int argc, const char* const argv[])
{
    if (argc < 2)
    {
        std::cerr << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;

    auto servinfo = std::move(socket_wrapper::get_serv_info(argv[1]));
    socket_wrapper::Socket server_sock = {servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol};

    if (!server_sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::set_reuse_addr(server_sock);

    if (bind(server_sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    if (-1 == listen(server_sock, SOMAXCONN))
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    int epoll_fd = epoll_create1(0);
    if (-1 == epoll_fd)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    epoll_event event;

    event.data.fd = server_sock;
    event.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock, &event) == -1)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    std::array<epoll_event, max_events> events;

    while (true)
    {
        int events_count = epoll_wait(epoll_fd, events.data(), events.size(), -1);

        if (-1 == events_count)
        {
            std::cerr << sock_wrap.get_last_error_string() << std::endl;
            return EXIT_FAILURE;
        }

        for (int i = 0; i < events_count; ++i)
        {
            if (events[i].data.fd == server_sock)
            {
                sockaddr_in client_addr = {0};
                socklen_t cl_a_len = sizeof(client_addr);
                auto client_sock = accept(server_sock, reinterpret_cast<sockaddr*>(&client_addr), &cl_a_len);
                if (-1 == client_sock)
                {
                    std::cerr << sock_wrap.get_last_error_string() << std::endl;
                    return EXIT_FAILURE;
                }

                std::string addr;
                addr.resize(INET_ADDRSTRLEN);
                std::cout << "Accepted new connection from: "
                          << inet_ntop(AF_INET, &client_addr.sin_addr, &addr[0], addr.size()) << std::endl;
                addr.append("\n");
                send(client_sock, addr.c_str(), addr.size(), 0);

                event.data.fd = client_sock;
                event.events = EPOLLIN | EPOLLET;

                if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &event))
                {
                    std::cerr << sock_wrap.get_last_error_string() << std::endl;
                    return EXIT_FAILURE;
                }
            }
            else
            {
                std::string s_data;
                s_data.resize(1024);

                ssize_t bytes_count = recv(events[i].data.fd, &s_data[0], s_data.size(), 0);
                if (!bytes_count)
                {
                    std::cout << "Connection closed by client" << std::endl;
                    close(events[i].data.fd);
                }
                else
                {
                    std::cout << "Data: " << s_data << std::endl;
                    send(events[i].data.fd, s_data.c_str(), s_data.size(), 0);
                }
            }
        }
    }

    close(server_sock);
    close(epoll_fd);

    return EXIT_FAILURE;
}
