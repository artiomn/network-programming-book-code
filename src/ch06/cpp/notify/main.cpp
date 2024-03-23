extern "C"
{
#include <iphlpapi.h>
#include <windows.h>
#include <winsock2.h>
}

#include <iostream>


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

    if (WAIT_OBJECT_0 == WaitForSingleObject(overlap.hEvent, INFINITE))
        std::cout << "Routing table changed." << std::endl;

    MIB_IPFORWARDROW fr_row;

    if (NO_ERROR == GetBestRoute(inet_addr("8.8.8.8"), inet_addr("127.0.0.1"), &fr_row))
    {
        std::cout << "Iface index: " << fr_row.dwForwardIfIndex << " [" << fr_row.dwForwardNextHop << "]" << std::endl;
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
