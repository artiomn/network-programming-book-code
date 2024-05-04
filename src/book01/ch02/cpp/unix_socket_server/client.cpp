extern "C"
{
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
}


#include <algorithm>
#include <cerrno>
#include <filesystem>
#include <iostream>
#include <string>


const std::string SOCK_PATH = "_socket_path";


int main(void)
{
    struct sockaddr_un sock_address;

    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (-1 == sock)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    sock_address.sun_family = AF_UNIX;
    std::copy(SOCK_PATH.begin(), SOCK_PATH.end(), sock_address.sun_path);

    const std::string msg{"Hello, process!"};

    ssize_t res = sendto(
        sock, msg.c_str(), msg.size(), 0, reinterpret_cast<const sockaddr*>(&sock_address), sizeof(sock_address));

    if (-1 == res)
    {
        perror("send");
        return EXIT_FAILURE;
    }

    close(sock);
    return EXIT_SUCCESS;
}
