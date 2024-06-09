#include <exception>
#include <iomanip>
#include <iostream>

#if !defined(_WIN32)

extern "C"
{
#    include <signal.h>  // NOLINT
}


class thread_exit_exception : public std::exception
{
};

void sigpipe_handler(int sig)
{
    std::cout << "SIGPIPE signal trapped. Exiting thread." << std::endl;
    throw thread_exit_exception();
}

#endif

#include <socket_wrapper/socket_wrapper.h>

#include "proxy_server.h"


int main(int argc, const char *const argv[])
{
#if !defined(_WIN32)
    // Ignore SIGPIPE.
    ::signal(SIGPIPE, sigpipe_handler);
#endif

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        socket_wrapper::SocketWrapper sock_wrap;
        ProxyServer proxy(std::stoi(argv[1]));

        proxy.start();
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
