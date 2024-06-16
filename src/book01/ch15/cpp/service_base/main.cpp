#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <svcguid.h>

#include <array>
#include <cassert>
#include <iostream>


int main(int argc, char* argv[])
{
    HANDLE h_lookup = 0;
    WSAQUERYSET first_query = {};
    // SVCID_UDP(7)
    GUID guid = SVCID_ECHO_UDP;

    try
    {
        socket_wrapper::SocketWrapper sw;

        first_query.dwSize = sizeof(WSAQUERYSET);
        first_query.lpServiceClassId = &guid;

        if (FAILED(
                WSALookupServiceBegin(&first_query, LUP_RETURN_NAME | LUP_RETURN_COMMENT | LUP_RETURN_ADDR, &h_lookup)))
        {
            throw std::system_error(
                errno, std::system_category(), "Error on WSALookupServiceBegin: " + std::to_string(WSAGetLastError()));
        }

        uint32_t result = 0;

        while (true)
        {
            WSAQUERYSET test_query = {};
            DWORD length = sizeof(test_query);

            if (SUCCEEDED(WSALookupServiceNext(h_lookup, 0, &length, &test_query)))
            {
                std::cerr << "Impossible" << std::endl;
                break;
            }

            auto err_code = GetLastError();
            if (WSAEFAULT == err_code)
            {
                auto pdata_shared = std::make_shared<char[]>(length);
                WSAQUERYSET* p_data = reinterpret_cast<WSAQUERYSET*>(pdata_shared.get());

                if (FAILED(WSALookupServiceNext(h_lookup, 0, &length, p_data)))
                {
                    result = WSAGetLastError();
                    // Windows can return two errors if there is no more result
                    if ((WSA_E_NO_MORE == result) || (WSAENOMORE == result))
                    {
                        std::cout << "No more records found!" << std::endl;
                        break;
                    }
                    else
                        throw std::system_error(
                            errno, std::system_category(),
                            "Error on WSALookupServiceNext: " + std::to_string(WSAGetLastError()));
                }

                std::wcout << "Service instance name: " << p_data->lpszServiceInstanceName << "\n"
                           << "Name space num: " << p_data->dwNameSpace << "\n"
                           << "Address count:  " << p_data->dwNumberOfCsAddrs << std::endl;


                for (size_t i = 0; i < p_data->dwNumberOfCsAddrs; ++i)
                {
                    if (IPPROTO_UDP == p_data->lpcsaBuffer[i].iProtocol)
                    {
                        char addr[INET_ADDRSTRLEN];
                        const auto& sa =
                            reinterpret_cast<const sockaddr_in*>(p_data->lpcsaBuffer[i].RemoteAddr.lpSockaddr);
                        assert(AF_INET == sa->sin_family);

                        std::cout << inet_ntop(AF_INET, &sa->sin_addr, addr, INET_ADDRSTRLEN) << std::endl;
                    }
                }
            }
            else if (result != WSA_E_NO_MORE && result != WSAENOMORE)
            {
                break;
            }
            else
                throw std::system_error(
                    errno, std::system_category(),
                    "Error on WSALookupServiceNext: " + std::to_string(WSAGetLastError()));
        };

        if (WSALookupServiceEnd(h_lookup))
            throw std::system_error(
                errno, std::system_category(),
                "WSALookupServiceEnd(hlookup) failed with error code " + std::to_string(WSAGetLastError()));
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
