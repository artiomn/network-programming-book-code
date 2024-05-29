#include "ping.h"

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <node>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    const std::string host_name = {argv[1]};
    auto addrs = socket_wrapper::get_client_info(host_name, 0, SOCK_DGRAM);

    std::string addr_p;
    addr_p.resize(INET6_ADDRSTRLEN);
    auto si = reinterpret_cast<const sockaddr_in *>(addrs->ai_addr);
    inet_ntop(addrs->ai_family, &si->sin_addr, addr_p.data(), addr_p.size());

    std::cout << "Pinging \"" << host_name << "\" [" << addr_p << "]" << std::endl;

    socket_wrapper::Socket sock = {AF_INET, SOCK_DGRAM, IPPROTO_ICMP};

    if (!sock)
    {
        std::cerr << "Can't create socket: " << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "Raw socket was created..." << std::endl;
    }

    std::cout << "Starting to send packets..." << std::endl;
    // Send pings continuously.
    send_ping(
        sock, host_name, *reinterpret_cast<const sockaddr_in *>(addrs->ai_addr), false /*ip_headers_enabled*/,
        true /*set_recv_timeout*/);

    return EXIT_SUCCESS;
}
