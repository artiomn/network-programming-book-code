#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

extern "C"
{
#include <ip2string.h>
#include <iphlpapi.h>
#include <windows.h>
}

#include <iostream>
#include <vector>


int main()
{
    std::vector<IP_ADAPTER_ADDRESSES> adapters(1);
    uint32_t out_size = adapters.size() * sizeof(IP_ADAPTER_ADDRESSES);
    auto ret_code = GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapters.data(), &out_size);

    if (ret_code != NO_ERROR)
    {
        if (ERROR_BUFFER_OVERFLOW == ret_code)
        {
            std::cout << "Increasing buffer [" << adapters.size() << " -> "
                      << out_size / sizeof(IP_ADAPTER_ADDRESSES) + 1 << "]" << std::endl;
            adapters.resize(out_size / sizeof(IP_ADAPTER_ADDRESSES) + 1);
            out_size = adapters.size() * sizeof(IP_ADAPTER_ADDRESSES);

            if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapters.data(), &out_size) != NO_ERROR)
            {
                std::cerr << "Can't get adapters addresses!" << std::endl;
                return EXIT_FAILURE;
            }
        }
        else
        {
            std::cerr << "Can't get adapters addresses!" << std::endl;
            return EXIT_FAILURE;
        }
    }

    SOCKADDR adapter_addr = {0};

    // Network initialization need for the address conversion.
    socket_wrapper::SocketWrapper sw;

    for (auto adapter = adapters.data(); adapter; adapter = adapter->Next)
    {
        std::wcout << "Adapter name: " << adapter->FriendlyName << " (" << adapter->AdapterName << ")\n"
                   << "Description: " << adapter->Description << "\n"
                   << "IPv4 interface index: " << adapter->IfIndex << "\n"
                   << "IPv6 interface index: " << adapter->Ipv6IfIndex << "\n"
                   << "MTU: " << adapter->Mtu << "\n"
                   << "DNS Suffix: " << adapter->DnsSuffix << "\n"
                   << "Have IPv4: " << adapter->Ipv4Enabled << "\n"
                   << "Have IPv6: " << adapter->Ipv6Enabled << "\n"
                   << "Adapter type: ";

        switch (adapter->IfType)
        {
            case IF_TYPE_ETHERNET_CSMACD:
                std::cout << "Ethernet device\n";
                break;
            case IF_TYPE_PPP:
                std::cout << "PPP network interface\n";
                break;
            case IF_TYPE_SOFTWARE_LOOPBACK:
                std::cout << "Software loopback\n";
                break;
            case IF_TYPE_REGULAR_1822:
                std::cout << "1822\n";
                break;
            case IF_TYPE_IEEE80211:
                std::cout << "Wireless IEEE 802.11\n";
                break;
            default:
                std::cout << "Other\n";
        }

        std::cout << "Operational status: ";

        switch (adapter->OperStatus)
        {
            case IfOperStatusUp:
                std::cout << "IfOperStatusUp\n";
                break;
            case IfOperStatusDown:
                std::cout << "IfOperStatusDown\n";
                break;
            case IfOperStatusTesting:
                std::cout << "IfOperStatusTesting\n";
                break;
            case IfOperStatusUnknown:
                std::cout << "IfOperStatusUnknown\n";
                break;
            case IfOperStatusDormant:
                std::cout << "IfOperStatusDormant\n";
                break;
            case IfOperStatusNotPresent:
                std::cout << "IfOperStatusNotPresent\n";
                break;
            case IfOperStatusLowerLayerDown:
                std::cout << "IfOperStatusLowerLayerDown\n";
                break;
        };

        for (auto addr = adapter->FirstUnicastAddress; addr; addr = addr->Next)
        {
            std::string ip;
            ip.resize(INET_ADDRSTRLEN * 10);
            DWORD sz = ip.size() - 1;
            WSAAddressToStringA(addr->Address.lpSockaddr, addr->Address.iSockaddrLength, nullptr, ip.data(), &sz);

            std::cout << "My address = " << ip.c_str() << std::endl;
        }

        std::cout << std::endl;
    }

    return EXIT_FAILURE;
}
