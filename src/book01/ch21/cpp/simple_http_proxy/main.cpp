#include <cassert>
#include <exception>
#include <iomanip>
#include <iostream>

#if !defined(_WIN32)

extern "C"
{
#    include <signal.h>
}


class exit_exception : public std::exception
{
};

void sigpipe_handler(int s)
{
    std::cout << "SIGPIPE signal trapped. Terminating." << std::endl;
    // TODO: This is not fatal, please use proper handling
    throw exit_exception();
}

#endif  // _WIN32

#include <socket_wrapper/socket_wrapper.h>

#include "proxy_server.h"


int main(int argc, const char *const argv[])
{
#if !defined(_WIN32)
    // Ignore SIGPIPE.
    ::signal(SIGPIPE, sigpipe_handler);
#endif  // _WIN32

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        const socket_wrapper::SocketWrapper sock_wrap;
        assert(argv[1]);
        ProxyServer(std::stoi(argv[1])).start();
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
