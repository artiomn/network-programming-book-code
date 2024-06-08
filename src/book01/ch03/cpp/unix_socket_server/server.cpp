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
    struct sockaddr_un sock_address;

    int sock = socket(PF_UNIX, SOCK_DGRAM, 0);

    if (-1 == sock)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    sock_address.sun_family = AF_UNIX;
    assert(sizeof(SOCK_PATH) < sizeof(sock_address.sun_path));
    std::copy(std::begin(SOCK_PATH), std::end(SOCK_PATH), sock_address.sun_path);

    if (std::filesystem::exists(SOCK_PATH) && std::remove(SOCK_PATH) != 0)
    {
        perror("remove");
        return EXIT_FAILURE;
    }

    if (-1 == bind(sock, reinterpret_cast<const sockaddr *>(&sock_address), sizeof(sock_address)))
    {
        perror("bind");
        return EXIT_FAILURE;
    }

    std::string buf(1024, 0);

    while (true)
    {
        if (read(sock, &buf[0], buf.size()) < 0)
        {
            perror("server read");
            return EXIT_FAILURE;
        }

        std::cout << "Buffer: '" << buf << "'" << std::endl;
    }

    return EXIT_SUCCESS;
}
