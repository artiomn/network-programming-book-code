#if !defined(_GNU_SOURCE)
#    define _GNU_SOURCE
#endif

extern "C"
{
#include <netinet/in.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/wait.h>
}

#include <iostream>
#include <string>


void print_hostname()
{
    utsname uts;

    // Retrieve and display hostname.
    if (-1 == uname(&uts)) exit(EXIT_FAILURE);
    std::cout << uts.nodename << std::endl;
}


int make_socket(unsigned short port)
{
    const sockaddr_in addr = {AF_INET, port, INADDR_ANY, 0};

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (-1 == sock) return -1;
    if (-1 == bind(sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)))
    {
        perror("bind");
        close(sock);
        return -1;
    }

    if (-1 == listen(sock, SOMAXCONN))
    {
        perror("listen");
        close(sock);
        return -1;
    }

    std::cout << "Listening on port " << port << std::endl;

    return sock;
}


static int child(void *arg)
{
    // Change hostname in UTS namespace of child.
    const std::string new_hostname(static_cast<char *>(arg));

    if (-1 == sethostname(new_hostname.c_str(), new_hostname.size()))
    {
        return EXIT_FAILURE;
    }
    std::cout << "uts.nodename in child: ";
    print_hostname();

    auto sock = make_socket(8080);

    sleep(1);
    close(sock);

    return EXIT_SUCCESS;
}


int main(int argc, char *argv[])
{
    constexpr size_t stack_size = 1024 * 1024;

    std::cout << "uts.nodename in parent before run: ";
    print_hostname();

    // Allocate memory to be used for the stack of the child.
    char *stack = static_cast<char *>(
        mmap(nullptr, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0));

    if (MAP_FAILED == stack) return EXIT_FAILURE;

    char *stack_top = stack + stack_size;


    auto sock = make_socket(8080);

    std::string new_hostname{"new_host"};

    // Create child that has its own UTS and network namespaces.
    pid_t pid = clone(child, stack_top, CLONE_NEWUTS | CLONE_NEWNET | SIGCHLD, new_hostname.data());
    if (-1 == pid)
    {
        perror("clone");
        return EXIT_FAILURE;
    }
    std::cout << "clone() returned " << pid << std::endl;

    sleep(1);

    std::cout << "uts.nodename in parent: ";
    print_hostname();

    if (-1 == waitpid(pid, nullptr, 0)) return EXIT_FAILURE;
    std::cout << "child has terminated" << std::endl;

    close(sock);

    return EXIT_SUCCESS;
}
