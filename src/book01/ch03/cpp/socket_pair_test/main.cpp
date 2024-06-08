extern "C"
{
#include <sys/socket.h>

// For read() and write().
#include <unistd.h>
}

#include <cerrno>
#include <iostream>
#include <string>
#include <thread>


constexpr size_t buf_size = 1024;


void child(int socket)
{
    std::string buf(buf_size, 0);

    const ssize_t res = read(socket, &buf[0], buf.size());
    if (res < 0)
    {
        perror("child read");
        return;
    }

    std::cout << "Child received: '" << buf << "' (" << res << " bytes)" << std::endl;

    const std::string msg("Hello parent, I am child");

    if (write(socket, msg.c_str(), msg.size()) < 0)
    {
        perror("child write");
        return;
    }

    // Close could not be called,
    // but socket will be closed on process shutdown anyway
    close(socket);
}


void parent(int socket)
{
    std::string buf = "Hello, I'm your parent";

    buf.resize(buf_size);

    if (write(socket, buf.c_str(), buf.size()) < 0)
    {
        perror("parent write");
        return;
    }

    const ssize_t res = read(socket, &buf[0], buf.size());
    if (res < 0)
    {
        perror("parent read");
        return;
    }

    buf.resize(res);

    std::cout << "Parent received: '" << buf << "' (" << res << " bytes)" << std::endl;

    // Close could not be called,
    // but socket will be closed on process shutdown anyway
    close(socket);
}


int main()
{
    int fd[2];
    constexpr int parent_socket = 0;
    constexpr int child_socket = 1;

    if (-1 == socketpair(PF_LOCAL, SOCK_DGRAM, 0, fd))
    {
        perror("socketpair");
        return EXIT_FAILURE;
    }

    const pid_t pid = fork();

    if (0 == pid)
    {
        close(fd[parent_socket]);
        child(fd[child_socket]);
    }
    else if (-1 != pid)
    {
        close(fd[child_socket]);
        parent(fd[parent_socket]);
    }
    else
    {
        perror("fork");
    }

    return EXIT_SUCCESS;
}
