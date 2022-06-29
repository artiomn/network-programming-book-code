#define  _CRT_SECURE_NO_WARNINGS

#include <cassert>
#include <cstring>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


const size_t command_size = 5;


int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    socket_wrapper::Socket sock = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    const std::string host_name = { argv[1] };
    const struct hostent *remote_host { gethostbyname(host_name.c_str()) };

    struct sockaddr_in server_addr =
    {
        .sin_family = AF_INET,
        .sin_port = htons(std::stoi(argv[2]))
    };

    server_addr.sin_addr.s_addr = *reinterpret_cast<const in_addr_t*>(remote_host->h_addr);

    if (0 == connect(sock, reinterpret_cast<const sockaddr* const>(&server_addr), sizeof(server_addr)))
    {
        std::string request(command_size, '\0');
        std::vector<uint8_t> buffer;
        buffer.resize(command_size);

        std::cout << "Connected to \"" << host_name << "\"..." << std::endl;

        while (true)
        {
            // !---> Error! Buffer size maybe low! <---!
            int scanf_result = scanf("%s", &request[0]);

            assert(scanf_result > 0 && scanf_result < buffer.size());

            request += "\n";

            if (send(sock, request.c_str(), static_cast<int>(request.length()), 0) < 0)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
        }

    }
    else
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

}

