extern "C"
{
#include <linux/netlink.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
}

#include <cstdlib>
#include <iomanip>
#include <iostream>


const auto BUF_SIZE = 4096;


int main()
{
    char buf[BUF_SIZE];
    sockaddr_nl nls = {0};

    auto fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (-1 == fd)
    {
        return EXIT_FAILURE;
    }

    nls.nl_family = AF_NETLINK;
    nls.nl_pid = getpid();
    nls.nl_groups = 1;

    auto res = bind(fd, (struct sockaddr *)&nls, sizeof(nls));

    if (-1 == res)
    {
        return EXIT_FAILURE;
    }

    while (true)
    {
        auto len = recv(fd, buf, sizeof(buf), 0);

        std::cout << "\n<START>\n" << len << " bytes received..." << std::endl;
        for (int i = 0; i < len; ++i)
        {
            if (buf[i] == 0)
            {
                std::cout << "[0x00]\n";
            }
            else if (buf[i] < 33 || buf[i] > 126)
            {
                std::cout << "[0x" << std::hex << std::setw(2) << buf[i] << std::dec << "\n";
            }
            else
            {
                std::cout << static_cast<char>(buf[i]);
            }
        }
        std::cout << "<END>" << std::endl;
    }

    close(fd);
    return EXIT_SUCCESS;
}
