#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


const size_t buffer_size = 256;


int main(int argc, char const * const argv[])
{
    using namespace std::chrono_literals;

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;

    const int port { std::stoi(argv[1]) };

    std::cout << "Receiving messages on the port " << port << "...\n";

    struct sockaddr_in addr = {.sin_family = PF_INET, .sin_port = htons(port)};
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    socket_wrapper::Socket sock(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&broadcast), sizeof(broadcast));

    if (bind(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(sockaddr)) == -1)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    char buffer[buffer_size] = {};

    while (true)
    {

        if (recv(sock, buffer, sizeof(buffer) - 1, 0) < 0)
        {
            std::cerr << sock_wrap.get_last_error_string() << std::endl;
            return EXIT_FAILURE;
        }
        std::cout << buffer << std::endl;
    }
}
