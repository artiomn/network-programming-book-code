extern "C"
{
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>


void print_adapter_params(const std::string& name)
{
    ethtool_link_settings cmd = {.cmd = ETHTOOL_GLINKSETTINGS};
    ifreq ifr = {0};

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    std::copy_n(name.c_str(), std::min(static_cast<size_t>(IF_NAMESIZE), name.size()), ifr.ifr_name);
    ifr.ifr_data = reinterpret_cast<caddr_t>(&cmd);

    try
    {
        if (ioctl(sock, SIOCETHTOOL, &ifr) < 0)
        {
            throw std::system_error(errno, std::system_category(), "SIOCETHTOOL IOCtl");
        }

        // We expect a strictly negative value from kernel.
        if (cmd.link_mode_masks_nwords >= 0 || cmd.cmd != ETHTOOL_GLINKSETTINGS)
        {
            throw std::logic_error("Incorrect speed!");
        }

        // Got the real link_mode_masks_nwords,
        // Now send the real request.
        cmd.cmd = ETHTOOL_GLINKSETTINGS;
        cmd.link_mode_masks_nwords = -cmd.link_mode_masks_nwords;
        if (ioctl(sock, SIOCETHTOOL, &ifr) < 0)
        {
            throw std::system_error(errno, std::system_category(), "Cannot read speed on interface: " + name);
        }

        if (!ethtool_validate_speed(cmd.speed))
        {
            throw std::logic_error("Incorrect speed!");
        }

        if (!ethtool_validate_duplex(cmd.duplex))
        {
            throw std::logic_error("Incorrect duplex!");
        }

        const std::map<int, std::string> s_map = {{SPEED_10, "10 Mb/s"},    {SPEED_100, "100 Mb/s"},
                                                  {SPEED_1000, "1 Gb/s"},   {SPEED_2500, "2.5 Gb/s"},
                                                  {SPEED_10000, "10 Gb/s"}, {SPEED_UNKNOWN, "Unknown"}};

        const auto speed = s_map.find(cmd.speed);
        if (s_map.end() == speed)
        {
            throw std::logic_error("Incorrect speed!");
        }

        std::cout << "Iface = " << ifr.ifr_name << "\n"
                  << "Speed = " << speed->second << "\n"
                  << "Duplex = "
                  << (DUPLEX_HALF == cmd.duplex ? "half" : (DUPLEX_FULL == cmd.duplex ? "full" : "Unknown"))
                  << std::endl;
    }
    catch (const std::exception& e)
    {
        close(sock);
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    catch (...)
    {
        close(sock);
    }

    close(sock);
}


int main(int argc, const char* const argv[])
{
    if (argc < 2)
    {
        std::cerr << argv[0] << " <interface>" << std::endl;
        return EXIT_FAILURE;
    }

    print_adapter_params(argv[1]);

    return EXIT_SUCCESS;
}
