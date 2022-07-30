#include <iostream>

extern "C"
{
#if !defined(_XOPEN_SOURCE)
#    define _XOPEN_SOURCE
#endif
#include <limits.h>
#include <unistd.h>
}


int main(int argc, const char* const argv[])
{
    auto buf_count = sysconf(_SC_IOV_MAX);

    if (-1 == buf_count)
    {
        perror("sysconf");
#if defined(IOV_MAX)
        buf_count = IOV_MAX;
        std::cout
            << "Buffers count will be set via IOV_MAX\n"
#endif
    }
    else
    {
        std::cout
            << "Buffers count will be set via sysconf()\n"
    }

#if defined(IOV_MAX)
    std::cout
        << "IOV_MAX = " << IOV_MAX << "\n";
#endif
    std::cout
        << "Buffers count = "
        << buf_count << std::endl;
}
