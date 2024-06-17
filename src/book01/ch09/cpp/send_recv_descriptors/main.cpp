#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

extern "C"
{
#include <fcntl.h>
#include <sys/param.h>
#include <unistd.h>
}

#include <array>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


std::filesystem::path get_path_from_fd(int fd)
{
    char filename[PATH_MAX];

    const std::string link = "/proc/self/fd/" + std::to_string(fd);
    if (readlink(link.c_str(), filename, sizeof(filename)) < 0)
        throw std::system_error(errno, std::system_category(), "readlink");

    return std::filesystem::path(filename);
}


void send_descriptors(int socket, int fd)
{
    msghdr msg = {};
    std::array<char, 1> buf;
    std::array<char, CMSG_SPACE(sizeof(fd))> ancil_buf;

    iovec io = {.iov_base = &buf[0], .iov_len = buf.size()};

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = &ancil_buf[0];
    msg.msg_controllen = ancil_buf.size();

    cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));

    *reinterpret_cast<int *>(CMSG_DATA(cmsg)) = fd;

    if (sendmsg(socket, &msg, 0) < 0) throw std::system_error(errno, std::system_category(), "sendmsg");
}


int receive_descriptors(int socket)
{
    msghdr msgh = {};

    std::array<char, 255> buffer;
    iovec io = {.iov_base = &buffer[0], .iov_len = buffer.size()};
    msgh.msg_iov = &io;
    msgh.msg_iovlen = 1;

    std::array<char, 255> c_buffer;
    msgh.msg_control = &c_buffer[0];
    msgh.msg_controllen = c_buffer.size();

    if (recvmsg(socket, &msgh, 0) < 0) throw std::system_error(errno, std::system_category(), "recvmsg");

    int received_descriptor = -1;

    // Get ancillary data.
    for (cmsghdr *cmsg = CMSG_FIRSTHDR(&msgh); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msgh, cmsg))
    {
        if (SOL_SOCKET == cmsg->cmsg_level && SCM_RIGHTS == cmsg->cmsg_type)
        {
            if (received_descriptor < 0)
            {
                std::copy(CMSG_DATA(cmsg), CMSG_DATA(cmsg) + sizeof(received_descriptor), &received_descriptor);

                std::cout << "Descriptor was received: " << received_descriptor << std::endl;
            }
            else
            {
                int fd;
                std::copy(CMSG_DATA(cmsg), CMSG_DATA(cmsg) + sizeof(fd), &fd);
                close(fd);
            }
        }
    }

    if (-1 == received_descriptor)
    {
        throw std::logic_error("Descriptor receiving error");
    }

    return received_descriptor;
}


void parent(int sock)
{
    // Parent process.
    std::cout << "Parent started" << std::endl;

    char f_tmpl[] = "/tmp/descr_send_exampleXXXXXX";

    // Descriptor.
    // open(local_file.string().c_str(), O_RDWR | O_CREAT);
    const int file_fd = mkstemp(f_tmpl);

    if (file_fd < 0) throw std::system_error(errno, std::system_category(), "Parent opening local file");

    const std::filesystem::path local_file(get_path_from_fd(file_fd));

    std::cout << "Parent opened file with descriptor = " << file_fd << " [" << local_file << "]" << std::endl;

    send_descriptors(sock, file_fd);

    // File could not be closed and removed due to exceptions,
    // anyway it will be removed from tmp dir automatically
    close(file_fd);
    std::filesystem::remove(local_file);

    std::cout << "Parent exited" << std::endl;
}


void child(int sock)
{
    // Child process.
    std::cout << "Child" << std::endl;

    const int fd = receive_descriptors(sock);

    std::cout << "Child received descriptor = " << fd << " [" << get_path_from_fd(fd) << "]" << std::endl;

    std::array<char, 256> buffer;

    std::cout << "Reading from a file in the child..." << std::endl;

    for (ssize_t nbytes = read(fd, buffer.data(), buffer.size()); nbytes > 0;)
    {
        write(1, buffer.data(), nbytes);
    }

    // File could not be closed and removed due to exceptions,
    // anyway it will be closed on process shutdown
    close(fd);
    std::cout << "Child exited" << std::endl;
}


int main(int argc, char **argv)
{
    const socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        int sp[2];
        if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sp) != 0)
            throw std::system_error(sock_wrap.get_last_error_code(), std::system_category(), "socketpair");

        if (fork() > 0)
        {
            close(sp[1]);
            parent(sp[0]);
        }
        else
        {
            close(sp[0]);
            child(sp[1]);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
