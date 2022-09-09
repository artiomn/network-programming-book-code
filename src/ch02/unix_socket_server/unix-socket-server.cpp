#include <algorithm>
#include <cerrno>
#include <filesystem>
#include <iostream>
#include <string>

extern "C"
{
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
}


const std::string SOCK_PATH = "_socket_path";


int main(void)
{
    int s, s2, t, len;
    struct sockaddr_un local_address, remote;
    char str[100];

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);

    if (-1 == sock)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    local_address.sun_family = AF_UNIX;
    std::copy(SOCK_PATH.begin(), SOCK_PATH.end(), local_address.sun_path);
    std::remove(SOCK_PATH.c_str());

    if (-1 == bind(s, reinterpret_cast<const sockaddr *>(&local_address), sizeof(local_address)))
    {
        perror("bind");
        return EXIT_FAILURE;
    }

    if (-1 == listen(s, 5))
    {
        perror("listen");
        return EXIT_FAILURE;
    }

    while (true)
    {
        int done, n;
        std::cout
            << "Waiting for a connection..."
            << std::endl;
        t = sizeof(remote);
        if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1)
        {
            perror("accept");
            exit(1);
        }

        printf("Connected.\n");

        done = 0;
        do
        {
            n = recv(s2, str, 100, 0);
            if (n <= 0) {
                if (n < 0) perror("recv");
                done = 1;
            }

            if (!done) 
                if (send(s2, str, n, 0) < 0)
                {
                    perror("send");
                    done = 1;
                }
        } while (!done);

        close(s2);
    }

    return 0;
}

