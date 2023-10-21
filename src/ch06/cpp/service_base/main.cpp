#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_wrapper.h>

#include <winsock2.h>
#include <svcguid.h>
#include <iostream>

int main(int argc, char* argv[])
{
    uint32_t result = 0;
    HANDLE hLookup = 0;
    WSAQUERYSETW lpRestrict = {};
    GUID guid = SVCID_HOSTNAME;

    try
    {
        socket_wrapper::SocketWrapper sw;

        lpRestrict.dwSize = sizeof(WSAQUERYSETW);
        lpRestrict.lpServiceClassId = &guid;

        if (WSALookupServiceBeginW(&lpRestrict, LUP_RETURN_NAME, &hLookup) == SOCKET_ERROR)
        {
            throw std::runtime_error("Error on WSALookupServiceBegin: " + WSAGetLastError());
        }

        std::shared_ptr<WSAQUERYSETW> pData_shared = std::make_shared<WSAQUERYSETW>();
        WSAQUERYSETW* pData = pData_shared.get();
        DWORD length = sizeof(WSAQUERYSETW);

        while(true)
        {
            if (WSALookupServiceNextW(hLookup, 0, &length, pData) != 0)
            {
                result = WSAGetLastError();
                // Windows can return two errors if there is no more result
                if ((WSA_E_NO_MORE == result) || (WSAENOMORE == result))
                {
                    std::cout << "No more record found!" << std::endl;
                    break;
                }
                std::wcout << "WSALookupServiceNext() failed with error code " << WSAGetLastError() << std::endl;
                continue;
            }

            if (pData )
            {
                std::wcout << "  Service instance name: " << pData->lpszServiceInstanceName << std::endl;
                std::wcout << "  Name space num: " << pData->dwNameSpace << std::endl;
            }
        }

        if (WSALookupServiceEnd(hLookup))
            throw std::runtime_error("WSALookupServiceEnd(hLookup) failed with error code " + WSAGetLastError());
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << error.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}