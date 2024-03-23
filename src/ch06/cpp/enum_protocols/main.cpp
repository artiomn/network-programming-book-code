#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_wrapper.h>

extern "C"
{
#include <objbase.h>
}

#include <iostream>
#include <vector>


int main()
{
    // variables needed for converting provider GUID to a string
    WCHAR GuidString[40] = {0};

    socket_wrapper::SocketWrapper sw;
    
    // Allocate a 16K buffer to retrieve all the protocol providers.
    std::vector<WSAPROTOCOL_INFO> protocol_info(1);
    
    DWORD real_buffer_len = protocol_info.size() * sizeof(WSAPROTOCOL_INFO);

    auto info_count = WSAEnumProtocols(nullptr, protocol_info.data(), &real_buffer_len);

    if (SOCKET_ERROR == info_count)
    {
        int e_code = WSAGetLastError();
        if (e_code != WSAENOBUFS)
        {
            std::cerr << "WSAEnumProtocols failed with error: " << e_code << std::endl;
            return EXIT_FAILURE;
        }
        else
        {
            std::cerr << "WSAEnumProtocols failed with error: WSAENOBUFS " << e_code << std::endl;
            std::cout << "Increasing buffer size to " << real_buffer_len << std::endl;

            protocol_info.resize(real_buffer_len / sizeof(WSAPROTOCOL_INFO) + 1);
            real_buffer_len = protocol_info.size() * sizeof(WSAPROTOCOL_INFO);

            info_count = WSAEnumProtocols(NULL, protocol_info.data(), &real_buffer_len);
            if (SOCKET_ERROR == info_count)
            {
                std::cerr << "WSAEnumProtocols failed with error: " << WSAGetLastError() << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    std::cout << "WSAEnumProtocols succeeded with protocol count = " << info_count << std::endl;
    for (size_t i = 0; i < info_count; ++i)
    {
        std::cout
            << "Winsock Catalog Provider Entry " << i << "\n"
            << "Catalog Entry ID: " << protocol_info[i].dwCatalogEntryId << "\n"
            << "Version: " << protocol_info[i].iVersion << "\n"
            << "Entry type: "
            << ((protocol_info[i].ProtocolChain.ChainLen == 1) ? "Base Service Provider" : "Layered Chain Entry")
            << "\n"
            << "Protocol: " << protocol_info[i].szProtocol << "\n"
            << "Protocol Chain length: " << protocol_info[i].ProtocolChain.ChainLen << "\n"
            << std::endl;

        std::wstring guid_string;
        guid_string.resize(40);

        if (!StringFromGUID2(protocol_info[i].ProviderId, reinterpret_cast<LPOLESTR>(guid_string.data()), guid_string.size() - 1))
        {
            std::cerr << "StringFromGUID2 failed" << std::endl;
        }
        else
        {
            std::wcout << "Provider GUID: " << guid_string << std::endl;
        }

        std::cout
            << "Address Family: " << protocol_info[i].iAddressFamily << "\n"
            << "Max Socket Address Length: " << protocol_info[i].iMaxSockAddr << "\n"
            << "Min Socket Address Length: " << protocol_info[i].iMinSockAddr << "\n"
            << "Socket Type: " << protocol_info[i].iSocketType << "\n"
            << "Socket Protocol: " << protocol_info[i].iProtocol << "\n"
            << "Socket Protocol Max Offset: " << protocol_info[i].iProtocolMaxOffset << "\n"
            << "Network Byte Order: " << protocol_info[i].iNetworkByteOrder << "\n"
            << "Security Scheme: " << protocol_info[i].iSecurityScheme << "\n"
            << "Max Message Size: " << protocol_info[i].dwMessageSize << "\n"
            << std::endl;
    }

    return EXIT_SUCCESS;
}
