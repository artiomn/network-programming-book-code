#pragma once

#include <cerrno>
#include <cstring>
#include <string>

#include "socket_wrapper_impl.h"


namespace socket_wrapper
{

class SocketWrapperImpl : ISocketWrapperImpl
{
public:
    static constexpr size_t err_buffer_size = 256;

public:
    void initialize() override {}
    bool initialized() const noexcept override { return true; }
    void deinitialize() noexcept override {}
    int get_last_error_code() const noexcept override { return errno; }
    std::string get_last_error_string() const override
    {
        std::string buffer(err_buffer_size, '\0');
        ::strerror_r(get_last_error_code(), &buffer[0], buffer.size());
        return buffer;
    };
};

}  // namespace socket_wrapper
