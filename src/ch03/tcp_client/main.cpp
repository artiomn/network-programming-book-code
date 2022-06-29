#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#   define ioctl ioctlsocket
#else
extern "C"
{
#   include <netinet/tcp.h>
#   include <sys/ioctl.h>
// #   include <fcntl.h>
}
#endif

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


using namespace std::chrono_literals;

const auto MAX_RECV_BUFFER_SIZE = 256;


bool send_request(socket_wrapper::Socket &sock, const std::string &request)
{
    ssize_t bytes_count = 0;
    size_t req_pos = 0;
    auto const req_buffer = &(request.c_str()[0]);
    auto const req_length = request.length();

    while (true)
    {
        if ((bytes_count = send(sock, req_buffer + req_pos, req_length - req_pos, 0)) < 0)
        {
            if (EINTR == errno) continue;
        }
        else
        {
            if (!bytes_count) break;

            req_pos += bytes_count;

            if (req_pos >= req_length)
            {
                break;
            }
        }
    }

    return true;
}


int main(int argc, const char * const argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    socket_wrapper::Socket sock = {AF_INET, SOCK_STREAM, IPPROTO_TCP};

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

    if (connect(sock, reinterpret_cast<const sockaddr* const>(&server_addr), sizeof(server_addr)) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    std::string request;
    std::vector<char> buffer;
    buffer.resize(MAX_RECV_BUFFER_SIZE);

    std::cout << "Connected to \"" << host_name << "\"..." << std::endl;

    const IoctlType flag = 1;

    // Put the socket in non-blocking mode:
//#if !defined(_WIN32)
//    if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK) < 0)
//#else
    if (ioctl(sock, FIONBIO, const_cast<IoctlType*>(&flag)) < 0)
//#endif
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    // Disable Naggles's algorithm.
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&flag), sizeof(flag)) < 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Waiting for the user input..." << std::endl;

    while (true)
    {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, request)) break;

        std::cout
            << "Sending request: \"" << request << "\"..."
            << std::endl;

        request += "\r\n";

        if (!send_request(sock, request))
        {
            std::cerr << sock_wrap.get_last_error_string() << std::endl;
            return EXIT_FAILURE;
        }

        std::cout
            << "Request was sent, reading response..."
            << std::endl;

        std::this_thread::sleep_for(2ms);

        while (true)
        {
            auto recv_bytes = recv(sock, buffer.data(), buffer.size() - 1, 0);

            std::cout
                << recv_bytes
                << " was received..."
                << std::endl;

            if (recv_bytes > 0)
            {
                buffer[recv_bytes] = '\0';
                std::cout << "------------\n" << std::string(buffer.begin(), std::next(buffer.begin(), recv_bytes)) << std::endl;
                continue;
            }
            else if (-1 == recv_bytes)
            {
                if (EINTR == errno) continue;
                if (0 == errno) break;
                // std::cerr << errno << ": " << sock_wrap.get_last_error_string() << std::endl;
                break;
            }

            break;
        }
    }

    return EXIT_SUCCESS;
}

