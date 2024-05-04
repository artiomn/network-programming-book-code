#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#    define ioctl ioctlsocket
#else
extern "C"
{
#    include <netinet/tcp.h>
#    include <sys/ioctl.h>
    // #   include <fcntl.h>
}
#endif

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>


using std::chrono_literals::operator""ms;

const auto MAX_RECV_BUFFER_SIZE = 256;


bool send_request(socket_wrapper::Socket &sock, const std::string &request)
{
    size_t req_pos = 0;
    // cppcheck-suppress variableScope
    ssize_t bytes_count;
    auto const req_buffer = &(request.c_str()[0]);
    auto const req_length = request.length();

    while (true)
    {
        if ((bytes_count = send(sock, req_buffer + req_pos, req_length - req_pos, 0)) < 0)
        {
            if (EINTR == errno) continue;
            return false;
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


int main(int argc, const char *const argv[])
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

    const std::string host_name = {argv[1]};
    auto addrs = socket_wrapper::get_client_info(host_name, argv[2], SOCK_STREAM);

    if (connect(sock, addrs->ai_addr, addrs->ai_addrlen) != 0)
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
    if (ioctl(sock, FIONBIO, const_cast<IoctlType *>(&flag)) < 0)
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

        std::cout << "Sending request: \"" << request << "\"..." << std::endl;

        request += "\r\n";

        if (!send_request(sock, request))
        {
            std::cerr << sock_wrap.get_last_error_string() << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "Request was sent, reading response..." << std::endl;

        std::this_thread::sleep_for(2ms);

        while (true)
        {
            auto recv_bytes = recv(sock, buffer.data(), buffer.size() - 1, 0);

            std::cout << recv_bytes << " was received..." << std::endl;

            if (recv_bytes > 0)
            {
                buffer[recv_bytes] = '\0';
                std::cout << "------------\n"
                          << std::string(buffer.begin(), std::next(buffer.begin(), recv_bytes)) << std::endl;
                continue;
            }
            else if (-1 == recv_bytes)
            {
                if (EINTR == errno) continue;
                if (0 == errno) break;
                if (EAGAIN == errno) break;
                throw std::system_error(errno, std::system_category(), "recv");
            }

            break;
        }
    }

    return EXIT_SUCCESS;
}
