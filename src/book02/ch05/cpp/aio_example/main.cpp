extern "C"
{
#include <aio.h>
#include <fcntl.h>
#include <unistd.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <cerrno>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>


const size_t buffer_size = 4096;


int main(int argc, const char *const argv[])
{
    if (argc != 4)
    {
        std::cerr << argv[0] << " <address> <port> <file path>" << std::endl;
        exit(EXIT_FAILURE);
    }

    socket_wrapper::SocketWrapper sock_wrap;
    socket_wrapper::Socket sock = {AF_INET, SOCK_STREAM, IPPROTO_TCP};

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    auto addr = socket_wrapper::get_client_info(argv[1], argv[2], SOCK_STREAM, AF_INET);
    std::string file_path{argv[3]};
    file_path += "\n";

    if (connect(sock, reinterpret_cast<const sockaddr *const>(addr->ai_addr), sizeof(sockaddr)) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<char> buffer(buffer_size);

    aiocb write_cb = {
        .aio_fildes = sock, .aio_buf = const_cast<char *>(file_path.c_str()), .aio_nbytes = file_path.size()};

    aiocb read_cb = {.aio_fildes = sock, .aio_buf = buffer.data(), .aio_nbytes = buffer.size()};

    if (-1 == aio_write(&write_cb))
    {
        throw std::system_error(errno, std::system_category(), "aio_write");
    }

    while (EINPROGRESS == aio_error(&write_cb))
    {
    }

    ssize_t write_bytes = aio_return(&write_cb);

    std::cout << "Written " << write_bytes << " bytes." << std::endl;

    if (-1 == aio_read(&read_cb))
    {
        throw std::system_error(errno, std::system_category(), "aio_read");
    }

    aiocb *rc_list[] = {&read_cb, nullptr};

    while (EINPROGRESS == aio_error(&read_cb))
    {
        aio_suspend(rc_list, sizeof(rc_list) / sizeof(rc_list[0]) - 1, nullptr);

        ssize_t read_bytes = aio_return(&read_cb);

        if (read_bytes <= 0)
        {
            break;
        }

        std::cout << std::string(buffer.begin(), buffer.begin() + read_bytes) << std::flush;

        if (-1 == aio_read(&read_cb))
        {
            throw std::system_error(errno, std::system_category(), "aio_read");
        }
    }

    std::cout << std::endl;

    shutdown(sock, SHUT_RDWR);

    return EXIT_SUCCESS;
}
