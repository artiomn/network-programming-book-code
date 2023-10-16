#include <winsock2.h>
#include <svcguid.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    DWORD result=0;
    HANDLE hLookup = 0;
    WSAQUERYSET lpRestrict;
    GUID guid = SVCID_HOSTNAME;

    try
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            throw std::runtime_error("Cannot startup Winsock");
        }

        ZeroMemory(&lpRestrict, sizeof(WSAQUERYSET));
        lpRestrict.dwSize = sizeof(WSAQUERYSET);
        lpRestrict.lpServiceClassId = &guid;

        if (WSALookupServiceBegin(&lpRestrict, LUP_RETURN_NAME, &hLookup) == SOCKET_ERROR)
        {
            throw std::runtime_error("Error on WSALookupServiceBegin: " + WSAGetLastError());
        }

        std::shared_ptr<WSAQUERYSET> pqs_shared = std::make_shared<WSAQUERYSET>();
        WSAQUERYSET *pqs = pqs_shared.get();
        DWORD length = sizeof(WSAQUERYSET);

        while(1)
        {
            if (WSALookupServiceNext(hLookup, 0, &length, pqs) != 0)
            {
                result = WSAGetLastError();
                if ((result == WSA_E_NO_MORE) || (result == WSAENOMORE))
                {
                    std::cout << "No more record found!" << std::endl;
                    break;
                }
                std::wcout << "WSALookupServiceNext() failed with error code " << WSAGetLastError() << std::endl;
                continue;
            }

            if (pqs )
            {
                std::wcout << "  Service instance name: " << pqs->lpszServiceInstanceName << std::endl;
                std::wcout << "  Name space num: " << pqs->dwNameSpace << std::endl;
            }
        }

        if (WSALookupServiceEnd(hLookup))
            throw std::runtime_error("WSALookupServiceEnd(hLookup) failed with error code " + WSAGetLastError());

        if (WSACleanup())
            throw std::runtime_error("WSACleanup() failed with error code " + WSAGetLastError());

        return 0;
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << error.what();
    }
}