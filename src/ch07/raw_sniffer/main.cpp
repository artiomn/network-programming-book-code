#include <exception>
#include <cstdlib>
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
        socket_wrapper::SocketWrapper sock_wrap;

		Sniffer sniffer(argv[1], argv[2], sock_wrap);
        sniffer.start_capture();
    }
    catch(...)
    {
        std::cerr << "Unknown exception!" << std::endl;
		return EXIT_FAILURE;
    }
	// Our socket and file will be closed automatically.
	return EXIT_SUCCESS;
}

