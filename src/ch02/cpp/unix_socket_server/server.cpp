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

    int sock = socket(PF_UNIX, SOCK_DGRAM, 0);

    if (-1 == sock)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    sock_address.sun_family = AF_UNIX;
    std::copy(SOCK_PATH.begin(), SOCK_PATH.end(), sock_address.sun_path);
    std::remove(SOCK_PATH.c_str());

    if (-1 == bind(sock, reinterpret_cast<const sockaddr *>(&sock_address), sizeof(sock_address)))
    {
        perror("bind");
        return EXIT_FAILURE;
    }

    std::string buf;

    buf.resize(1024);

    while (true)
    {
        ssize_t bytes_read = read(sock, &buf[0], buf.size());

        if (bytes_read < 0)
        {
            perror("server read");
            return EXIT_FAILURE;
        }

        std::cout << "Buffer: '" << buf << "'" << std::endl;
    }

    return EXIT_SUCCESS;
}
