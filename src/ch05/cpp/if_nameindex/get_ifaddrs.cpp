#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

#include <net/if.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <linux/if_packet.h>

#include <socket_wrapper/socket_headers.h>


void print_address(const sockaddr *ia)
{
    if (ia)
    {
        std::vector<char> addr_buf;
        const void *addr;

        if (AF_INET == ia->sa_family)
        {
            addr_buf.resize(INET_ADDRSTRLEN);
            addr = &(reinterpret_cast<const sockaddr_in *>(ia))->sin_addr;

            std::cout
                << " IPv4: "
                << inet_ntop(ia->sa_family, addr, &addr_buf[0], addr_buf.size());
        }
        else if (AF_INET6 == ia->sa_family)
        {
            addr_buf.resize(INET6_ADDRSTRLEN);
            addr = &(reinterpret_cast<const sockaddr_in6 *>(ia))->sin6_addr;
            std::cout
                << " IPv6: "
                << inet_ntop(ia->sa_family, addr, &addr_buf[0], addr_buf.size());
        }
        else if (AF_PACKET == ia->sa_family)
        {
            std::cout
                << " AF_PACKET: ";
            std::ios_base::fmtflags f(std::cout.flags());
            auto sa = reinterpret_cast<const sockaddr_ll*>(ia);

            for (const auto *i = sa->sll_addr; i < sa->sll_addr + sa->sll_halen; ++i)
                std::cout
                    << std::hex
                    << std::setfill('0')
                    << std::setw(2) << int(*i) << ":";
            std::cout.flags(f);
            std::cout
                << "\n    if index = " << sa->sll_ifindex;
        }
        else
        {
            std::cout
                << ia->sa_family
                << " addr type";
        }
    }
    else
    {
        std::cout << ": nullptr, but flag is set";
    }
}


int main(int argc, const char * const argv[])
{
    ifaddrs *ifa;
    if (getifaddrs(&ifa) < 0)
    {
        perror("getifaddrs");
        return EXIT_FAILURE;
    }

    std::unique_ptr<ifaddrs, decltype(&freeifaddrs)> if_ni(ifa, &freeifaddrs);

    for (auto i = if_ni.get(); i->ifa_next != nullptr; i = i->ifa_next)
    {
        std::cout
            << i->ifa_name << ":";

        if (i->ifa_addr)
        {
            std::cout << "\n    addr";
            print_address(i->ifa_addr);
        }

        if (i->ifa_netmask)
        {
            std::cout << "\n    netmask";
            print_address(i->ifa_netmask);
        }

        if (i->ifa_flags & IFF_BROADCAST)
        {
            std::cout << "\n    broadcast addr";
            print_address(i->ifa_broadaddr);
        }
        else if (i->ifa_flags & IFF_POINTOPOINT)
        {
            std::cout << "\n    point to point addr";
            print_address(i->ifa_dstaddr);
        }
        else
        {
            std::cout << "\n    no addr";
        }

        if (i->ifa_data)
        {
            std::cout << "\n";
            if (i->ifa_addr && AF_PACKET == i->ifa_addr->sa_family)
            {
                auto stats = static_cast<const rtnl_link_stats*>(ifa->ifa_data);
                std::cout
                    << "    tx_packets = " << stats->tx_packets << "\n"
                    << "    rx_packets = " << stats->rx_packets << "\n"
                    << "    tx_bytes  = " << stats->tx_bytes << "\n"
                    << "    rx_bytes   = " << stats->rx_bytes;
            }
            /*
            else if (i->ifa_addr && AF_LINK == i->ifa_addr->sa_family)
            {
                auto data = static_cast<const if_data*>(ifa->ifa_data);
			    std::cout
                    << "    receive_packets = " << int(data.ifi_ipackets) << "\n";
            } */
            else if (i->ifa_addr)
            {
                std::cout
                    << "    " << i->ifa_addr->sa_family
                    << " - unimplemented address family to parse data";
            }
            else
            {
                std::cout
                    << "    address is null, but data is not.";
            }
        }
        std::cout << "\n\n";
    }
    std::cout << std::endl;

   return EXIT_SUCCESS;
}
