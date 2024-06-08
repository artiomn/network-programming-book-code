extern "C"
{
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
}


#include <algorithm>
#include <cassert>
#include <cerrno>
#include <filesystem>
#include <iostream>
#include <string>


constexpr char SOCK_PATH[]{"_socket_path"};


int main(void)
{
    sockaddr_un sock_address;

    const int sock = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (-1 == sock)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    sock_address.sun_family = AF_UNIX;
    assert(sizeof(SOCK_PATH) < sizeof(sock_address.sun_path));
    std::copy(std::begin(SOCK_PATH), std::end(SOCK_PATH), sock_address.sun_path);

    const std::string msg{"Hello, process!"};

    if (-1 ==
        sendto(
            sock, msg.c_str(), msg.size(), 0, reinterpret_cast<const sockaddr*>(&sock_address), sizeof(sock_address)))
    {
        perror("send");
        return EXIT_FAILURE;
    }

    close(sock);
    return EXIT_SUCCESS;
}
