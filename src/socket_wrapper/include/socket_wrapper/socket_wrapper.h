#pragma once

#include <memory>
#include <string>


namespace socket_wrapper
{

class SocketWrapperImpl;

class SocketWrapper
{
public:
    SocketWrapper();
    ~SocketWrapper();

public:
    bool initialized() const noexcept;
    int get_last_error_code() const noexcept;
    std::string get_last_error_string() const;

private:
    std::unique_ptr<SocketWrapperImpl> impl_;
};

}  // namespace socket_wrapper
