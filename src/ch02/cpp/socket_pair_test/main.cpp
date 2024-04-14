extern "C"
{
#include <sys/socket.h>

// Для read() и write().
#include <unistd.h>
}

#include <cerrno>
#include <iostream>
#include <string>
#include <thread>


const size_t buf_size = 1024;


void child(int socket)
{
    std::string buf;

    buf.resize(buf_size);

    ssize_t res = read(socket, &buf[0], buf.size());

    if (res < 0)
    {
        perror("child read");
        return;
    }

    std::cout << "Child received: '" << buf << "'" << std::endl;

    const std::string msg("Hello parent, I am child");

    write(socket, msg.c_str(), msg.size());
    close(socket);
}


void parent(int socket)
{
    std::string buf = "Hello, I'm your parent";

    buf.resize(buf_size);

    ssize_t res = write(socket, buf.c_str(), buf.size());

    if (res < 0)
    {
        perror("parent write");
        return;
    }

    res = read(socket, &buf[0], buf.size());

    if (res < 0)
    {
        perror("parent read");
        return;
    }

    buf.resize(res);

    std::cout << "Parent received: '" << buf << "'" << std::endl;

    close(socket);
}


int main()
{
    int fd[2];
    const int parent_socket = 0;
    const int child_socket = 1;
    pid_t pid;

    socketpair(PF_LOCAL, SOCK_DGRAM, 0, fd);

    pid = fork();

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
