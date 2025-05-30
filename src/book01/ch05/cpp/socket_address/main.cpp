#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <cassert>
#include <iomanip>
#include <iostream>
#include <string>


int main(int argc, const char *const argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    const socket_wrapper::SocketWrapper sock_wrap;
    const socket_wrapper::Socket sock = {AF_INET, SOCK_STREAM, IPPROTO_TCP};

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    assert(argv[1]);
    const std::string host_name = {argv[1]};

    assert(argv[2]);
    const auto addrs = socket_wrapper::get_client_info(host_name, argv[2], SOCK_STREAM);

    if (connect(sock, addrs->ai_addr, addrs->ai_addrlen) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in my_address{0};
    sockaddr_in his_address{0};

    socklen_t my_address_len(sizeof(my_address));
    socklen_t his_address_len(sizeof(his_address));

    if (getsockname(sock, reinterpret_cast<sockaddr *>(&my_address), &my_address_len) != 0)
    {
        std::cerr << "getsockname: " << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    if (getpeername(sock, reinterpret_cast<sockaddr *>(&his_address), &his_address_len) != 0)
    {
        std::cerr << "getpeername: " << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    std::string my_ip(INET_ADDRSTRLEN, 0);
    inet_ntop(AF_INET, &my_address.sin_addr, &my_ip[0], my_ip.size());

    std::string his_ip(INET_ADDRSTRLEN, 0);
    inet_ntop(AF_INET, &his_address.sin_addr, &his_ip[0], his_ip.size());

    std::string user_passed_ip(INET_ADDRSTRLEN, 0);
    auto si = reinterpret_cast<const sockaddr_in *>(addrs->ai_addr);
    inet_ntop(addrs->ai_family, &si->sin_addr, &user_passed_ip[0], user_passed_ip.size());

    std::cout << "User passed address: " << user_passed_ip.c_str() << " (" << host_name.c_str() << "):" << argv[2]
              << "\n"
              << "My address: " << my_ip.c_str() << ":" << ntohs(my_address.sin_port) << "\n"
              << "Another host address: " << his_ip.c_str() << ":" << ntohs(his_address.sin_port) << std::endl;

    return EXIT_SUCCESS;
}
