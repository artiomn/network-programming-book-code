#include <algorithm>
#include <exception>
#include <iostream>
#include <string>

extern "C"
{
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <linux/if.h>

}


std::string set_interface_name(const std::string &old_name, const std::string &new_name)
{
    if (new_name.size() >= IFNAMSIZ)
    {
        throw std::logic_error("Incorrect name size");
    }

    int sock = socket(PF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        throw std::system_error(errno, std::generic_category(), "Opening socket");
    }

    ifreq ifr = {0};

    std::copy(old_name.begin(), old_name.end(), ifr.ifr_name);
    std::copy(new_name.begin(), new_name.end(), ifr.ifr_newname);

    if (ioctl(sock, SIOCSIFNAME, &ifr) != 0)
    {
        auto e = errno;
        close(sock);
        throw std::system_error(e, std::generic_category(), "SIOCSIFNAME ioctl failed");
    }

    return ifr.ifr_name;
}


int main(int argc, const char * const argv[])
{
    if (argc != 3)
    {
        std::cerr
            << argv[0] << " <old_name> <new_name>"
            << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        std::cout
            << set_interface_name(argv[1], argv[2])
            << std::endl;
    }
    catch(const std::system_error &e)
    {
        std::cerr
            << e.what()
            << std::endl;
        return EXIT_FAILURE;
    }

   return EXIT_SUCCESS;
}

