#pragma once

#if defined(_WIN32)
extern "C"
{
#    include <winsock2.h>
#    include <ws2tcpip.h>
}

// See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32.
#    if !defined(_WIN32_WINNT)
#        define _WIN32_WINNT 0x0501 /* Windows XP. */
#    endif

using SocketDescriptorType = SOCKET;
using ssize_t = int;
using IoctlType = uint32_t;

#    if !defined(in_addr_t)
#        include <cinttypes>
using in_addr_t = uint32_t;
#    endif

#else  // not WIN32
// Assume that any non-Windows platform uses POSIX-style sockets instead.
extern "C"
{
#    include <arpa/inet.h>
#    include <sys/socket.h>
// Needed for getaddrinfo() and freeaddrinfo().
#    include <netdb.h>
// Needed for close().
#    include <unistd.h>
}

using SocketDescriptorType = int;
using IoctlType = int;

#endif

// Defined for Windows Sockets.
#if !defined(INVALID_SOCKET)
#    define INVALID_SOCKET (-1)
#endif

// Defined for Windows Sockets.
#if !defined(SOCKET_ERROR)
#    define SOCKET_ERROR (-1)
#endif
