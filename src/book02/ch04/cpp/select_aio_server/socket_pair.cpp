#include "socket_pair.h"

// socketpair() emulation for Windows, using local thread.
int windows_socketpair(SOCKET socks[2])
{
    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET) return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    // Automatic port selection.
    addr.sin_port = 0;

    if (SOCKET_ERROR == bind(listener, static_cast<sockaddr*>(&addr), sizeof(addr)))
    {
        closesocket(listener);
        return -1;
    }

    int len = sizeof(addr);
    if (SOCKET_ERROR == getsockname(listener, static_cast<sockaddr*>(&addr), &len) ||
        SOCKET_ERROR == listen(listener, 1))
    {
        closesocket(listener);
        return -1;
    }

    socks[0] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socks[0] == INVALID_SOCKET)
    {
        closesocket(listener);
        return -1;
    }

    // Unblocking mode.
    u_long flags = 1;
    ioctlsocket(socks[0], FIONBIO, &flags);

    // Async connection.
    connect(socks[0], static_cast<sockaddr*>(&addr), sizeof(addr));

    socks[1] = accept(listener, nullptr, nullptr);
    if (socks[1] == INVALID_SOCKET)
    {
        closesocket(socks[0]);
        closesocket(listener);
        return -1;
    }

    closesocket(listener);
    return 0;
}
