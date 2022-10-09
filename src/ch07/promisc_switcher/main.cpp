#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <iostream>

// Include winsock headers firstly!
#include <socket_wrapper/socket_headers.h>

#ifdef WIN32
#    include <mstcpip.h>
#    include <iphlpapi.h>
#else
#    include <sys/ioctl.h>
#    include <sys/socket.h>
// For IF_PROMISC.
#    include <linux/if.h>
// For ETH_P_ALL.
#    include <netinet/if_ether.h>
#endif

#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


int main(int argc, const char* const argv[])
{
	if (argc != 3)
    {
		std::cerr << "Usage: " << argv[0] << " <interface name or ip> <e|d>" << std::endl;
		return EXIT_FAILURE;
	}

    socket_wrapper::SocketWrapper sock_wrap;

    // Create a raw socket.
#if defined(WIN32)
    socket_wrapper::Socket sock = {AF_INET, SOCK_RAW, IPPROTO_IP};
#else
    socket_wrapper::Socket sock = {AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)};
#endif
    if (!sock)
    {
        std::cerr << "Socket error: " << sock_wrap.get_last_error_string() << "." << std::endl;
        return EXIT_FAILURE;
    }

    const std::string if_name = { argv[1] };
    std::string i_flag = argv[2];
    bool enable_promisc;

    std::transform(i_flag.begin(), i_flag.end(), i_flag.begin(), [](unsigned char c) { return std::tolower(c); });

    if ("e" == i_flag) enable_promisc = true;
    else if ("d" == i_flag) enable_promisc = false;
    else
    {
        std::cerr << "Unknown flag \"" << i_flag << "\"! Try \"e\" or \"d\"." << std::endl;
        return EXIT_FAILURE;
    }

#if defined(WIN32)
    // Give us ALL IPv4 packets sent and received to the specific IP address.
    // Possible values: RCVALL_ON - promisc mode, RCVALL_OFF, RCVALL_IPLEVEL, RCVALL_SOCKETLEVELONLY.
    DWORD value = enable_promisc ? RCVALL_ON : RCVALL_OFF;
    DWORD out = 0;

    sockaddr_in sa = { .sin_family = AF_INET, .sin_port = 0 };
    inet_pton(AF_INET, if_name.c_str(), &sa.sin_addr.s_addr);

    int rc = bind(sock, reinterpret_cast<const sockaddr*>(&sa), sizeof(sa));
    if (SOCKET_ERROR == rc)
    {
        std::cerr << "bind() failed: " << sock_wrap.get_last_error_string() << std::endl;
    }

    // The SIO_RCVALL control code enables a socket to receive all IPv4 or IPv6 packets passing through a network interface.
    // Another values: SIO_RCVALL, SIO_RCVALL_IGMPMCAST, SIO_RCVALL_MCAST.
    rc = WSAIoctl(sock, SIO_RCVALL, &value, sizeof(value), nullptr, 0, &out, nullptr, nullptr);

    if (SOCKET_ERROR == rc)
    {
        std::cerr << "WSAIotcl() failed: " << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    // Need to test, that interface in the promisc mode.
    getchar();
#else
    struct ifreq ifr = {0};
    std::copy(if_name.begin(), if_name.end(), ifr.ifr_name);

    if (-1 == ioctl(sock, SIOCGIFINDEX, &ifr))
    {
        std::cerr << "Unable to find interface \"" << if_name << "\" index!" << std::endl;
        return EXIT_FAILURE;
    }

    if (-1 == ioctl(sock, SIOCGIFFLAGS, &ifr))
    {
        std::cerr << "Unable to get interface flags!" << std::endl;
        return EXIT_FAILURE;
    }

    if (enable_promisc) ifr.ifr_flags |= IFF_PROMISC;
    else ifr.ifr_flags &= ~IFF_PROMISC;

    if (-1 == ioctl(sock, SIOCSIFFLAGS, &ifr))
    {
        std::cerr << "Unable to set promisc mode!" << std::endl;
        return EXIT_FAILURE;
    }
#endif
    return EXIT_SUCCESS;
}

