#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_wrapper.h>

#include <svcguid.h>
#include <iostream>


int main(int argc, char* argv[])
{
    uint32_t result = 0;
    HANDLE hlookup = 0;
    WSAQUERYSETW lp_restrict = {};
    GUID guid = SVCID_HOSTNAME;

    try
    {
        socket_wrapper::SocketWrapper sw;

        lp_restrict.dwSize = sizeof(WSAQUERYSETW);
        lp_restrict.lpServiceClassId = &guid;

        if (SOCKET_ERROR == WSALookupServiceBeginW(&lp_restrict, LUP_RETURN_NAME, &hlookup))
        {
            throw std::system_error(errno, std::system_category(), "Error on WSALookupServiceBegin: " + std::to_string(WSAGetLastError()));
        }

        std::shared_ptr<WSAQUERYSETW> pdata_shared = std::make_shared<WSAQUERYSETW>();
        WSAQUERYSETW* pdata = pdata_shared.get();
        DWORD length = sizeof(WSAQUERYSETW);

        while (true)
        {
            if (WSALookupServiceNextW(hlookup, 0, &length, pdata) != 0)
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

            if (pdata)
            {
                std::wcout << "  Service instance name: " << pdata->lpszServiceInstanceName << std::endl;
                std::wcout << "  Name space num: " << pdata->dwNameSpace << std::endl;
            }
        }

        if (WSALookupServiceEnd(hlookup))
            throw std::system_error(errno, std::system_category(), "WSALookupServiceEnd(hlookup) failed with error code " + std::to_string(WSAGetLastError()));
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
