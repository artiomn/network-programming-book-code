#pragma once

#include <string>


class ISocketWrapperImpl
{
public:
    virtual void initialize() = 0;
    virtual bool initialized() const noexcept = 0;
    virtual void deinitialize() noexcept = 0;
    virtual int get_last_error_code() const noexcept = 0;
    virtual std::string get_last_error_string() const = 0;
};
