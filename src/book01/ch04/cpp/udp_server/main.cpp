#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>


// Trim from end (in place).
static inline std::string &rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); }).base(), s.end());
    return s;
}


int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;

    const socket_wrapper::Socket sock = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    const auto addrs = socket_wrapper::get_serv_info(argv[1], SOCK_DGRAM);

    if (bind(sock, addrs->ai_addr, addrs->ai_addrlen) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        // Socket will be closed in the Socket destructor.
        return EXIT_FAILURE;
    }

    std::cout << "Starting echo server on the port " << argv[1] << "...\n";

    std::array<char, 256> buffer;

    // socket address used to store client address
    sockaddr_in client_address = {0};
    socklen_t client_address_len = sizeof(sockaddr_in);

    std::cout << "Running echo server...\n" << std::endl;
    std::array<char, INET_ADDRSTRLEN> client_address_buf;

    while (true)
    {
        // Read content into buffer from an incoming client.
        const ssize_t recv_len = recvfrom(
            sock, buffer.data(), buffer.size() - 1, 0, reinterpret_cast<sockaddr *>(&client_address),
            &client_address_len);

        if (recv_len > 0)
        {
            buffer[recv_len] = '\0';
            std::cout << "Client with address "
                      << inet_ntop(
                             AF_INET, &client_address.sin_addr, client_address_buf.data(), client_address_buf.size())
                      << ":" << ntohs(client_address.sin_port) << " sent datagram "
                      << "[length = " << recv_len << "]:\n'''\n"
                      << buffer.data() << "\n'''" << std::endl;
            // if ("exit" == command_string) run = false;
            // send(sock, &buf, readden, 0);

            /*            std::string command_string = {buffer, 0, len};
                        rtrim(command_string);
                        std::cout << command_string << std::endl;
            */
            // Send same content back to the client ("echo").
            sendto(
                sock, buffer.data(), recv_len, 0, reinterpret_cast<const sockaddr *>(&client_address),
                client_address_len);
        }
        else if (recv_len < 0)
            perror("recvfrom");

        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}
