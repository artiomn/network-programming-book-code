#pragma once

#include <windows.h>

#include <cassert>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <system_error>

#include "scope_guard.h"
#include "socket_wrapper_impl.h"


namespace socket_wrapper
{

class SocketWrapperImpl : ISocketWrapperImpl
{
public:
    void initialize() override
    {
        WSADATA wsaData;
        // Initialize Winsock
        if (auto result = WSAStartup(MAKEWORD(2, 2), &wsaData); result != 0)
        {
            throw std::system_error(result, std::system_category(), "WSAStartup()");
        }
    }

    bool initialized() const noexcept override { return initialized_; }

    void deinitialize() noexcept override { WSACleanup(); }

    int get_last_error_code() const noexcept override { return WSAGetLastError(); }

    std::string get_last_error_string() const override
    {
        char *s = nullptr;
        if (!FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                get_last_error_code(), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), reinterpret_cast<LPTSTR>(&s), 0,
                nullptr))
        {
            throw std::system_error(GetLastError(), std::system_category(), "FormatMessage()");
        }

        const auto on_scope_exit = scope_guard([s](void *) { LocalFree(s); });
        assert(s != nullptr);
        return std::string(s);
    };

private:
    bool initialized_{false};
};

}  // namespace socket_wrapper
