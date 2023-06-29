/*
#ifndef UNICODE
#define UNICODE 1
#endif
*/

#include <objbase.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <stdio.h>

#include <vector>


int wmain()
{
    //-----------------------------------------
    // Declare and initialize variables
    WSADATA wsaData;

    INT iNuminfo = 0;

    // Allocate a 16K buffer to retrieve all the protocol providers
    DWORD dwBufferLen = 16384;

    // LPWSAPROTOCOL_INFO lpProtocolInfo = NULL;

    // variables needed for converting provider GUID to a string
    WCHAR GuidString[40] = {0};

    socket_wrapper::SocketWrapper sw;

    lpProtocolInfo = std::vector<char>(dwBufferLen);

    iNuminfo = WSAEnumProtocols(nullptr, lpProtocolInfo.data(), &dwBufferLen);
    if (SOCKET_ERROR == iNuminfo)
    {
        int iError = WSAGetLastError();
        if (iError != WSAENOBUFS)
        {
            wprintf(L"WSAEnumProtocols failed with error: %d\n", iError);
            return 1;
        }
        else
        {
            wprintf(L"WSAEnumProtocols failed with error: WSAENOBUFS (%d)\n", iError);
            wprintf(L"  Increasing buffer size to %d\n\n", dwBufferLen);

            lpProtocolInfo.resize(dwBufferLen);
            if (nullptr == lpProtocolInfo)
            {
                wprintf(L"Memory allocation increase for buffer failed\n");
                return 1;
            }
            iNuminfo = WSAEnumProtocols(NULL, lpProtocolInfo, &dwBufferLen);
            if (SOCKET_ERROR == iNuminfo)
            {
                iError = WSAGetLastError();
                wprintf(L"WSAEnumProtocols failed with error: %d\n", iError);
                return 1;
            }
        }
    }

    wprintf(L"WSAEnumProtocols succeeded with protocol count = %d\n\n", iNuminfo);
    for (int i = 0; i < iNuminfo; ++i)
    {
        wprintf(L"Winsock Catalog Provider Entry #%d\n", i);
        wprintf(L"----------------------------------------------------------\n");
        wprintf(L"Entry type:\t\t\t ");
        if (lpProtocolInfo[i].ProtocolChain.ChainLen == 1)
            wprintf(L"Base Service Provider\n");
        else
            wprintf(L"Layered Chain Entry\n");

        wprintf(L"Protocol:\t\t\t %ws\n", lpProtocolInfo[i].szProtocol);

        int iRet = StringFromGUID2(
            reinterpret_cast<LPWSAPROTOCOL_INFO>(lpProtocolInfo.data())[i] ProviderId, (LPOLESTR)&GuidString, 39);
        if (iRet == 0)
            wprintf(L"StringFromGUID2 failed\n");
        else
            wprintf(L"Provider ID:\t\t\t %ws\n", GuidString);

        wprintf(L"Catalog Entry ID:\t\t %u\n", lpProtocolInfo[i].dwCatalogEntryId);

        wprintf(L"Version:\t\t\t %d\n", lpProtocolInfo[i].iVersion);

        wprintf(L"Address Family:\t\t\t %d\n", lpProtocolInfo[i].iAddressFamily);
        wprintf(L"Max Socket Address Length:\t %d\n", lpProtocolInfo[i].iMaxSockAddr);
        wprintf(L"Min Socket Address Length:\t %d\n", lpProtocolInfo[i].iMinSockAddr);

        wprintf(L"Socket Type:\t\t\t %d\n", lpProtocolInfo[i].iSocketType);
        wprintf(L"Socket Protocol:\t\t %d\n", lpProtocolInfo[i].iProtocol);
        wprintf(L"Socket Protocol Max Offset:\t %d\n", lpProtocolInfo[i].iProtocolMaxOffset);

        wprintf(L"Network Byte Order:\t\t %d\n", lpProtocolInfo[i].iNetworkByteOrder);
        wprintf(L"Security Scheme:\t\t %d\n", lpProtocolInfo[i].iSecurityScheme);
        wprintf(L"Max Message Size:\t\t %u\n", lpProtocolInfo[i].dwMessageSize);

        wprintf(L"ServiceFlags1:\t\t\t 0x%x\n", lpProtocolInfo[i].dwServiceFlags1);
        wprintf(L"ServiceFlags2:\t\t\t 0x%x\n", lpProtocolInfo[i].dwServiceFlags2);
        wprintf(L"ServiceFlags3:\t\t\t 0x%x\n", lpProtocolInfo[i].dwServiceFlags3);
        wprintf(L"ServiceFlags4:\t\t\t 0x%x\n", lpProtocolInfo[i].dwServiceFlags4);
        wprintf(L"ProviderFlags:\t\t\t 0x%x\n", lpProtocolInfo[i].dwProviderFlags);

        wprintf(L"Protocol Chain length:\t\t %d\n", lpProtocolInfo[i].ProtocolChain.ChainLen);

        wprintf(L"\n");
    }

    return 0;
}
