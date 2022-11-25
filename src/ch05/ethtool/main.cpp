#include <algorithm>
#include <iostream>
#include <cstdint>
#include <string>

extern "C"
{
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>
}


const auto ETHTOOL_GSET = 0x1;
const auto SIOCETHTOOL  = 0x8946;


// GSET ("get settings") structure.
struct ethtool_cmd
{
    // Command.
    uint32_t    cmd;
    // Features this interface supports.
    uint32_t    supported;
    // Features this interface advertises.
    uint32_t    advertising;
    // The forced speed, 10Mb, 100Mb, gigabit.
    uint16_t    speed;
    // Duplex, half or full.
    uint8_t     duplex;
    // Which connector port.
    uint8_t     port;
    uint8_t     phy_address;
    // Which tranceiver to use.
    uint8_t     transceiver;
    // Enable or disable auto negotiation.
    uint8_t     autoneg;
    // Tx pkts before generating tx int.
    uint32_t    maxtxpkt;
    // Rx pkts before generating rx int.
    uint32_t    maxrxpkt;
    uint32_t    reserved[4];
};


void print_adapter_params(const std::string &name)
{
    ethtool_cmd cmd = {0};
    ifreq ifr = {0};

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    // ETHTOOL ioctl -> non-root permissions issues for old kernels.
    cmd.cmd = ETHTOOL_GSET;
    ifr.ifr_data = reinterpret_cast<caddr_t>(&cmd);
    std::copy_n(name.c_str(), IF_NAMESIZE, ifr.ifr_name);
    ifr.ifr_name[IF_NAMESIZE - 1] = '\0';
    if (ioctl(sock, SIOCETHTOOL, &ifr) < 0)
    {
        throw std::system_error(errno, std::system_category(), "SIOCETHTOOL IOCtl");
    }

    std::cout
        << "Iface = " << ifr.ifr_name << "\n"
        << "Speed = " << cmd.speed << "\n"
        << "Duplex = " << (cmd.duplex ? "half" : "full") << std::endl;
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

