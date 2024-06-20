#include <cassert>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>

#if defined(WIN32)
// Include winsock headers firstly!
#    include <socket_wrapper/socket_headers.h>
#    include <windows.h>
#endif

#include <socket_wrapper/socket_wrapper.h>

#include "sniffer.h"


int main(int argc, const char* const argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <interface-ip> <capture-file>" << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        // Create a raw socket which supports IPv4 only.
        const socket_wrapper::SocketWrapper sock_wrap;

        assert(argv[1]);
        assert(argv[2]);
        if (!Sniffer(argv[1], argv[2], sock_wrap).start_capture()) throw std::runtime_error("Failed to start captuure");
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception!" << std::endl;
        return EXIT_FAILURE;
    }
    // Our socket and file will be closed automatically.
    return EXIT_SUCCESS;
}
