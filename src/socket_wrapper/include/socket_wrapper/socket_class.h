#pragma once

#include "socket_headers.h"

namespace socket_wrapper
{

class Socket
{
public:
    Socket(int domain, int type, int protocol);
    explicit Socket(SocketDescriptorType socket_descriptor);

    Socket(const Socket &) = delete;
    Socket(Socket &&s);
    Socket &operator=(const Socket &s) = delete;
    Socket &operator=(Socket &&s);

    virtual ~Socket();

public:
    bool opened() const;

public:
    operator bool() const { return opened(); }
    operator SocketDescriptorType() const { return socket_descriptor_; }

public:
    int close();

protected:
    void open(int domain, int type, int protocol);

private:
    SocketDescriptorType socket_descriptor_;
};

}  // namespace socket_wrapper
