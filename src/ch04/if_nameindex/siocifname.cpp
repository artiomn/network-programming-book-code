#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <string>

#include <sys/ioctl.h>
#include <net/if.h>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


int main(int argc, const char * const argv[])
{
    if (argc != 3)
    {
        std::cerr
            << argv[0]
            << " <-n|-i> <if name|if index>"
            << std::endl;
        return EXIT_FAILURE;
    }

    struct ifreq ifr = { 0 };
    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        socket_wrapper::Socket sock = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};

        if (!socket)
        {
            throw std::logic_error("socket");
        }

        std::string if_param;

        if_param.resize(std::strlen(argv[1]));
        std::transform(argv[1], argv[1] + std::strlen(argv[1]),
                       if_param.begin(), [](unsigned char c)
                       { return std::tolower(c); }
        );

        if ("-n" == if_param)
        {
            std::copy_n(argv[2], std::min(strlen(argv[2]), static_cast<size_t>(IFNAMSIZ)),
                        static_cast<char*>(ifr.ifr_name));

            if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0)
            {
                throw std::logic_error("SIOCGIFINDEX");
            }
            std::cout
                << "Interface \"" << argv[2] << "\""
                << " index: " << ifr.ifr_ifindex
                << std::endl;
        }
        else if ("-i" == if_param)
        {
            auto if_index = std::stoi(argv[2]);
            ifr.ifr_ifindex = if_index;

            if (ioctl(sock, SIOCGIFNAME, &ifr) < 0)
            {
                throw std::logic_error("SIOCGIFNAME");
            }
            std::cout
                << "interface " << if_index
                << " name: \"" << ifr.ifr_name << "\""
                << std::endl;
        }
        else
        {
            std::cerr
                << "Please use '-n' for name "
                << "or '-i' for index"
                << std::endl;
            return EXIT_FAILURE;
        }
    }
    catch (const std::logic_error &e)
    {
        std::cerr
            << e.what()
            << ": " << sock_wrap.get_last_error_string() << "!"
            << std::endl;
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

