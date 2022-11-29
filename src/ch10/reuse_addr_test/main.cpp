#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


const size_t buffer_size = 256;


int main(int argc, char const * const argv[])
{
    if (argc != 2)
    {
          std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
          return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    sockaddr_in si_other;

    const int port { std::stoi(argv[1]) };

    std::cout << "Trying bind on the port " << port << "...\n";

    struct sockaddr_in addr1 = {.sin_family = PF_INET,
                                .sin_port = htons(port),
                                .sin_addr = {.s_addr = INADDR_ANY}};


    struct sockaddr_in addr2 = {.sin_family = PF_INET,
                                .sin_port = htons(port),
                                .sin_addr = {.s_addr = INADDR_ANY}};

    socket_wrapper::Socket sock1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    socket_wrapper::Socket sock2(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (!(sock1 && sock2))
    {
          std::cerr << sock_wrap.get_last_error_string() << std::endl;
          return EXIT_FAILURE;
    }

    int flag = 1;

    setsockopt(sock1, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    setsockopt(sock2, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    if (bind(sock1, reinterpret_cast<const sockaddr *>(&addr1),
             sizeof(sockaddr)) == -1)
    {
          std::cerr << sock_wrap.get_last_error_string() << std::endl;
          return EXIT_FAILURE;
    }

    if (bind(sock2, reinterpret_cast<const sockaddr *>(&addr2),
               sizeof(sockaddr)) == -1)
    {
          std::cerr << sock_wrap.get_last_error_string() << std::endl;
          return EXIT_FAILURE;
    }
    std::cout << "Bind is ok..." << std::endl;
}
