extern "C"
{
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
}

#include <iostream>
#include <stdexcept>
#include <string>


void print_current_alg(int sock)
{
    std::string alg_name;
    alg_name.resize(256);
    socklen_t len = alg_name.length();

    if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, &alg_name[0], &len) != 0)
    {
        throw std::system_error(errno, std::system_category(), "getsockopt");
    }
    alg_name.resize(len);

    std::cout << "Current algorithm: " << alg_name << std::endl;
}


void set_new_alg(int sock, const std::string &alg_name)
{
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, alg_name.c_str(), alg_name.length()) != 0)
    {
        throw std::system_error(errno, std::system_category(), "setsockopt");
    }
}


int main(int argc, const char *const argv[])
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    try
    {
        if (-1 == sock)
        {
            throw std::system_error(errno, std::system_category(), "socket");
        }

        print_current_alg(sock);
        std::cout << "Trying to set new algorithm..." << std::endl;
        set_new_alg(sock, "reno");
        print_current_alg(sock);

        close(sock);
    }
    catch (const std::exception &e)
    {
        close(sock);
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
