#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

extern "C"
{
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
}

#include <iostream>


int main(int argc, const char *const argv[])
{
    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        int ttl_val = 0;
        socklen_t ttl_val_len = sizeof(ttl_val);

        auto sock = socket_wrapper::Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (getsockopt(sock, SOL_IP, IP_TTL, &ttl_val, &ttl_val_len) != 0)
        {
            throw std::system_error(errno, std::system_category(), "TTL getting failed");
        }

        std::cout << "Old TTL = " << ttl_val << std::endl;
        ttl_val = 255;
        std::cout << "Setting TTL = " << ttl_val << std::endl;

        if (setsockopt(sock, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
        {
            throw std::system_error(errno, std::system_category(), "TTL setting failed");
        }

        ttl_val = 0;

        if (getsockopt(sock, SOL_IP, IP_TTL, &ttl_val, &ttl_val_len) != 0)
        {
            throw std::system_error(errno, std::system_category(), "TTL getting failed");
        }
        std::cout << "New TTL = " << ttl_val << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
