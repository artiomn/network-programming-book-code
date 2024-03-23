extern "C"
{
#include <ip2string.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
}

#include <iostream>
#include <vector>


int main()
{
    OVERLAPPED overlap = {.hEvent = WSACreateEvent()};
    HANDLE hand = nullptr;

    if (NotifyRouteChange(&hand, &overlap) != NO_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            std::cerr << "NotifyRouteChange error: " << WSAGetLastError() << std::endl;
            return EXIT_FAILURE;
        }
    }

    MIB_IPFORWARDROW fr_row;

    std::vector<MIB_IPADDRTABLE> mib_table;
    ULONG mib_size = 0;
    if (GetIpAddrTable(nullptr, &mib_size, false) != ERROR_INSUFFICIENT_BUFFER)
    {
        std::cerr << "Incorrect GetIpAddrTable() call!" << std::endl;
        return EXIT_FAILURE;
    }

    mib_table.resize(mib_size / sizeof(MIB_IPADDRTABLE) + 1);

    if (GetIpAddrTable(mib_table.data(), &mib_size, false) != NO_ERROR)
    {
        std::cerr << "Incorrect GetIpAddrTable() call!" << std::endl;
        return EXIT_FAILURE;
    }

    std::string ip_addr;
    uint32_t my_ip_addr = 0;
    ip_addr.resize(INET_ADDRSTRLEN);

    for (size_t i = 0; i < mib_table[0].dwNumEntries; ++i)
    {
        auto row = mib_table[0].table[i];
        if (INET_IS_ADDR_LOOPBACK(AF_INET, &row.dwAddr)) continue;
        my_ip_addr = row.dwAddr;
        std::cout << "My IP = " << inet_ntop(AF_INET, &row.dwAddr, ip_addr.data(), ip_addr.size()) << std::endl;
        break;
    }

    uint32_t google_dns_addr;
    inet_pton(AF_INET, "8.8.8.8", &google_dns_addr);

    if (NO_ERROR == GetBestRoute(google_dns_addr, my_ip_addr, &fr_row))
    {
        std::cout << "1. Iface index: " << fr_row.dwForwardIfIndex << " ["
                  << inet_ntop(AF_INET, &fr_row.dwForwardNextHop, ip_addr.data(), ip_addr.size()) << "]" << std::endl;
    }

    if (WAIT_OBJECT_0 == WaitForSingleObject(overlap.hEvent, INFINITE))
        std::cout << "Routing table changed." << std::endl;

    CancelIPChangeNotify(&overlap);

    if (NO_ERROR == GetBestRoute(google_dns_addr, my_ip_addr, &fr_row))
    {
        std::cout << "2. Iface index: " << fr_row.dwForwardIfIndex << " ["
                  << inet_ntop(AF_INET, &fr_row.dwForwardNextHop, ip_addr.data(), ip_addr.size()) << "]" << std::endl;
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
