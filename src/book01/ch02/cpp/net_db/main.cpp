extern "C"
{
#include <netdb.h>
}

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <iostream>


int main()
{
    socket_wrapper::SocketWrapper sock_wrap;

    hostent *he;

    sethostent(1);
    while ((he = gethostent()) != nullptr)
    {
        std::cout << he->h_name << std::endl;
    }
    endhostent();

    return EXIT_SUCCESS;
}
