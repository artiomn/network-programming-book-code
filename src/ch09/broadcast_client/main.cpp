#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


int main(int argc, const char * const argv[])
{
    using namespace std::chrono_literals;

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;

    const int port { std::stoi(argv[1]) };

    std::cout << "Running sending on the port " << port << "...\n";

    struct sockaddr_in addr = {.sin_family = PF_INET, .sin_port = htons(port)};

    inet_pton(AF_INET, "127.255.255.255", &addr.sin_addr);
    // addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    socket_wrapper::Socket sock(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (!sock)
    {
        return EXIT_FAILURE;
    }

    int broadcast = 1;
    if (-1 == setsockopt(sock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&broadcast), sizeof(broadcast)))
    {
        throw std::runtime_error("setsockopt()");
    }

    std::string message = {"Test broadcast messaging!"};

    while (true)
    {
        std::cout << "Sending message to broadcast..." << std::endl;
        sendto(sock, message.c_str(), message.length(), 0, reinterpret_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
        std::cout << "Message was sent..." << std::endl;
        std::this_thread::sleep_for(1s);
    }
}
