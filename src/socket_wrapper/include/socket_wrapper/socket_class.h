#pragma once

#include "socket_headers.h"

namespace socket_wrapper
{

class Socket final
{
public:
    Socket(int domain, int type, int protocol);
    explicit Socket(SocketDescriptorType socket_descriptor);

    Socket(const Socket &) = delete;
    Socket(Socket &&s);
    Socket &operator=(const Socket &s) = delete;
    Socket &operator=(Socket &&s);

    ~Socket();

public:
    bool opened() const noexcept;

public:
    explicit operator bool() const noexcept { return opened(); }
    operator SocketDescriptorType() const noexcept { return socket_descriptor_; }

public:
    int close() noexcept;

private:
    void open(int domain, int type, int protocol);

private:
    SocketDescriptorType socket_descriptor_{INVALID_SOCKET};
};

}  // namespace socket_wrapper
