#include <cstdio>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <mstcpip.h>
#include <iphlpapi.h>


// This peace of code is from MS repo: https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/netds/winsock/rcvall/rcvall.c


int FormatAddress(SOCKADDR* sa, int salen, char* addrbuf, int addrbuflen)
{
    char    host[NI_MAXHOST],
        serv[NI_MAXSERV];
    int     hostlen = NI_MAXHOST,
        servlen = NI_MAXSERV,
        rc = 0;

    // Validate input
    if ((sa == NULL) || (addrbuf == NULL))
        return WSAEFAULT;

    // Format the name
    rc = getnameinfo(
        sa,
        salen,
        host,
        hostlen,
        serv,
        servlen,
        NI_NUMERICHOST | NI_NUMERICSERV     // Convert to numeric representation
    );
    if (rc != 0)
    {
        fprintf(stderr, "%s: getnameinfo failed: %d\n", __FILE__, rc);
        return rc;
    }
    if ((strlen(host) + strlen(serv) + 1) > (unsigned)addrbuflen)
        return WSAEFAULT;
    if (strncmp(serv, "0", 1) != 0)
    {
        if (sa->sa_family == AF_INET)
            _snprintf_s(addrbuf, addrbuflen, addrbuflen - 1, "%s:%s", host, serv);
        else if (sa->sa_family == AF_INET6)
            _snprintf_s(addrbuf, addrbuflen, addrbuflen - 1, "[%s]:%s", host, serv);
        else
            addrbuf[0] = '\0';
    }
    else
    {
        _snprintf_s(addrbuf, addrbuflen, addrbuflen - 1, "%s", host);
    }

    return NO_ERROR;
}


void PrintInterfaceList()
{
    int g_iFamilyMap[] = { AF_INET, AF_INET6 };
    SOCKET_ADDRESS_LIST* slist = NULL;
    SOCKET               s = INVALID_SOCKET;
    char* buf = NULL;
    DWORD                dwBytesRet = 0;
    int                  rc = 0,
        i = 0, j = 0, k = 0;

    k = 0;
    for (i = 0; i < sizeof(g_iFamilyMap) / sizeof(int); i++)
    {
        s = socket(g_iFamilyMap[i], SOCK_STREAM, 0);
        if (s != INVALID_SOCKET)
        {
            rc = WSAIoctl(
                s,
                SIO_ADDRESS_LIST_QUERY,
                NULL,
                0,
                NULL,
                0,
                &dwBytesRet,
                NULL,
                NULL
            );
            if ((rc == SOCKET_ERROR) && (GetLastError() == WSAEFAULT))
            {
                char addrbuf[INET6_ADDRSTRLEN] = { '\0' };

                // Allocate the necessary size
                buf = (char*)HeapAlloc(GetProcessHeap(), 0, dwBytesRet);
                if (buf == NULL)
                {
                    if (INVALID_SOCKET != s)
                    {
                        closesocket(s);
                        s = INVALID_SOCKET;
                    }

                    return;
                }

                rc = WSAIoctl(
                    s,
                    SIO_ADDRESS_LIST_QUERY,
                    NULL,
                    0,
                    buf,
                    dwBytesRet,
                    &dwBytesRet,
                    NULL,
                    NULL
                );
                if (rc == SOCKET_ERROR)
                {
                    if (buf) HeapFree(GetProcessHeap(), 0, buf);
                    if (INVALID_SOCKET != s)
                    {
                        closesocket(s);
                        s = INVALID_SOCKET;
                    }
                    return;
                }

                // Display the addresses
                slist = (SOCKET_ADDRESS_LIST*)buf;
                for (j = 0; j < slist->iAddressCount; j++)
                {
                    FormatAddress(
                        slist->Address[j].lpSockaddr,
                        slist->Address[j].iSockaddrLength,
                        addrbuf,
                        INET6_ADDRSTRLEN
                    );
                    printf("               %-2d ........ %s\n",
                        ++k, addrbuf);
                }

                if (buf) HeapFree(GetProcessHeap(), 0, buf);

            }
            else
            {
                // Unexpected failure
                fprintf(stderr, "WSAIoctl: SIO_ADDRESS_LIST_QUERY failed with unexpected error: %d\n",
                    WSAGetLastError());
            }
            if (INVALID_SOCKET != s)
            {
                closesocket(s);
                s = INVALID_SOCKET;
            }
        }
        else
        {
            fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
            return;
        }
    }
    return;
}


int main(int argc, const char * const argv[])
{
    socket_wrapper::SocketWrapper sw;
    PrintInterfaceList();
}
