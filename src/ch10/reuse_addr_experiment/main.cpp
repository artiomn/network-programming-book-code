#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


void bind_sockets(const socket_wrapper::SocketWrapper &sock_wrap,
                  const std::string &address1,
                  const std::string &address2,
                  unsigned short port,
                  int option, const int flag,
                  bool add_listen = false)
{
    struct sockaddr_in addr1 = { .sin_family = PF_INET, .sin_port = htons(port) };
    addr1.sin_addr.s_addr = htonl(INADDR_ANY);
    struct sockaddr_in addr2 = { .sin_family = PF_INET, .sin_port = htons(port) };
    addr2.sin_addr.s_addr = htonl(INADDR_ANY);

    inet_pton(AF_INET, address1.c_str(), &addr1.sin_addr);
    inet_pton(AF_INET, address2.c_str(), &addr2.sin_addr);

    std::cout
        << "Address 1: " << address1 << '\n'
        << "Address 2: " << address2
        << std::endl;

    socket_wrapper::Socket sock1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    socket_wrapper::Socket sock2(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    setsockopt(sock1, SOL_SOCKET, option, reinterpret_cast<const char*>(&flag), sizeof(flag));
    setsockopt(sock2, SOL_SOCKET, option, reinterpret_cast<const char*>(&flag), sizeof(flag));

    if (!(sock1 && sock2))
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return;
    }

    std::cout << "Binding socket 1..." << std::endl;
    bool bind_ok = true;
    if (bind(sock1, reinterpret_cast<const sockaddr *>(&addr1), sizeof(sockaddr)) == -1)
    {
        std::cerr
            << "Bind error: "
            << sock_wrap.get_last_error_string()
            << std::endl;
        bind_ok = false;
    }

    if (add_listen && bind_ok)
    {
        std::cout << "Listening on socket 1..." << std::endl;
        if (listen(sock1, 1) == -1)
        {
            std::cerr
                << "Listen error: "
                << sock_wrap.get_last_error_string()
                << std::endl;
            return;
        }
    }

    std::cout << "Binding socket 2..." << std::endl;
    if (bind(sock2, reinterpret_cast<const sockaddr *>(&addr2), sizeof(sockaddr)) == -1)
    {
        std::cerr
            << "Bind error: "
            << sock_wrap.get_last_error_string()
            << std::endl;
        bind_ok = false;
    }
    else
    {
        std::cout << "Bind is ok..." << std::endl;
    }

    if (add_listen && bind_ok)
    {
        std::cout << "Listening on socket 2..." << std::endl;
        if (listen(sock2, 1) == -1)
        {
            std::cerr
                << "Listen error: "
                << sock_wrap.get_last_error_string()
                << std::endl;
            return;
        }
    }
}


int main(int argc, char const * const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;

    const int port { std::stoi(argv[1]) };

    std::cout << "-SO_REUSEADDR unset: trying bind on the port " << port << std::endl;
    bind_sockets(sock_wrap, "0.0.0.0", "0.0.0.0", port, SO_REUSEADDR, 0);
    std::cout << "+SO_REUSEADDR set: trying bind on the port " << port << std::endl;
    bind_sockets(sock_wrap, "0.0.0.0", "0.0.0.0", port, SO_REUSEADDR, 1);
    std::cout << "+SO_REUSEADDR set: trying bind on the port " << port << std::endl;
    bind_sockets(sock_wrap, "0.0.0.0", "0.0.0.0", port, SO_REUSEADDR, 1, true);

    std::cout << "-SO_REUSEADDR unset: trying bind on the port " << port << std::endl;
    bind_sockets(sock_wrap, "0.0.0.0", "127.0.0.1", port, SO_REUSEADDR, 0);
    std::cout << "+SO_REUSEADDR set: trying bind on the port " << port << std::endl;
    bind_sockets(sock_wrap, "0.0.0.0", "127.0.0.1", port, SO_REUSEADDR, 1);

    std::cout << "\n-SO_REUSEADDR unset: trying bind on the port " << port << std::endl;
    bind_sockets(sock_wrap, "127.0.0.1", "127.0.0.2", port, SO_REUSEADDR, 0);
    std::cout << "+SO_REUSEADDR set: trying bind on the port " << port << std::endl;
    bind_sockets(sock_wrap, "127.0.0.1", "127.0.0.2", port, SO_REUSEADDR, 1);

    std::cout << "\n-SO_REUSEADDR unset: trying bind on the port " << port << std::endl;
    bind_sockets(sock_wrap, "127.0.0.1", "127.0.0.1", port, SO_REUSEADDR, 0);
    std::cout << "+SO_REUSEADDR set: trying bind on the port " << port << std::endl;
    bind_sockets(sock_wrap, "127.0.0.1", "127.0.0.1", port, SO_REUSEADDR, 1);

#if !defined(_WIN32)
    std::cout << "\n-SO_REUSEPORT unset: trying bind on the port " << port << std::endl;
    bind_sockets(sock_wrap, "127.0.0.1", "127.0.0.1", port, SO_REUSEPORT, 0, true);
    std::cout << "+SO_REUSEPORT set: trying bind on the port " << port << std::endl;
    bind_sockets(sock_wrap, "127.0.0.1", "127.0.0.1", port, SO_REUSEPORT, 1, true);
#endif
    std::cout << "Finished, press any key..." << std::endl;
    getchar();
}
