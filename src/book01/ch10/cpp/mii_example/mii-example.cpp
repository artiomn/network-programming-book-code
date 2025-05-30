extern "C"
{
#include <linux/mii.h>
#include <linux/sockios.h>
#include <linux/types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>


int main(int argc, const char *const argv[])
{
    if (argc < 2)
    {
        std::cout << argv[0] << " <interface>" << std::endl;
        return EXIT_FAILURE;
    }

    assert(argv[1]);
    std::string iface_name{argv[1]};

    std::cout << "Interface: " << iface_name << std::endl;

    ifreq ifr = {0};
    std::copy_n(iface_name.c_str(), std::min(static_cast<size_t>(IF_NAMESIZE - 1), iface_name.size()), ifr.ifr_name);

    const int sock = socket(PF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    if (ioctl(sock, SIOCGMIIPHY, &ifr) < 0)
    {
        close(sock);
        throw std::system_error(errno, std::system_category(), "SIOCFMIIPHY");
    }

    mii_ioctl_data *mii_data = reinterpret_cast<mii_ioctl_data *>(&ifr.ifr_data);

    for (int i = 0; i < 0x1c; ++i)
    {
        mii_data->reg_num = i;
        if (ioctl(sock, SIOCGMIIREG, &ifr) < 0)
        {
            std::array<char, 256> buffer{};

            std::cerr << "Ioctl error: " << ::strerror_r(errno, buffer.data(), buffer.size()) << std::endl;
            continue;
        }
        std::cout << std::hex << "PHY addr: 0x" << mii_data->phy_id << ", reg: 0x" << mii_data->reg_num << ", value: 0x"
                  << mii_data->val_out << std::endl;
    }

    close(sock);
    return EXIT_SUCCESS;
}
