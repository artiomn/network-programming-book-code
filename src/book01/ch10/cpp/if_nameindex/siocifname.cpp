extern "C"
{
#include <net/if.h>
#include <sys/ioctl.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <string>


int main(int argc, const char *const argv[])
{
    if (argc != 3)
    {
        std::cerr << argv[0] << " <-n|-i> <if name|if index>" << std::endl;
        return EXIT_FAILURE;
    }

    const socket_wrapper::SocketWrapper sock_wrap;


    try
    {
        ifreq ifr = {};
        const socket_wrapper::Socket sock = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};

        if (!sock)
        {
            throw std::logic_error("socket");
        }

        std::string if_param(std::strlen(argv[1]), 0);
        std::transform(
            argv[1], argv[1] + std::strlen(argv[1]), if_param.begin(), [](unsigned char c) { return std::tolower(c); });

        if ("-n" == if_param)
        {
            std::copy_n(
                argv[2], std::min(strlen(argv[2]), static_cast<size_t>(IFNAMSIZ)), static_cast<char *>(ifr.ifr_name));

            if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0)
            {
                throw std::system_error(errno, std::system_category(), "SIOCGIFINDEX");
            }
            std::cout << "Interface \"" << argv[2] << "\""
                      << " index: " << ifr.ifr_ifindex << std::endl;
        }
        else if ("-i" == if_param)
        {
            const auto if_index = std::stoi(argv[2]);
            ifr.ifr_ifindex = if_index;

            if (ioctl(sock, SIOCGIFNAME, &ifr) < 0)
            {
                throw std::system_error(errno, std::system_category(), "SIOCGIFNAME");
            }
            std::cout << "interface " << if_index << " name: \"" << ifr.ifr_name << "\"" << std::endl;
        }
        else
        {
            std::cerr << "Please use '-n' for name "
                      << "or '-i' for index" << std::endl;
            return EXIT_FAILURE;
        }
    }
    catch (const std::system_error &e)
    {
        std::cerr << e.what() << ": " << sock_wrap.get_last_error_string() << "!" << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
