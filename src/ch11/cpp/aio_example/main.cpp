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

    aiocb read_cb = {.aio_fildes = sock, .aio_buf = &buffer[0], .aio_nbytes = buffer_size};

    if (-1 == aio_write(&write_cb))
    {
        perror("aio_write");
        exit(EXIT_FAILURE);
    }

    if (-1 == aio_read(&read_cb))
    {
        perror("aio_read");
        exit(EXIT_FAILURE);
    }

    while (EINPROGRESS == aio_error(&read_cb))
    {
    }

    ssize_t read_bytes = aio_return(&read_cb);
    std::cout << "Read " << read_bytes << " bytes: " << buffer.data();

    shutdown(sock, SHUT_RDWR);

    return EXIT_SUCCESS;
}
