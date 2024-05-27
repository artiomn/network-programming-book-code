#include <socket_wrapper/socket_wrapper.h>

#ifdef _WIN32
#    include "socket_wrapper_windows.h"
#else
#    include "socket_wrapper_unix.h"
#endif


namespace socket_wrapper
{


SocketWrapper::SocketWrapper() : impl_{std::make_unique<SocketWrapperImpl>()}
{
    impl_->initialize();
}


SocketWrapper::~SocketWrapper()
{
    impl_->deinitialize();
}


bool SocketWrapper::initialized() const noexcept
{
    return impl_->initialized();
}


int SocketWrapper::get_last_error_code() const noexcept
{
    return impl_->get_last_error_code();
}


std::string SocketWrapper::get_last_error_string() const
{
    return impl_->get_last_error_string();
}

}  // namespace socket_wrapper
