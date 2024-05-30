#pragma once

#include <array>
#include <cerrno>
#include <cstring>
#include <string>

#include "socket_wrapper_impl.h"


namespace socket_wrapper
{

class SocketWrapperImpl : ISocketWrapperImpl
{
public:
    void initialize() override {}
    bool initialized() const noexcept override { return true; }
    void deinitialize() noexcept override {}
    int get_last_error_code() const noexcept override { return errno; }
    std::string get_last_error_string() const override
    {
        std::array<char, 256> buffer{};
        ::strerror_r(get_last_error_code(), buffer.data(), buffer.size() - 1);
        return std::string(buffer.data());
    };
};

}  // namespace socket_wrapper
