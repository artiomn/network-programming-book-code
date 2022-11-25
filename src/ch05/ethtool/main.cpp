#include <algorithm>
#include <iostream>
#include <map>
#include <cstdint>
#include <string>

extern "C"
{
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
}


void print_adapter_params(const std::string &name)
{
    ethtool_cmd cmd = { .cmd = ETHTOOL_GSET };
    ifreq ifr = {0};

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    ifr.ifr_data = reinterpret_cast<caddr_t>(&cmd);
    std::copy_n(name.c_str(), IF_NAMESIZE, ifr.ifr_name);
    ifr.ifr_name[IF_NAMESIZE - 1] = '\0';

    if (ioctl(sock, SIOCETHTOOL, &ifr) < 0)
    {
        throw std::system_error(errno, std::system_category(), "SIOCETHTOOL IOCtl");
    }

    if (!ethtool_validate_speed(cmd.speed))
    {
        throw std::logic_error("Incorrect speed!");
    }

    if (!ethtool_validate_duplex(cmd.duplex))
    {
        throw std::logic_error("Incorrect duplex!");
    }

    const std::map<int, std::string> s_map =
    {
      {SPEED_10, "10 Mb/s"},
      {SPEED_100, "100 Mb/s"},
      {SPEED_1000, "1 Gb/s"},
      {SPEED_2500, "2.5 Gb/s"},
      {SPEED_10000, "10 Gb/s"},
      {SPEED_UNKNOWN, "Unknown"}
    };

    const auto speed = s_map.find(cmd.speed);
    if (s_map.end() == speed)
    {
        throw std::logic_error("Incorrect speed!");
    }

    std::cout
        << "Iface = " << ifr.ifr_name << "\n"
        << "Speed = " << speed->second << "\n"
        << "Duplex = "
        << (DUPLEX_HALF == cmd.duplex ? "half" :
               (DUPLEX_FULL == cmd.duplex ? "full" : "unknown"))
        << std::endl;
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

