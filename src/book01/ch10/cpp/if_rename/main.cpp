extern "C"
{
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

#include <algorithm>
#include <cassert>
#include <exception>
#include <iostream>
#include <string>


std::string set_interface_name(const std::string &old_name, const std::string &new_name)
{
    if (new_name.size() >= IFNAMSIZ)
    {
        throw std::logic_error("Incorrect name size");
    }

    const int sock = socket(PF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        throw std::system_error(errno, std::generic_category(), "Opening socket");
    }

    ifreq ifr = {0};

    std::copy_n(old_name.begin(), std::min(old_name.size(), static_cast<size_t>(IFNAMSIZ)), ifr.ifr_name);
    std::copy_n(new_name.begin(), std::min(new_name.size(), static_cast<size_t>(IFNAMSIZ)), ifr.ifr_newname);

    if (ioctl(sock, SIOCSIFNAME, &ifr) != 0)
    {
        const auto e = errno;
        close(sock);
        throw std::system_error(e, std::generic_category(), "SIOCSIFNAME ioctl failed");
    }

    return ifr.ifr_name;
}


int main(int argc, const char *const argv[])
{
    if (argc != 3)
    {
        std::cerr << argv[0] << " <old_name> <new_name>" << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        assert(argv[1]);
        assert(argv[2]);
        std::cout << set_interface_name(argv[1], argv[2]) << std::endl;
    }
    catch (const std::system_error &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
