#pragma once

#include <windows.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <system_error>

#include "socket_wrapper_impl.h"


namespace socket_wrapper
{

class SocketWrapperImpl : ISocketWrapperImpl
{
public:
    SocketWrapperImpl() : initialized_(false) {}

    void initialize() override
    {
        WSADATA wsaData;
        // Initialize Winsock
        auto result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0)
        {
            throw std::system_error(errno, std::system_category(), "WSAStartup()");
        }
    }

    bool initialized() const override { return initialized_; }

    void deinitialize() override { WSACleanup(); }

    int get_last_error_code() const override { return WSAGetLastError(); }

    std::string get_last_error_string() const override
    {
        // return std::strerror(std::errno);
        char *s = NULL;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
            get_last_error_code(), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), reinterpret_cast<LPTSTR>(&s), 0,
            nullptr);
        std::string result{s};
        LocalFree(s);

        return result;
    };

private:
    bool initialized_;
};

}  // namespace socket_wrapper
